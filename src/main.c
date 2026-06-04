#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#define LED_PORT       GPIOD
#define LED_PIN        GPIO12

// =====================================================
// BANDPASS FILTER — Direct Form II, 4th order
// =====================================================
// Numerator (b)
static const float b0 =  0.013202172f;
static const float b1 =  0.0f;
static const float b2 = -0.026404344f;
static const float b3 =  0.0f;
static const float b4 =  0.013202172f;

// Denominator (a), знаки: y = b*x - a*w (a1..a4 уже со знаком «+» в уравнении)
static const float a1 = -3.6585689f;
static const float a2 =  5.0865269f;
static const float a3 = -3.1905211f;
static const float a4 =  0.7630810f;

// Внутренние состояния DF-II (w[k-1]..w[k-4])
static float w1 = 0.0f;
static float w2 = 0.0f;
static float w3 = 0.0f;
static float w4 = 0.0f;

// =====================================================
// FILTER  —  Direct Form II (single section, order 4)
//
//   w[k]  =  x[k] - a1*w1 - a2*w2 - a3*w3 - a4*w4
//   y[k]  =  b0*w[k] + b1*w1 + b2*w2 + b3*w3 + b4*w4
// =====================================================
static float filter(float x)
{
    // Вычисляем новое состояние
    float w = x - a1*w1 - a2*w2 - a3*w3 - a4*w4;

    // Вычисляем выход
    float y = b0*w + b1*w1 + b2*w2 + b3*w3 + b4*w4;

    // Сдвиг состояний
    w4 = w3;
    w3 = w2;
    w2 = w1;
    w1 = w;

    return y;
}

// =====================================================
// ADC
// =====================================================
static uint16_t read_adc(uint8_t ch)
{
    uint8_t channels[1] = { ch };
    adc_set_regular_sequence(ADC1, 1, channels);
    adc_start_conversion_regular(ADC1);
    while (!adc_eoc(ADC1));
    return (uint16_t)adc_read_regular(ADC1);
}

// =====================================================
// TIMER ISR
// =====================================================
void tim2_isr(void)
{
    if (timer_get_flag(TIM2, TIM_SR_UIF)) {
        timer_clear_flag(TIM2, TIM_SR_UIF);

        // ADC read
        uint16_t input = read_adc(0);

        // Convert 0..4095 -> -1..1
        float x = ((float)input / 4095.0f) * 2.0f - 1.0f;

        // Filter
        float y = filter(x);

        // Convert back -1..1 -> 0..1
        y = (y + 1.0f) * 0.5f;

        // Clipping protection
        if (y < 0.0f) y = 0.0f;
        if (y > 1.0f) y = 1.0f;

        // DAC value
        uint16_t out = (uint16_t)(y * 4095.0f);

        // DAC output
        dac_load_data_buffer_single(DAC1, out, DAC_ALIGN_RIGHT12, DAC_CHANNEL2);
        dac_software_trigger(DAC1, DAC_CHANNEL2);

        // LED toggle
        gpio_toggle(LED_PORT, LED_PIN);
    }
}

// =====================================================
// CLOCK
// =====================================================
static void clock_setup(void)
{
    rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART2);
    rcc_periph_clock_enable(RCC_ADC1);
    rcc_periph_clock_enable(RCC_DAC);
    rcc_periph_clock_enable(RCC_TIM2);
}

// =====================================================
// GPIO
// =====================================================
static void gpio_setup(void)
{
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
}

// =====================================================
// ADC SETUP
// =====================================================
static void adc_setup(void)
{
    gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
    adc_power_off(ADC1);
    adc_disable_scan_mode(ADC1);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_84CYC);
    adc_power_on(ADC1);
}

// =====================================================
// DAC SETUP
// =====================================================
static void dac_setup(void)
{
    gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO5);
    dac_disable(DAC1, DAC_CHANNEL2);
    dac_enable(DAC1, DAC_CHANNEL2);
}

// =====================================================
// TIMER
// =====================================================
static void tim_setup(void)
{
    nvic_enable_irq(NVIC_TIM2_IRQ);
    rcc_periph_reset_pulse(RST_TIM2);
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_prescaler(TIM2, (rcc_apb1_frequency * 2) / 1000000);
    timer_set_period(TIM2, 1000000 / 17500);   // Fs = 17500 Hz
    timer_enable_irq(TIM2, TIM_DIER_UIE);
    timer_enable_counter(TIM2);
}

// =====================================================
// MAIN
// =====================================================
int main(void)
{
    clock_setup();
    gpio_setup();
    adc_setup();
    dac_setup();
    tim_setup();
    while (1) {}
}