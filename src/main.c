#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <libopencm3/cm3/nvic.h>

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>

#define LED_PORT       GPIOD
#define LED_PIN        GPIO12
#define USART_CONSOLE  USART2

// =====================================================
// BANDPASS FILTER
// =====================================================

// Numerator
static const float b0 =  0.013202172f;
static const float b1 =  0.0f;
static const float b2 = -0.026404344f;
static const float b3 =  0.0f;
static const float b4 =  0.013202172f;

// Denominator
static const float a1 = -3.6585689f;
static const float a2 =  5.0865269f;
static const float a3 = -3.1905211f;
static const float a4 =  0.7630810f;

// Input history
static float x1 = 0.0f;
static float x2 = 0.0f;
static float x3 = 0.0f;
static float x4 = 0.0f;

// Output history
static float y_1 = 0.0f;
static float y_2 = 0.0f;
static float y_3 = 0.0f;
static float y_4 = 0.0f;

// =====================================================
// FILTER
// =====================================================

static float filter(float x)
{
    float y =b0 * x  + b1 * x1 + b2 * x2 + b3 * x3 + b4 * x4 -a1 * y_1 - a2 * y_2 - a3 * y_3 - a4 * y_4;

    // Shift input samples
    x4 = x3;
    x3 = x2;
    x2 = x1;
    x1 = x;

    // Shift output samples
    y_4 = y_3;
    y_3 = y_2;
    y_2 = y_1;
    y_1 = y;

    return y;
}

// =====================================================
// printf support
// =====================================================

int _write(int file, char *ptr, int len)
{
    (void)file;

    for (int i = 0; i < len; i++) {
        usart_send_blocking(USART_CONSOLE, ptr[i]);
    }

    return len;
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
        float x =
            ((float)input / 4095.0f) * 2.0f - 1.0f;

        // Filter
        float y = filter(x);

        // Convert back -1..1 -> 0..1
        y = (y + 1.0f) * 0.5f;

        // Clipping protection
        if (y < 0.0f)
            y = 0.0f;

        if (y > 1.0f)
            y = 1.0f;

        // DAC value
        uint16_t out =
            (uint16_t)(y * 4095.0f);

        // DAC output
        dac_load_data_buffer_single(
            DAC1,
            out,
            DAC_ALIGN_RIGHT12,
            DAC_CHANNEL2
        );

        dac_software_trigger(
            DAC1,
            DAC_CHANNEL2
        );

        // LED toggle
        gpio_toggle(LED_PORT, LED_PIN);
    }
}

// =====================================================
// CLOCK
// =====================================================

static void clock_setup(void)
{
    rcc_clock_setup_pll(
        &rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]
    );

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
    gpio_mode_setup(
        LED_PORT,
        GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE,
        LED_PIN
    );
}

// =====================================================
// ADC SETUP
// =====================================================

static void adc_setup(void)
{
    gpio_mode_setup(
        GPIOA,
        GPIO_MODE_ANALOG,
        GPIO_PUPD_NONE,
        GPIO0
    );

    adc_power_off(ADC1);

    adc_disable_scan_mode(ADC1);

    // Bigger sample time = less noise
    adc_set_sample_time_on_all_channels(
        ADC1,
        ADC_SMPR_SMP_84CYC
    );

    adc_power_on(ADC1);
}

// =====================================================
// DAC SETUP
// =====================================================

static void dac_setup(void)
{
    gpio_mode_setup(
        GPIOA,
        GPIO_MODE_ANALOG,
        GPIO_PUPD_NONE,
        GPIO5
    );

    dac_disable(DAC1, DAC_CHANNEL2);

    dac_enable(DAC1, DAC_CHANNEL2);
}

// =====================================================
// USART
// =====================================================

static void usart_setup(void)
{
    gpio_mode_setup(
        GPIOA,
        GPIO_MODE_AF,
        GPIO_PUPD_NONE,
        GPIO2
    );

    gpio_set_af(GPIOA, GPIO_AF7, GPIO2);

    usart_set_baudrate(
        USART_CONSOLE,
        115200
    );

    usart_set_databits(
        USART_CONSOLE,
        8
    );

    usart_set_stopbits(
        USART_CONSOLE,
        USART_STOPBITS_1
    );

    usart_set_parity(
        USART_CONSOLE,
        USART_PARITY_NONE
    );

    usart_set_flow_control(
        USART_CONSOLE,
        USART_FLOWCONTROL_NONE
    );

    usart_set_mode(
        USART_CONSOLE,
        USART_MODE_TX
    );

    usart_enable(USART_CONSOLE);
}

// =====================================================
// TIMER
// =====================================================

static void tim_setup(void)
{
    nvic_enable_irq(NVIC_TIM2_IRQ);

    rcc_periph_reset_pulse(RST_TIM2);

    timer_set_mode(
        TIM2,
        TIM_CR1_CKD_CK_INT,
        TIM_CR1_CMS_EDGE,
        TIM_CR1_DIR_UP
    );

    // Timer clock = 1 MHz
    timer_set_prescaler(
        TIM2,
        (rcc_apb1_frequency * 2) / 1000000
    );

    // Fs = 17500 Hz
    timer_set_period(
        TIM2,
        1000000 / 17500
    );

    timer_enable_irq(
        TIM2,
        TIM_DIER_UIE
    );

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

    usart_setup();

    tim_setup();

    printf("Bandpass filter Fs = 17500 Hz\r\n");

    while (1) {
    }
}

// =====================================================
// STUBS FOR NEWLIB
// =====================================================

int _close(int file)
{
    (void)file;
    return -1;
}

int _fstat(int file, void *st)
{
    (void)file;
    (void)st;
    return 0;
}

int _isatty(int file)
{
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}