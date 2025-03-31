cmake_minimum_required(VERSION 3.25)

# target (project) name
set(TARGET_NAME at32f437_pwm)

# set target system
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# to skip cmake to test a simple executable on a host
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

############################ user defined toolchain options ##############################################

# arm mcu prefix
set(CROSS_COMPILE_PREFIX arm-none-eabi-)

# specify the cross compiler
set(CMAKE_AR ${CROSS_COMPILE_PREFIX}ar)
set(CMAKE_C_COMPILER ${CROSS_COMPILE_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${CROSS_COMPILE_PREFIX}gcc)
set(CMAKE_LINKER ${CROSS_COMPILE_PREFIX}ld)
set(CMAKE_OBJCOPY ${CROSS_COMPILE_PREFIX}objcopy)
set(CMAKE_OBJDUMP ${CROSS_COMPILE_PREFIX}objdump)
set(CMAKE_SIZE ${CROSS_COMPILE_PREFIX}size)
set(CMAKE_STRIP ${CROSS_COMPILE_PREFIX}strip)

# find additional toolchain executables
find_program(ARM_SIZE_EXECUTABLE ${CROSS_COMPILE_PREFIX}size)
find_program(ARM_GDB_EXECUTABLE ${CROSS_COMPILE_PREFIX}gdb)
find_program(ARM_OBJCOPY_EXECUTABLE ${CROSS_COMPILE_PREFIX}objcopy)
find_program(ARM_OBJDUMP_EXECUTABLE ${CROSS_COMPILE_PREFIX}objdump)

# search for program/library/include in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

########################################################################################################

# set project name
project(${TARGET_NAME} C CXX ASM)

# language version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

# project version
set(VERSION_MAJOR 0 CACHE STRING "Project major version number.")
set(VERSION_MINOR 1 CACHE STRING "Project minor version number.")
set(VERSION_PATCH 0 CACHE STRING "Project patch version number.")

if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
    endif()
endif()

# print project name
message(STATUS "Building project: ${PROJECT_NAME}")

# define mcu informations
set(MCU_ARCH cortex-m4)
set(MCU_NAME AT32F437ZMT7)

# add compile options
add_compile_options(-mcpu=${MCU_ARCH} 
                    -mthumb 
                    -mfloat-abi=hard 
                    -mfpu=fpv4-sp-d16 
                    -ffunction-sections 
                    -fdata-sections
)

# for suppress deprecated operations with volatile objects
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>")

# set optimization options
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()

# add definitions
add_definitions(-D${MCU_NAME} -DUSE_STDPERIPH_DRIVER -DARM_MATH_CM4)

# add all appliction sources to the build
file(GLOB APP_SOURCES
    "${CMAKE_SOURCE_DIR}/Application/Source/*.c"
    "${CMAKE_SOURCE_DIR}/Application/Source/*.cpp"
)

# add mcu support sources to the build
file(GLOB MCU_SOURCES
    "${CMAKE_SOURCE_DIR}/Startup/startup_at32f435_437.S"
    "${CMAKE_SOURCE_DIR}/Startup/system_at32f435_437.c"
    "${CMAKE_SOURCE_DIR}/Libraries/StdPeriph/Source/*.c"
    "${CMAKE_SOURCE_DIR}/Libraries/Print/*.c"
)

# add FreeRTOS sources to the build
file(GLOB FREERTOS_SOURCES
    "${CMAKE_SOURCE_DIR}/Libraries/FreeRTOS/Source/*.c"
    "${CMAKE_SOURCE_DIR}/Libraries/FreeRTOS/Portable/port.c"
    "${CMAKE_SOURCE_DIR}/Libraries/FreeRTOS/Memory_management/heap_4.c"
)

# add include directories to the build
include_directories(
    "${CMAKE_SOURCE_DIR}/CMSIS/cm4/core_support"
    "${CMAKE_SOURCE_DIR}/CMSIS/cm4/device_support"
    "${CMAKE_SOURCE_DIR}/Application/Include"
    "${CMAKE_SOURCE_DIR}/Libraries/StdPeriph/Include"
    "${CMAKE_SOURCE_DIR}/Libraries/FreeRTOS"
    "${CMAKE_SOURCE_DIR}/Libraries/FreeRTOS/Include"
    "${CMAKE_SOURCE_DIR}/Libraries/FreeRTOS/Portable"
    "${CMAKE_SOURCE_DIR}/Libraries/Print"
)

# enable use static library
option(USE_STATIC "Use static library" ON)

# find the arm_cortexM4lf_math library and set the path in the ARM_CMSIS_DSP variable
find_library(ARM_CMSIS_DSP
             arm_cortexM4lf_math
             ${CMAKE_SOURCE_DIR}/Libraries/DSP
)

# print summary of build configurations
message(STATUS "MCU_ARCH: ${MCU_ARCH}")
message(STATUS "MCU_NAME: ${MCU_NAME}")
message(STATUS "Specified C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Specified C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Linker script: ${LINKER_SCRIPT}")

# sources list
list(APPEND SOURCES
    ${APP_SOURCES}
    ${MCU_SOURCES}
    ${FREERTOS_SOURCES}
)

# print project source files
message(STATUS "SOURCES:  ${SOURCES}")

# add linker script file
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/Linker/AT32F437xM_FLASH.ld")

# add linker options
add_link_options(-Wl,--gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-specs=nano.specs -specs=nosys.specs -u _printf_float)
add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
add_link_options(-mcpu=cortex-m4 -mthumb)

# add executable to the project
add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

# link static libraries in the project
target_link_libraries(${TARGET_NAME}.elf PRIVATE ${ARM_CMSIS_DSP})

# set linker options
target_link_options(${PROJECT_NAME}.elf PRIVATE -T ${LINKER_SCRIPT})

# create .hex and .bin name
set(HEX_FILE ${PROJECT_BINARY_DIR}/${CMAKE_CXX_COMPILER_ID}_${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${CMAKE_CXX_COMPILER_ID}_${PROJECT_NAME}.bin)

# create .hex and .bin files in postbuild
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
                   COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
                   COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
                   COMMENT "Building ${HEX_FILE}
                   Building ${BIN_FILE}")
