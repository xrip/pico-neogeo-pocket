#pragma once
// MURMULATOR 2.0 with a Raspberry Pi Pico 2 (RP2350A, 4 MB flash) - "m2p2"
// in the pico-class kit. The flash size comes from boards/pico2.h.
//   HDMI/VGA GPIO12..19 (HDMI: clock 12/13, D0 14/15, D1 16/17, D2 18/19)
//   Sound    PWM_Enable jumper: TDA1387 I2S (DIN 9, BCK 10, LRCK 11) or PWM
//            on GP10/GP11 (cmake -DI2S=OFF, AUDIO_PWM_PIN=10)
//   PS/2     keyboard: CLK GP2, DATA GP3 (ps2kbd_mrmltr2.pio)
//   SD       SPI0: CS 5, SCK 6, MOSI 7, MISO 4
//   NES      pads: CLK 20, LAT 21, DATA 26
#if PICO_RP2350
#include "boards/pico2.h"
#else
#include "boards/pico.h"
#endif

// SDCARD
#define SDCARD_PIN_SPI0_CS 5
#define SDCARD_PIN_SPI0_SCK 6
#define SDCARD_PIN_SPI0_MOSI 7
#define SDCARD_PIN_SPI0_MISO 4

// PS2KBD
#define PS2KBD_GPIO_FIRST 2

// NES Gamepad
#define NES_GPIO_CLK 20
#define NES_GPIO_LAT 21
#define NES_GPIO_DATA 26

// VGA 8 pins starts from pin:
#define VGA_BASE_PIN 12

// HDMI 8 pins starts from pin:
#define HDMI_BASE_PIN 12

// I2S sound: TDA1387 DIN GP9, BCK GP10, LRCK GP11
#define AUDIO_DATA_PIN 9
#define AUDIO_CLOCK_PIN 10

// TFT
#define TFT_CS_PIN 12
#define TFT_RST_PIN 14
#define TFT_LED_PIN 15
#define TFT_DC_PIN 16
#define TFT_DATA_PIN 18
#define TFT_CLK_PIN 19
