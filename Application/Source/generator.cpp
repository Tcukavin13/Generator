#include "generator.h"

extern "C" {
#include "at32f435_437.h"
}

#include <algorithm>

static constexpr uint32_t IRQ_PRIORITY = 4U;

static constexpr uint32_t LOWER_FREQ = 100U;
static constexpr uint32_t UPPER_FREQ = 20000U;

static constexpr uint32_t TIM_PERIOD = 50U;
static constexpr float TIM_CLK = 288000000.0f;
/* Timer overflow frequency for the divider = 1 */
static constexpr float TIM_OVERFLOW_FREQ = TIM_CLK / TIM_PERIOD;

static constexpr uint32_t MAX_PULSES = 2560U;
static uint16_t sig[MAX_PULSES + 1U] = {0U};

static volatile bool timIsBusy = false;
static volatile bool dataIsValid = false;

static void gpioConfig(void)
{
    gpio_init_type gpioSettings;
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

    gpio_default_para_init(&gpioSettings);

    gpioSettings.gpio_mode = GPIO_MODE_MUX;
    gpioSettings.gpio_pull = GPIO_PULL_DOWN;
    gpioSettings.gpio_pins = GPIO_PINS_8;

    gpio_init(GPIOA, &gpioSettings);

    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE8,  GPIO_MUX_1);  /* TMR1_CH1  */
}

static void timConfig(void)
{
    tmr_output_config_type timOcSettings;
    crm_periph_clock_enable(CRM_TMR1_PERIPH_CLOCK, TRUE);

    tmr_output_default_para_init(&timOcSettings);

    timOcSettings.oc_idle_state = FALSE;
    timOcSettings.oc_output_state = TRUE;
    timOcSettings.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;
    timOcSettings.oc_mode = TMR_OUTPUT_CONTROL_FORCE_LOW;

    tmr_cnt_dir_set(TMR1, TMR_COUNT_UP);
    tmr_clock_source_div_set(TMR1, TMR_CLOCK_DIV1);

    tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_1, TIM_PERIOD / 2U);
    tmr_output_channel_buffer_enable(TMR1, TMR_SELECT_CHANNEL_1, TRUE);
    tmr_output_channel_config(TMR1, TMR_SELECT_CHANNEL_1, &timOcSettings);

    /* Enable TIM1 CH1 DMA request */
    tmr_dma_request_enable(TMR1, TMR_C1_DMA_REQUEST, TRUE);

    /* Enable global TMR1 overflow interrupt */
    nvic_irq_enable(TMR1_OVF_TMR10_IRQn, IRQ_PRIORITY, 0U);

    tmr_output_enable(TMR1, TRUE);
}

static void dmaConfig(void)
{
    /* Enable DMAs clock */
    crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
    /* Enable DMA MUX */
    dmamux_enable(DMA1, TRUE);

    /* DMA1 Channel 1 configuration */
    dma_reset(DMA1_CHANNEL1);

    DMA1_CHANNEL1->ctrl &= 0xbfef;
    DMA1_CHANNEL1->ctrl |= DMA_DIR_MEMORY_TO_PERIPHERAL;
    DMA1_CHANNEL1->ctrl_bit.lm = FALSE;
    DMA1_CHANNEL1->ctrl_bit.mincm = TRUE;
    DMA1_CHANNEL1->ctrl_bit.pincm = FALSE;
    DMA1_CHANNEL1->ctrl_bit.chpl = DMA_PRIORITY_MEDIUM;
    DMA1_CHANNEL1->ctrl_bit.mwidth = DMA_MEMORY_DATA_WIDTH_HALFWORD;
    DMA1_CHANNEL1->ctrl_bit.pwidth = DMA_PERIPHERAL_DATA_WIDTH_WORD;
    DMA1_CHANNEL1->paddr = reinterpret_cast<uint32_t>(&TMR1->div);

    /* Enable global DMA1 Channel1 interrupt */
    nvic_irq_enable(DMA1_Channel1_IRQn, IRQ_PRIORITY, 0U);
    dma_interrupt_enable(DMA1_CHANNEL1, DMA_FDT_INT, TRUE);

    /* Config DMA request for TIM1 channel 1 compare event */
    dmamux_init(DMA1MUX_CHANNEL1, DMAMUX_DMAREQ_ID_TMR1_CH1);
}

static inline uint16_t calculateDivider(uint32_t freq)
{
    return static_cast<uint16_t>(TIM_OVERFLOW_FREQ / std::clamp(freq, LOWER_FREQ, UPPER_FREQ)) - 1U;
}

static inline void timForceDisable(void)
{
    tmr_output_channel_mode_select(TMR1, TMR_SELECT_CHANNEL_1, TMR_OUTPUT_CONTROL_FORCE_LOW);
    tmr_counter_enable(TMR1, FALSE);
    timIsBusy = false;
}

/**
 * @brief »нициализирует генератор импульсов
 * 
 * @details ¬ыполн€ет инициализацию периферии микроконтроллера необходимую дл€ работы генератора.
 *          ƒолжен быть вызван перед использованием других методов класса.
 */
void Generator::init(void)
{
    gpioConfig();
    timConfig();
    dmaConfig();
}

/**
 * @brief ƒобавл€ет пакет сигналов дл€ генерации.
 * 
 * @param[in] signal - пакет сигналов дл€ генерации.
 * 
 * @return —татус выполнени€ операции.
 *          @def Status::OK в случае успеха.
 *          @def Status::BUSY если текуща€ последовательность не завершена.
 *          @def Status::OVERFLOW - длина последовательности импульсов больше максимальной.
 */
Status Generator::add(std::span<Packet> signal)
{
    Status status = Status::BUSY;
    uint32_t pulses = 0U;
    uint16_t point = 0U;

    if(timIsBusy != true)
    {
        for(auto s : signal)
        {
            pulses += s.n;
        }

        if(pulses <= MAX_PULSES)
        {
            for(auto s : signal)
            {
                uint32_t div = calculateDivider(s.freq);

                for(uint32_t i = 0U; i < s.n; ++i)
                {
                    sig[point++] = div;
                }
            }

            /* New transactions setup */
            DMA1_CHANNEL1->ctrl_bit.chen = FALSE;
            DMA1_CHANNEL1->dtcnt_bit.cnt = pulses;
            DMA1_CHANNEL1->maddr = reinterpret_cast<uint32_t>(&sig[1U]);

            dataIsValid = true;
            status = Status::OK;
        }
        else
        {
            status = Status::OVERFLOW;
        }
    }

    return status;
}

/**
 * @brief «апускает генерацию импульсов.
 * 
 * @param[in] none
 * 
 * @return —татус выполнени€ операции.
 *          @def Status::OK в случае успеха.
 *          @def Status::BUSY если текуща€ последовательность не завершена.
 *          @def Status::OVERFLOW - длина последовательности импульсов больше максимальной.
 */
Status Generator::start(void)
{
    Status status = Status::BUSY;

    if(timIsBusy != true)
    {
        if(dataIsValid != false)
        {
            timIsBusy = true;

            /* Setup the first pulse */
            tmr_interrupt_enable(TMR1, TMR_OVF_INT, FALSE);
            tmr_counter_value_set(TMR1, 0U);
            tmr_base_init(TMR1, TIM_PERIOD - 1U, sig[0U]);
            tmr_output_channel_mode_select(TMR1, TMR_SELECT_CHANNEL_1, TMR_OUTPUT_CONTROL_PWM_MODE_A);
            /* Enable DMA channel */
            dma_channel_enable(DMA1_CHANNEL1, TRUE);
            /* Enable timer */
            tmr_counter_enable(TMR1, TRUE);

            status = Status::OK;
        }
        else
        {
            status = Status::OVERFLOW;
        }
    }

    return status;
}

/**
 * @brief ќстанавливает генерацию сигналов.
 * 
 * @param[in] none
 * 
 * @return none
 */
void Generator::stop(void)
{
    timForceDisable();
}

/**
 * @brief ¬озвращает максимальное количество импульсов, которое может сгенерировать устройство.
 * 
 * @param[in] none
 * 
 * @return ћаксимальное количество импульсов.
 */
uint32_t Generator::getMaxPulses(void)
{
    return MAX_PULSES;
}

/**
 * @brief ¬озвращает текущий статус работы генератора.
 * 
 * @param[in] none
 * 
 * @return “екущий статус работы генератора.
 *          @def Status::READY генератор готов к работе.
 *          @def Status::BUSY генератор зан€т.
 */
Status Generator::getStatus(void)
{
    Status status = Status::READY;
    
    if(timIsBusy != false)
    {
        status = Status::BUSY;
    }

    return status;
}

extern "C" {
    void DMA1_Channel1_IRQHandler(void)
    {
        if(dma_flag_get(DMA1_FDT1_FLAG) != RESET)
        {
            /* If it the last transaction, enable timer overflow interrupt */
            tmr_interrupt_enable(TMR1, TMR_OVF_INT, TRUE);
            dma_flag_clear(DMA1_FDT1_FLAG);
        }
    }

    void TMR1_OVF_TMR10_IRQHandler(void)
    {
        if(tmr_flag_get(TMR1, TMR_OVF_FLAG) != RESET)
        {
            timForceDisable();
            tmr_flag_clear(TMR1, TMR_OVF_FLAG);
        }
    }
}
