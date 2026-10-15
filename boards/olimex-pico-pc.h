#pragma once
// Olimex RP2040-PICO-PC with a Raspberry Pi Pico (RP2040) or Pico 2 (RP2350)
#if PICO_RP2350
#include "boards/pico2.h"
#else
#include "boards/pico.h"
#endif

// HDMI connector of the PICO-PC has the red and green pairs swapped
#define PICO_PC 1

// SDCARD
#define SDCARD_PIN_SPI0_SCK 6
#define SDCARD_PIN_SPI0_MOSI 7
#define SDCARD_PIN_SPI0_MISO 4
#define SDCARD_PIN_SPI0_CS 22

// PS2KBD
#define PS2KBD_GPIO_FIRST 0

// NES Gamepad
#define NES_GPIO_CLK 8
#define NES_GPIO_LAT 9
#define NES_GPIO_DATA 20

// VGA 8 pins starts from pin:
#define VGA_BASE_PIN 12

// HDMI 8 pins starts from pin:
#define HDMI_BASE_PIN 12

// TFT
#define TFT_CS_PIN 12
#define TFT_RST_PIN 14
#define TFT_LED_PIN 15
#define TFT_DC_PIN 16
#define TFT_DATA_PIN 18
#define TFT_CLK_PIN 19
