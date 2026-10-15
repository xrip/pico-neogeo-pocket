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

#define PWM_PIN0 (AUDIO_PWM_PIN&0xfe)
#define PWM_PIN1 (PWM_PIN0+1)

#include "audio.h"

/* I2S pins: DIN, then BCK and LRCK = BCK + 1. The board header may set them. */
#ifndef AUDIO_DATA_PIN
#define AUDIO_DATA_PIN 26
#endif
#ifndef AUDIO_CLOCK_PIN
#define AUDIO_CLOCK_PIN 27
#endif

#include "hardware/irq.h"
#include "hardware/sync.h"

static i2s_config_t *volatile i2s_irq_config = NULL;
static void i2s_dma_irq_handler(void);

#ifdef AUDIO_PWM_PIN
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#endif

/**
 * return the default i2s context used to store information about the setup
 */
i2s_config_t i2s_get_default_config(void) {
    i2s_config_t i2s_config = {
		.sample_freq = 44100, 
		.channel_count = 2,
		.data_pin = AUDIO_DATA_PIN,
		.clock_pin_base = AUDIO_CLOCK_PIN,
		.pio = pio1,
		.sm = 0,
        .dma_channel = 0,
        .dma_buf = NULL,
        .dma_trans_count = 0,
        .volume = 0,
	};

    return i2s_config;
}

/**
 * Initialize the I2S driver. Must be called before calling i2s_write or i2s_dma_write
 * i2s_config: I2S context obtained by i2s_get_default_config()
 */


void i2s_init(i2s_config_t *i2s_config) {


#ifndef AUDIO_PWM_PIN

    uint8_t func=GPIO_FUNC_PIO1;    // TODO: GPIO_FUNC_PIO0 for pio0 or GPIO_FUNC_PIO1 for pio1
    gpio_set_function(i2s_config->data_pin, func);
    gpio_set_function(i2s_config->clock_pin_base, func);
    gpio_set_function(i2s_config->clock_pin_base+1, func);
    
    i2s_config->sm = pio_claim_unused_sm(i2s_config->pio, true);

    /* Set PIO clock */
    uint32_t system_clock_frequency = clock_get_hz(clk_sys);
    uint32_t divider = system_clock_frequency * 4 / i2s_config->sample_freq; // avoid arithmetic overflow

#ifdef I2S_CS4334
    uint offset = pio_add_program(i2s_config->pio, &audio_i2s_cs4334_program);
    audio_i2s_cs4334_program_init(i2s_config->pio, i2s_config->sm , offset, i2s_config->data_pin , i2s_config->clock_pin_base);
    divider >>= 3;
#else
    uint offset = pio_add_program(i2s_config->pio, &audio_i2s_program);
    audio_i2s_program_init(i2s_config->pio, i2s_config->sm , offset, i2s_config->data_pin , i2s_config->clock_pin_base);

#endif

    pio_sm_set_clkdiv_int_frac(i2s_config->pio, i2s_config->sm , divider >> 8u, divider & 0xffu);

    pio_sm_set_enabled(i2s_config->pio, i2s_config->sm, false);
#endif
    /* Allocate memory for the DMA buffer */
    i2s_config->dma_buf=malloc(i2s_config->dma_trans_count*sizeof(uint32_t));
    for (unsigned i = 0; i < I2S_RING_BLOCKS; ++i)
        i2s_config->dma_ring[i] = malloc(i2s_config->dma_trans_count * sizeof(uint32_t));
    i2s_config->hold_buf = malloc(i2s_config->dma_trans_count * sizeof(uint32_t));
#ifdef AUDIO_PWM_PIN
    i2s_config->hold_l = i2s_config->hold_r = (65536 / 2) >> 4;   /* PWM mid level */
#else
    i2s_config->hold_l = i2s_config->hold_r = 0;
#endif
    i2s_config->ring_head = i2s_config->ring_tail = 0;

    /* Direct Memory Access setup */
    i2s_config->dma_channel = dma_claim_unused_channel(true);
    
    dma_channel_config dma_config = dma_channel_get_default_config(i2s_config->dma_channel);
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);

    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);

    volatile uint32_t* addr_write_DMA=&(i2s_config->pio->txf[i2s_config->sm]);
#ifdef AUDIO_PWM_PIN
    gpio_set_function(PWM_PIN0, GPIO_FUNC_PWM);
    gpio_set_function(PWM_PIN1, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN0);


   
    pwm_config c_pwm=pwm_get_default_config();
    pwm_config_set_clkdiv(&c_pwm,1.0);
    //pwm_config_set_wrap(&c_pwm,(1<<12)-1);//MAX PWM value
    pwm_config_set_wrap(&c_pwm,clock_get_hz(clk_sys)/(i2s_config->sample_freq));//MAX PWM value
    pwm_init(slice_num,&c_pwm,true);

    //Для синхронизации используем другой произвольный канал ШИМ


    channel_config_set_dreq(&dma_config, pwm_get_dreq(slice_num));     


    addr_write_DMA=(uint32_t*)&pwm_hw->slice[slice_num].cc;
#else
    channel_config_set_dreq(&dma_config, pio_get_dreq(i2s_config->pio, i2s_config->sm, true));
#endif
    
    dma_channel_configure(i2s_config->dma_channel,
                          &dma_config,
                          addr_write_DMA,    // Destination pointer
                          i2s_config->dma_buf,                        // Source pointer
                          i2s_config->dma_trans_count,                // Number of 32 bits words to transfer
                          false                                       // Start immediately
    );

    pio_sm_set_enabled(i2s_config->pio, i2s_config->sm , true);

    /* Keep the DMA running from now on: each completed block immediately
       starts the next one (or hold_buf) from the DMA_IRQ_1 handler. */
    i2s_irq_config = i2s_config;
    dma_channel_acknowledge_irq1(i2s_config->dma_channel);
    dma_channel_set_irq1_enabled(i2s_config->dma_channel, true);
    irq_add_shared_handler(DMA_IRQ_1, i2s_dma_irq_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(DMA_IRQ_1, true);
    i2s_dma_pump(i2s_config);
}

/**
 * Write samples to I2S directly and wait for completion (blocking)
 * i2s_config: I2S context obtained by i2s_get_default_config()
 *     sample: pointer to an array of len x 32 bits samples
 *             Each 32 bits sample contains 2x16 bits samples, 
 *             one for the left channel and one for the right channel
 *        len: length of sample in 32 bits words
 */
void i2s_write(const i2s_config_t *i2s_config,const int16_t *samples,const size_t len) {
    for(size_t i=0;i<len;i++) {
            pio_sm_put_blocking(i2s_config->pio, i2s_config->sm, (uint32_t)samples[i]);
    }
}

/**
 * Write samples to DMA buffer and initiate DMA transfer (non blocking)
 * i2s_config: I2S context obtained by i2s_get_default_config()
 *     sample: pointer to an array of dma_trans_count x 32 bits samples
 */
/**
 * Start the next DMA block: the oldest ring block, or on underrun hold_buf
 * at the last output level.  Runs from the DMA completion interrupt, so the
 * I2S clocks never stop between blocks.  When the DMA was restarted only by
 * polling, the PIO stalled with BCK/LRCK frozen until the next poll and the
 * TDA1387 output sagged toward zero during each stall: crackling.
 */
static void __not_in_flash_func(i2s_dma_start_next)(i2s_config_t *i2s_config) {
    uint16_t *blk;
    const uint16_t n = i2s_config->dma_trans_count;
    const uint8_t tail = i2s_config->ring_tail;
    if (tail != i2s_config->ring_head) {
        blk = i2s_config->dma_ring[tail];
        i2s_config->hold_l = blk[(n - 1) * 2 + 0];
        i2s_config->hold_r = blk[(n - 1) * 2 + 1];
        i2s_config->ring_tail = (uint8_t)((tail + 1u) % I2S_RING_BLOCKS);
    } else {
        blk = i2s_config->hold_buf;
        for (uint16_t i = 0; i < n; ++i) {
            blk[i * 2 + 0] = i2s_config->hold_l;
            blk[i * 2 + 1] = i2s_config->hold_r;
        }
    }
    dma_channel_transfer_from_buffer_now(i2s_config->dma_channel, blk, n);
}

static void __isr __not_in_flash_func(i2s_dma_irq_handler)(void) {
    i2s_config_t *i2s_config = i2s_irq_config;
    if (!i2s_config || !dma_channel_get_irq1_status(i2s_config->dma_channel))
        return;
    dma_channel_acknowledge_irq1(i2s_config->dma_channel);
    i2s_dma_start_next(i2s_config);
}

/**
 * Kick the DMA if it is idle (only before the first block; afterwards the
 * DMA interrupt keeps it running).  Safe to call often from core0.
 */
void __not_in_flash_func(i2s_dma_pump)(i2s_config_t *i2s_config) {
    if (dma_channel_is_busy(i2s_config->dma_channel)) return;
    const uint32_t ints = save_and_disable_interrupts();
    if (!dma_channel_is_busy(i2s_config->dma_channel) &&
        !dma_channel_get_irq1_status(i2s_config->dma_channel))
        i2s_dma_start_next(i2s_config);
    restore_interrupts(ints);
}

/**
 * Queue one block of dma_trans_count stereo samples (non blocking).
 * If the ring is full the block is dropped.
 */
void i2s_dma_write(i2s_config_t *i2s_config,const int16_t *samples) {
    const uint8_t head = i2s_config->ring_head;
    const uint8_t next = (uint8_t)((head + 1u) % I2S_RING_BLOCKS);
    if (next == i2s_config->ring_tail)
        return;                                   /* ring full: drop the block */

    uint16_t *dst = i2s_config->dma_ring[head];
#ifdef AUDIO_PWM_PIN
    for (uint16_t i = 0; i < i2s_config->dma_trans_count * 2; i++)
        dst[i] = (uint16_t)((65536 / 2 + samples[i]) >> (4 + i2s_config->volume));
#else
    if (i2s_config->volume == 0)
        memcpy(dst, samples, i2s_config->dma_trans_count * sizeof(int32_t));
    else
        for (uint16_t i = 0; i < i2s_config->dma_trans_count * 2; i++)
            dst[i] = samples[i] >> i2s_config->volume;
#endif

    __dmb();                                      /* block data before the index */
    i2s_config->ring_head = next;
}

/**
 * Adjust the output volume
 * i2s_config: I2S context obtained by i2s_get_default_config()
 *     volume: desired volume between 0 (highest. volume) and 16 (lowest volume)
 */
void i2s_volume(i2s_config_t *i2s_config,uint8_t volume) {
    if(volume>16) volume=16;
    i2s_config->volume=volume;
}

/**
 * Increases the output volume
 */
void i2s_increase_volume(i2s_config_t *i2s_config) {
    if(i2s_config->volume>0) {
        i2s_config->volume--;
    }
}

/**
 * Decreases the output volume
 */
void i2s_decrease_volume(i2s_config_t *i2s_config) {
    if(i2s_config->volume<16) {
        i2s_config->volume++;
    }
}
