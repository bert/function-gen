/*
* File: code for controlling the ad9833 DDS
* Author: Tuomas Nylund (tuomas.nylund@gmail.com)
* Website: http://tuomasnylund.fi
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <avr/io.h>
#include <util/delay.h>
#include "ad9833.h"
#include "spi.h"


uint16_t ad_cmd_reg;
uint8_t  ad_mode;

float    ad_freq[2];
uint8_t  ad_freq_out;

uint16_t ad_phase[2];
uint8_t  ad_phase_out;

static inline void ad9833_send(uint16_t packet){
    spi_send_byte((uint8_t)(packet>>8));
    spi_send_byte((uint8_t)packet);
}

void ad9833_init(void){
    //init FSYNC pin (aka Chip select)
    ad_cmd_reg |= (1<<AD_B28);
    AD_FSYNC_DDR |= (1<<AD_FSYNC_BIT);
    AD_FSYNC_HI();

    _delay_us(10);

    AD_FSYNC_LO();
    _delay_us(5);
    ad9833_send((1<<AD_SLEEP12)|(1<<AD_RESET));
    ad_cmd_reg |= (1<<AD_SLEEP12);
    _delay_us(5);
    AD_FSYNC_HI();

    ad9833_set_frequency(0, 0);
    ad9833_set_frequency(1, 0);
    ad9833_set_phase(0, 0);
    ad9833_set_phase(1, 0);

    ad9833_set_freq_out(0);
    ad9833_set_phase_out(0);

    ad9833_power(0);
}

void ad9833_power(uint8_t power){
    if (power){
        ad_cmd_reg &= ~(1<<AD_SLEEP12);
        ad_cmd_reg &= ~(1<<AD_SLEEP1);
    }
    else{
        ad_cmd_reg |= (1<<AD_SLEEP12);
        ad_cmd_reg |= (1<<AD_SLEEP1);
    }
    AD_FSYNC_LO();
    _delay_us(5);
    ad9833_send(ad_cmd_reg);
    _delay_us(5);
    AD_FSYNC_HI();
}

void ad9833_set_mode(uint8_t mode){
    ad_mode = mode;
    switch (mode){
        case AD_OFF:
            ad_cmd_reg |= (1<<AD_SLEEP12);
            ad_cmd_reg |= (1<<AD_SLEEP1);
            break;
        case AD_TRIANGLE:
            ad_cmd_reg &= ~(1<<AD_OPBITEN);
            ad_cmd_reg |=  (1<<AD_MODE);
            ad_cmd_reg &= ~(1<<AD_SLEEP12);
            ad_cmd_reg &= ~(1<<AD_SLEEP1);
            break;
        case AD_SQUARE:
            ad_cmd_reg |=  (1<<AD_OPBITEN);
            ad_cmd_reg &= ~(1<<AD_MODE);
            ad_cmd_reg |=  (1<<AD_DIV2);
            ad_cmd_reg &= ~(1<<AD_SLEEP12);
            ad_cmd_reg &= ~(1<<AD_SLEEP1);
            break;
        case AD_SINE:
            ad_cmd_reg &= ~(1<<AD_OPBITEN);
            ad_cmd_reg &= ~(1<<AD_MODE);
            ad_cmd_reg &= ~(1<<AD_SLEEP12);
            ad_cmd_reg &= ~(1<<AD_SLEEP1);
            break;
    }

    AD_FSYNC_LO();
    _delay_us(5);
    ad9833_send(ad_cmd_reg);
    _delay_us(5);
    AD_FSYNC_HI();
}

void ad9833_set_phase(uint8_t reg, uint16_t phase){
    uint16_t reg_reg; //probably should be renamed...
    if (reg==1)
        reg_reg = AD_PHASE1;
    else
        reg_reg = AD_PHASE0;

    ad_phase[reg] = phase;

    AD_FSYNC_LO();
    _delay_us(5);
    ad9833_send(reg_reg | phase);
    _delay_us(5);
    AD_FSYNC_HI();
}

uint16_t ad9833_get_phase(uint8_t reg){
    return ad_phase[reg];
}

void    ad9833_set_freq_out(uint8_t freq_out){
    ad_freq_out = freq_out;
    switch (freq_out){
        case 0:
            ad_cmd_reg &= ~(1<<AD_FSELECT);
            break;
        case 1:
            ad_cmd_reg |= (1<<AD_FSELECT);
            break;
        case 2:
            //TODO
            break;
    }

    AD_FSYNC_LO();
    _delay_us(5);
    ad9833_send(ad_cmd_reg);
    _delay_us(5);
    AD_FSYNC_HI();
}

uint8_t ad9833_get_freq_out(void){
    return ad_freq_out;
}

void    ad9833_set_phase_out(uint8_t phase_out){
    ad_phase_out = phase_out;
    switch (phase_out){
        case 0:
            ad_cmd_reg &= ~(1<<AD_PSELECT);
            break;
        case 1:
            ad_cmd_reg |= (1<<AD_PSELECT);
            break;
        case 2:
            //TODO
            break;
    }

    AD_FSYNC_LO();
    _delay_us(5);
    ad9833_send(ad_cmd_reg);
    _delay_us(5);
    AD_FSYNC_HI();
}

uint8_t ad9833_get_phase_out(void){
    return ad_phase_out;
}

void ad9833_set_frequency(uint8_t reg, double freq){
    uint32_t freq_reg;
    uint16_t reg_reg; //probably should be renamed...
    freq_reg = AD_FREQ_CALC(freq);
    ad_freq[reg] = freq;

    if (reg==1)
        reg_reg = AD_FREQ1;
    else
        reg_reg = AD_FREQ0;

    AD_FSYNC_LO();
    _delay_us(5);
    ad9833_send((1<<AD_B28) | ad_cmd_reg);
    ad9833_send(reg_reg | (0x3FFF&(uint16_t)(freq_reg>>2 )));
    ad9833_send(reg_reg | (0x3FFF&(uint16_t)(freq_reg>>16)));
    _delay_us(5);
    AD_FSYNC_HI();
}

double ad9833_get_frequency(uint8_t reg){
    return ad_freq[reg];
}
