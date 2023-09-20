// Hardware PWM support on samd
//
// Copyright (C) 2018-2019  Kevin O'Connor <kevin@koconnor.net>
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "command.h" // shutdown
#include "gpio.h" // gpio_pwm_write
#include "internal.h" // GPIO
#include "sched.h" // sched_shutdown

// Available TCC devices
struct tcc_info_s {
    Tcc *tcc;
    uint32_t pclk_id, pm_id;
};
static const struct tcc_info_s tcc_info[] = {
    { TCC0, TCC0_GCLK_ID, ID_TCC0 },
    { TCC1, TCC1_GCLK_ID, ID_TCC1 },
    { TCC2, TCC2_GCLK_ID, ID_TCC2 },
    #if CONFIG_MACH_SAMD51J19 || CONFIG_MACH_SAMD51N19 \
     || CONFIG_MACH_SAMD51P19 || CONFIG_MACH_SAMD51P20 \
     || CONFIG_MACH_SAME51J19 || CONFIG_MACH_SAME54P20
    { TCC3, TCC3_GCLK_ID, ID_TCC3 },
    { TCC4, TCC4_GCLK_ID, ID_TCC4 },
    #endif
};

#if CONFIG_MACH_SAMX5
#define TC_OFFSET 5

// Available TC devices
struct tc_info_s {
    Tc *tc;
    uint32_t pclk_id, pm_id;
};
static const struct tc_info_s tc_info[] = {
    { TC0, TC0_GCLK_ID, ID_TC0 },
    { TC1, TC1_GCLK_ID, ID_TC1 },
    { TC2, TC2_GCLK_ID, ID_TC2 },
    { TC3, TC3_GCLK_ID, ID_TC3 },
    #if CONFIG_MACH_SAMD51J19 || CONFIG_MACH_SAMD51N19 \
     || CONFIG_MACH_SAMD51P20 || CONFIG_MACH_SAME51J19
    { TC4, TC4_GCLK_ID, ID_TC4 },
    { TC5, TC5_GCLK_ID, ID_TC5 },
    #if CONFIG_MACH_SAMD51N19 || CONFIG_MACH_SAMD51P20 || CONFIG_MACH_SAME54P20
    { TC6, TC6_GCLK_ID, ID_TC6 },
    { TC7, TC7_GCLK_ID, ID_TC7 },
    #endif
    #endif
};
#endif

// PWM pins and their TCC device/channel
struct gpio_pwm_info {
    uint8_t gpio, ptype, tcc, channel;
};
static const struct gpio_pwm_info pwm_regs[] = {
#if CONFIG_MACH_SAMC21 || CONFIG_MACH_SAMD21
    { GPIO('A', 4),  'E', 0, 0 },
    { GPIO('A', 5),  'E', 0, 1 },
    { GPIO('A', 6),  'E', 1, 0 },
    { GPIO('A', 7),  'E', 1, 1 },
    { GPIO('A', 8),  'E', 0, 0 },
    { GPIO('A', 9),  'E', 0, 1 },
    { GPIO('A', 10), 'E', 1, 0 },
    { GPIO('A', 11), 'E', 1, 1 },
    { GPIO('A', 12), 'E', 2, 0 },
    { GPIO('A', 13), 'E', 2, 1 },
    { GPIO('A', 16), 'E', 2, 0 },
    { GPIO('A', 17), 'E', 2, 1 },
    { GPIO('A', 18), 'F', 0, 2 },
    { GPIO('A', 19), 'F', 0, 3 },
    { GPIO('A', 24), 'F', 1, 2 },
    { GPIO('A', 25), 'F', 1, 3 },
    { GPIO('A', 30), 'E', 1, 0 },
    { GPIO('A', 31), 'E', 1, 1 },
    { GPIO('B', 30), 'E', 0, 0 },
    { GPIO('B', 31), 'E', 0, 1 },
#elif CONFIG_MACH_SAMX5
    { GPIO('A', 4),  'E', 5, 0 }, // TC
    { GPIO('A', 6),  'E', 6, 0 }, // TC
    { GPIO('A', 12), 'E', 7, 0 }, // TC
    { GPIO('A', 13), 'E', 7, 1 }, // TC, Tied to A12
    { GPIO('A', 14), 'E', 8, 0 }, // TC
    { GPIO('A', 16), 'F', 1, 0 },
    { GPIO('A', 17), 'F', 1, 1 },
    { GPIO('A', 18), 'F', 1, 2 },
    { GPIO('A', 19), 'F', 1, 3 },
    { GPIO('A', 20), 'G', 0, 0 },
    { GPIO('A', 21), 'G', 0, 1 },
    { GPIO('A', 22), 'G', 0, 2 },
    { GPIO('A', 23), 'G', 0, 3 },
    { GPIO('B', 2),  'F', 2, 2 },
    { GPIO('B', 8),  'E', 9, 0 }, // TC
    { GPIO('B', 9),  'E', 9, 1 }, // TC, Tied to B8
    { GPIO('B', 12), 'F', 3, 0 },
    { GPIO('B', 13), 'F', 3, 1 },
    { GPIO('B', 14), 'F', 4, 0 },
    { GPIO('B', 15), 'F', 4, 1 },
    { GPIO('B', 16), 'G', 0, 4 },
    { GPIO('B', 17), 'G', 0, 5 },
#endif
};

#define MAX_PWM 255
DECL_CONSTANT("PWM_MAX", MAX_PWM);

uint32_t
generate_clock_divisor(uint32_t cycle_time)
{
    // Map cycle_time to pwm clock divisor
    switch (cycle_time) {
    case                      0 ...      (1+2) * MAX_PWM / 2 - 1: return 0;
    case    (1+2) * MAX_PWM / 2 ...      (2+4) * MAX_PWM / 2 - 1: return 1;
    case    (2+4) * MAX_PWM / 2 ...      (4+8) * MAX_PWM / 2 - 1: return 2;
    case    (4+8) * MAX_PWM / 2 ...     (8+16) * MAX_PWM / 2 - 1: return 3;
    case   (8+16) * MAX_PWM / 2 ...    (16+64) * MAX_PWM / 2 - 1: return 4;
    case  (16+64) * MAX_PWM / 2 ...   (64+256) * MAX_PWM / 2 - 1: return 5;
    case (64+256) * MAX_PWM / 2 ... (256+1024) * MAX_PWM / 2 - 1: return 6;
    default:                                                      return 7;
    }
}

struct gpio_pwm
tcc_setup(uint8_t tcc_id, uint8_t channel, uint32_t cycle_time)
{
    // Enable timer clock
    enable_pclock(tcc_info[tcc_id].pclk_id, tcc_info[tcc_id].pm_id);

    uint32_t cs = generate_clock_divisor(cycle_time);
    uint32_t ctrla = TCC_CTRLA_ENABLE | TCC_CTRLA_PRESCALER(cs);

    // Enable timer
    Tcc *tcc = tcc_info[tcc_id].tcc;
    uint32_t old_ctrla = tcc->CTRLA.reg;
    if (old_ctrla != ctrla) {
        if (old_ctrla & TCC_CTRLA_ENABLE)
            shutdown("PWM already programmed at different speed");
        tcc->CTRLA.reg = ctrla & ~TCC_CTRLA_ENABLE;
        tcc->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
        tcc->PER.reg = MAX_PWM;
        tcc->CTRLA.reg = ctrla;
    }

    // Return pwm access
    #if CONFIG_MACH_SAMD21
    return (struct gpio_pwm) { (void*)&tcc->CCB[channel].reg };
    #elif CONFIG_MACH_SAMC21 || CONFIG_MACH_SAMX5
    return (struct gpio_pwm) { (void*)&tcc->CCBUF[channel].reg };
    #endif
}

#ifdef TC_OFFSET
struct gpio_pwm
tc_setup(uint8_t tc_id, uint8_t channel, uint32_t cycle_time)
{
    // Enable timer clock
    enable_pclock(tc_info[tc_id].pclk_id, tc_info[tc_id].pm_id);

    uint32_t cs = generate_clock_divisor(cycle_time);
    uint32_t ctrla = TC_CTRLA_ENABLE | TC_CTRLA_PRESCALER(cs);

    // Enable timer
    TcCount8 *tc = &tc_info[tc_id].tc->COUNT8;
    uint32_t old_ctrla = tc->CTRLA.reg;
    if (old_ctrla != ctrla) {
        if (old_ctrla & TC_CTRLA_ENABLE)
            shutdown("PWM already programmed at different speed");
        tc->CTRLA.reg = ctrla & ~TC_CTRLA_ENABLE;
        tc->WAVE.reg = TC_WAVE_WAVEGEN_NPWM;
        tc->PER.reg = MAX_PWM;
        tc->CTRLA.reg = ctrla;
    }

    // Return pwm access
    return (struct gpio_pwm) { (void*)&tc->CCBUF[channel].reg };
}
#endif

struct gpio_pwm
gpio_pwm_setup(uint8_t pin, uint32_t cycle_time, uint8_t val)
{
    // Find pin in pwm_regs table
    const struct gpio_pwm_info *p = pwm_regs;
    for (; ; p++) {
        if (p >= &pwm_regs[ARRAY_SIZE(pwm_regs)])
            shutdown("Not a valid PWM pin");
        if (p->gpio == pin)
            break;
    }

    struct gpio_pwm g;

    // Initialize pwm hardware
    #ifdef TC_OFFSET
    if (p->tcc < TC_OFFSET) // TCC's end at 4, TC's start at 5 for SAMX5
    #endif
        g = tcc_setup(p->tcc, p->channel, cycle_time);
    #ifdef TC_OFFSET
    else
        g = tc_setup(p->tcc - TC_OFFSET, p->channel, cycle_time);
    #endif

    // Set initial value
    gpio_pwm_write(g, val);

    // Route output to pin
    gpio_peripheral(pin, p->ptype, 0);

    return g;
}

void
gpio_pwm_write(struct gpio_pwm g, uint8_t val)
{
    *(volatile uint32_t*)g.reg = val;
}
