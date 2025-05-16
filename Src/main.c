#include <stdint.h>
#include <stdbool.h>
#include "stm32f401re_gpio.h"
#include "stm32f401re_rcc.h"
#include "typedefs.h"

#define BTN_PRESS               0   // Active low button
#define GPIO_PIN_SET            1
#define GPIO_PIN_RESET          0

// Buzzer
#define BUZZER_PORT             GPIOC
#define BUZZER_PIN              GPIO_Pin_9
#define BUZZER_CLK              RCC_AHB1Periph_GPIOC

// Buttons
#define BUTTON_B2_PORT          GPIOB
#define BUTTON_B2_PIN           GPIO_Pin_3  // B2
#define B2_CLK                  RCC_AHB1Periph_GPIOB

#define BUTTON_B3_PORT          GPIOA
#define BUTTON_B3_PIN           GPIO_Pin_4  // B3
#define B3_CLK                  RCC_AHB1Periph_GPIOA

// LEDs
#define LED_ONBOARD_GREEN_PORT  GPIOA
#define LED_ONBOARD_GREEN_PIN   GPIO_Pin_5  // PA5 onboard green

#define LED_RGB_GREEN_PORT      GPIOA
#define LED_RGB_GREEN_PIN       GPIO_Pin_11 // PA10 RGB green

#define LED_BLUE_PORT           GPIOA
#define LED_BLUE_PIN            GPIO_Pin_10 // PA11 RGB blue

static void Buzzer_Init(void);
static void Button_Init(void);
static void LEDs_Init(void);
static void SetPin(GPIO_TypeDef* port, uint16_t pin, uint8_t state);
static uint8_t ReadPin(GPIO_TypeDef* port, uint16_t pin);
static void Delay(volatile uint32_t count);

int main(void) {
    Buzzer_Init();
    Button_Init();
    LEDs_Init();

    // Power-up indication: flash onboard green LED once
    SetPin(LED_ONBOARD_GREEN_PORT, LED_ONBOARD_GREEN_PIN, GPIO_PIN_SET);
    Delay(200000);
    SetPin(LED_ONBOARD_GREEN_PORT, LED_ONBOARD_GREEN_PIN, GPIO_PIN_RESET);
    Delay(200000);

    bool blue_on = false;
    uint32_t b2_counter = 0;

    while (1) {
        // B3 pressed once: flash RGB green LED + beep buzzer
        if (ReadPin(BUTTON_B3_PORT, BUTTON_B3_PIN) == BTN_PRESS) {
            Delay(20000);
            if (ReadPin(BUTTON_B3_PORT, BUTTON_B3_PIN) == BTN_PRESS) {
                SetPin(LED_RGB_GREEN_PORT, LED_RGB_GREEN_PIN, GPIO_PIN_SET); // Bật LED xanh lá đúng
                SetPin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
                Delay(200000);
                SetPin(LED_RGB_GREEN_PORT, LED_RGB_GREEN_PIN, GPIO_PIN_RESET);
                SetPin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
                while (ReadPin(BUTTON_B3_PORT, BUTTON_B3_PIN) == BTN_PRESS) {
                    Delay(10000);
                }
            }
        }

        // B2 hold > 500ms: bật LED xanh dương cho tới khi nhả nút
        if (ReadPin(BUTTON_B2_PORT, BUTTON_B2_PIN) == BTN_PRESS) {
            b2_counter++;
            if (b2_counter > 50 && !blue_on) {
                SetPin(LED_BLUE_PORT, LED_BLUE_PIN, GPIO_PIN_SET); // Bật LED xanh dương đúng
                blue_on = true;
            }
        } else {
            if (blue_on) {
                SetPin(LED_BLUE_PORT, LED_BLUE_PIN, GPIO_PIN_RESET);
                blue_on = false;
            }
            b2_counter = 0;
        }
        Delay(10000);
    }
    return 0;
}

/******************************* Initialization **********************************/
static void Buzzer_Init(void) {
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(BUZZER_CLK, ENABLE);
    gpio.GPIO_Pin = BUZZER_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(BUZZER_PORT, &gpio);
}

static void Button_Init(void) {
    GPIO_InitTypeDef gpio;
    // B2
    RCC_AHB1PeriphClockCmd(B2_CLK, ENABLE);
    gpio.GPIO_Pin = BUTTON_B2_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(BUTTON_B2_PORT, &gpio);

    // B3
    RCC_AHB1PeriphClockCmd(B3_CLK, ENABLE);
    gpio.GPIO_Pin = BUTTON_B3_PIN;
    GPIO_Init(BUTTON_B3_PORT, &gpio);
}

static void LEDs_Init(void) {
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    gpio.GPIO_Pin = LED_ONBOARD_GREEN_PIN | LED_RGB_GREEN_PIN | LED_BLUE_PIN;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LED_ONBOARD_GREEN_PORT, &gpio);
}

/******************************* Helper Functions *********************************/
static void SetPin(GPIO_TypeDef* port, uint16_t pin, uint8_t state) {
    if (state == GPIO_PIN_SET) port->BSRRL = pin;
    else                       port->BSRRH = pin;
}

static uint8_t ReadPin(GPIO_TypeDef* port, uint16_t pin) {
    return ((port->IDR & pin) == 0) ? BTN_PRESS : 1;
}

static void Delay(volatile uint32_t count) {
    while (count--) __NOP();
}
