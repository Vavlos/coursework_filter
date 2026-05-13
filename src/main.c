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

#define LED_PORT GPIOD
#define LED_PIN GPIO12
#define USART_CONSOLE USART2

// ===== FILTER =====
float b0_1=1.0f, b1_1=0.0f, b2_1=-1.0f;
float a1_1=-1.7741107f, a2_1=0.85366075f;
float g1=0.1080308f;

float b0_2=1.0f, b1_2=0.0f, b2_2=-1.0f;
float a1_2=-1.9491102f, a2_2=0.9558400f;
float g2=0.1080308f;

float g3=0.8035261f;

float x1_1=0, x2_1=0, y1_1=0, y2_1=0;
float x1_2=0, x2_2=0, y1_2=0, y2_2=0;

float biquad1(float x)
{
    float y = b0_1*x + b1_1*x1_1 + b2_1*x2_1
            - a1_1*y1_1 - a2_1*y2_1;

    x2_1 = x1_1;
    x1_1 = x;
    y2_1 = y1_1;
    y1_1 = y;
    return y;
}

float biquad2(float x)
{
    float y = b0_2*x + b1_2*x1_2 + b2_2*x2_2
            - a1_2*y1_2 - a2_2*y2_2;

    x2_2 = x1_2;
    x1_2 = x;
    y2_2 = y1_2;
    y1_2 = y;
    return y;
}

float filter(float x)
{
    float s = x * g1;
    s = biquad1(s);
    s = s * g2;
    s = biquad2(s);
    s = s * g3;
    return s;
}

// ===== printf support =====
int _write(int file, char *ptr, int len)
{
    (void)file;

    for (int i = 0; i < len; i++) {
        usart_send_blocking(USART_CONSOLE, ptr[i]);
    }
    return len;
}

// ===== ADC =====
static uint16_t read_adc(uint8_t ch)
{
    uint8_t chn[1] = {ch};
    adc_set_regular_sequence(ADC1, 1, chn);
    adc_start_conversion_regular(ADC1);
    while (!adc_eoc(ADC1));
    return (uint16_t)adc_read_regular(ADC1);
}

// ===== TIMER ISR =====
//прерывание
void tim2_isr(void)
{
    if (timer_get_flag(TIM2, TIM_SR_UIF)) {
        timer_clear_flag(TIM2, TIM_SR_UIF);

        uint16_t input = read_adc(0);

        // 0..4095 → -1..1
        float x = ((float)input / 4095.0f) * 2.0f - 1.0f;

        float y = filter(x);

        // обратно в 0..1
        y = (y + 1.0f) * 0.5f;

        // защита (минимальная)
        if (y < 0) y = 0;
        if (y > 1) y = 1;

        uint16_t out = (uint16_t)(y * 4095.0f);

        dac_load_data_buffer_single(DAC1, out,
            DAC_ALIGN_RIGHT12, DAC_CHANNEL2);
        dac_software_trigger(DAC1, DAC_CHANNEL2);

        gpio_toggle(LED_PORT, LED_PIN);
    }
}

// ===== SETUP =====
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

static void gpio_setup(void)
{
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
}

static void adc_setup(void)
{
    gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);

    adc_power_off(ADC1);
    adc_disable_scan_mode(ADC1);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC);
    adc_power_on(ADC1);
}

static void dac_setup(void)
{
    gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO5);

    dac_disable(DAC1, DAC_CHANNEL2);
    dac_enable(DAC1, DAC_CHANNEL2);
}

static void usart_setup(void)
{
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2);

    usart_set_baudrate(USART_CONSOLE, 115200);
    usart_set_databits(USART_CONSOLE, 8);
    usart_set_stopbits(USART_CONSOLE, USART_STOPBITS_1);
    usart_set_parity(USART_CONSOLE, USART_PARITY_NONE);
    usart_set_flow_control(USART_CONSOLE, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART_CONSOLE, USART_MODE_TX);

    usart_enable(USART_CONSOLE);
}

// ===== TIMER =====
static void tim_setup(void)
{
    nvic_enable_irq(NVIC_TIM2_IRQ);

    rcc_periph_reset_pulse(RST_TIM2);

    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
        TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    timer_set_prescaler(TIM2, (rcc_apb1_frequency * 2) / 1000000);

    timer_set_period(TIM2, 1000000 / 17500);

    timer_enable_irq(TIM2, TIM_DIER_UIE);
    timer_enable_counter(TIM2);
}

// ===== MAIN =====
int main(void)
{
    clock_setup();
    gpio_setup();
    adc_setup();
    dac_setup();
    usart_setup();
    tim_setup();

    printf("Fs = 17.5 kHz OK\n");

    while (1) {}
}




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