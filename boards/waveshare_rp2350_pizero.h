/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// -----------------------------------------------------
// NOTE: THIS HEADER IS ALSO INCLUDED BY ASSEMBLER SO
//       SHOULD ONLY CONSIST OF PREPROCESSOR DIRECTIVES
// -----------------------------------------------------

// This header may be included by other board headers as "boards/waveshare_rp2350_pizero.h"

#ifndef _BOARDS_WAVESHARE_RP2350_PIZERO
#define _BOARDS_WAVESHARE_RP2350_PIZERO

pico_board_cmake_set(PICO_PLATFORM, rp2350)

// For board detection
#define RASPBERRYPI_RP2350_PIZERO

// --- RP2350 VARIANT ---
#define PICO_RP2350A 0

#define PSRAM_CS1_GPIO_RP2350A 8
#define PSRAM_CS1_GPIO_RP2350B 47

// --- UART ---
#ifndef PICO_DEFAULT_UART
#define PICO_DEFAULT_UART 0
#endif
#ifndef PICO_DEFAULT_UART_TX_PIN
#define PICO_DEFAULT_UART_TX_PIN 0
#endif
#ifndef PICO_DEFAULT_UART_RX_PIN
#define PICO_DEFAULT_UART_RX_PIN 1
#endif

// --- I2C ---
#ifndef PICO_DEFAULT_I2C
#define PICO_DEFAULT_I2C 0
#endif
#ifndef PICO_DEFAULT_I2C_SDA_PIN
#define PICO_DEFAULT_I2C_SDA_PIN 10
#endif
#ifndef PICO_DEFAULT_I2C_SCL_PIN
#define PICO_DEFAULT_I2C_SCL_PIN 11
#endif

// --- SPI ---
#ifndef PICO_DEFAULT_SPI
#define PICO_DEFAULT_SPI 1
#endif
#ifndef PICO_DEFAULT_SPI_SCK_PIN
#define PICO_DEFAULT_SPI_SCK_PIN 30
#endif
#ifndef PICO_DEFAULT_SPI_TX_PIN
#define PICO_DEFAULT_SPI_TX_PIN 31
#endif
#ifndef PICO_DEFAULT_SPI_RX_PIN
#define PICO_DEFAULT_SPI_RX_PIN 40
#endif
#ifndef PICO_DEFAULT_SPI_CSN_PIN
#define PICO_DEFAULT_SPI_CSN_PIN 43
#endif

// --- FLASH ---

#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1

#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2
#endif

pico_board_cmake_set_default(PICO_FLASH_SIZE_BYTES, (16 * 1024 * 1024))
#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (16 * 1024 * 1024)
#endif

pico_board_cmake_set_default(PICO_RP2350_A2_SUPPORTED, 1)
#ifndef PICO_RP2350_A2_SUPPORTED
#define PICO_RP2350_A2_SUPPORTED 1
#endif

// Waveshare RP2350-PiZero, z0p2 (RP2350B, 16 MB flash). Pins as in the
// other z0p2 builds of the pico-class kit. Do not include boards/pico2.h:
// it makes the SDK treat the chip as RP2350A (GPIO32..39 unusable).
#define ZERO 1
// 504 MHz: the HDMI PIO divider (clk_sys / 252 MHz) is exactly 2. At 378 MHz
// it is 1.5, and HDMI lost sync on this board (pico-wonderswan).
#define CPU_FREQ 504

// SDCARD (hardware SPI1)
#define SDCARD_SPI_BUS spi1
#define SDCARD_PIN_SPI0_SCK 30
#define SDCARD_PIN_SPI0_MOSI 31
#define SDCARD_PIN_SPI0_MISO 40
#define SDCARD_PIN_SPI0_CS 43

// PS2KBD: CLK GP2, DATA GP3 (ps2kbd_mrmltr2.pio)
#define PS2KBD_GPIO_FIRST 2

// NES Gamepad: CLK GP4, LATCH GP5, DATA GP7
#define NES_GPIO_CLK 4
#define NES_GPIO_LAT 5
#define NES_GPIO_DATA 7

// HDMI: data GP32..37, clock pair GP38/39 (PIO0 with GPIO base 16)
#define HDMI_BASE_PIN 32
#define VGA_BASE_PIN 32

// Sound: PCM5102A on I2S (PIO1): DIN GP10, BCK GP11, LRCK GP12
#define AUDIO_DATA_PIN 10
#define AUDIO_CLOCK_PIN 11

#define PICO_DEFAULT_LED_PIN 25

#endif
