/**
 * MIT License
 *
 * Copyright (c) 2022 Vincent Mistler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include "audio_i2s.pio.h"

typedef struct i2s_config 
{
    uint32_t sample_freq;        
    uint16_t channel_count; 
    uint8_t  data_pin;
    uint8_t  clock_pin_base;
    PIO	     pio;
    uint8_t  sm; 
    uint8_t  dma_channel;
    uint16_t dma_trans_count;
    uint16_t *dma_buf;
    uint8_t volume;
    /* Non-blocking output: a small ring of finished blocks of dma_trans_count
       stereo samples.  The DMA completion interrupt (DMA_IRQ_1, core0) starts
       the next block at once, so the I2S clocks never stop between blocks;
       i2s_dma_write() (core0) only moves ring_head, the interrupt only
       ring_tail. */
#define I2S_RING_BLOCKS 4
    uint16_t *dma_ring[I2S_RING_BLOCKS];
    uint16_t *hold_buf;      /* played on underrun: holds the last level, no click */
    uint16_t  hold_l;
    uint16_t  hold_r;
    volatile uint8_t ring_head;
    volatile uint8_t ring_tail;
} i2s_config_t;


i2s_config_t i2s_get_default_config(void);
void i2s_init(i2s_config_t *i2s_config);
void i2s_write(const i2s_config_t *i2s_config,const int16_t *samples,const size_t len);
void i2s_dma_write(i2s_config_t *i2s_config,const int16_t *samples);
void i2s_dma_pump(i2s_config_t *i2s_config);
void i2s_volume(i2s_config_t *i2s_config,uint8_t volume);
void i2s_increase_volume(i2s_config_t *i2s_config);
void i2s_decrease_volume(i2s_config_t *i2s_config);

#ifdef __cplusplus
}
#endif
