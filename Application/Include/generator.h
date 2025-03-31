#ifndef GENERATOR_H_
#define GENERATOR_H_

#include <span>
#include <cstdint>

enum class Status: uint32_t
{
    OK,
    BUSY,
    READY,
    ERROR,
    OVERFLOW
};

struct Packet
{
    uint8_t n;
    uint16_t freq;  // 100Hz - 20'000 Hz
};

class Generator
{
    public:
        static void init(void);
        static Status add(std::span<Packet> signal);
        static Status start(void);
        static void stop(void);
        static uint32_t getMaxPulses(void);
        static Status getStatus(void);
};

#endif  /* GENERATOR_H_ */
