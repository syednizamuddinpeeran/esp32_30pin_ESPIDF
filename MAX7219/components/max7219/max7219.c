/*
 * Copyright (c) 2019 Ruslan V. Uss <unclerus@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of itscontributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file max7219.c
 *
 * ESP-IDF driver for MAX7219/MAX7221
 * Serially Interfaced, 8-Digit LED Display Drivers
 *
 * Ported from esp-open-rtos
 *
 * Copyright (c) 2017 Ruslan V. Uss <unclerus@gmail.com>
 *
 * BSD Licensed as described in the file LICENSE
 */
#include "max7219.h"
#include <string.h>
#include <esp_log.h>

#include "max7219_priv.h"

static const char *TAG = "max7219";

#define ALL_CHIPS 0xff
#define ALL_DIGITS 8

#define REG_DIGIT_0      (1 << 8)
#define REG_DECODE_MODE  (9 << 8)
#define REG_INTENSITY    (10 << 8)
#define REG_SCAN_LIMIT   (11 << 8)
#define REG_SHUTDOWN     (12 << 8)
#define REG_DISPLAY_TEST (15 << 8)

#define VAL_CLEAR_BCD    0x0f
#define VAL_CLEAR_NORMAL 0x00

#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)
#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

static inline uint16_t shuffle(uint16_t val)
{
    return (val >> 8) | (val << 8);
}

static esp_err_t send(max7219_t *dev, uint8_t chip, uint16_t value)
{
    uint16_t buf[MAX7219_MAX_CASCADE_SIZE] = { 0 };
    if (chip == ALL_CHIPS)
    {
        for (uint8_t i = 0; i < dev->cascade_size; i++)
            buf[i] = shuffle(value);
    }
    else buf[chip] = shuffle(value);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = dev->cascade_size * 16;
    t.tx_buffer = buf;
    return spi_device_transmit(dev->spi_dev, &t);
}


inline static uint8_t get_char(max7219_t *dev, char c)
{
    if (dev->bcd)
    {
        if (c >= '0' && c <= '9')
            return c - '0';
        switch (c)
        {
            case '-':
                return 0x0a;
            case 'E':
            case 'e':
                return 0x0b;
            case 'H':
            case 'h':
                return 0x0c;
            case 'L':
            case 'l':
                return 0x0d;
            case 'P':
            case 'p':
                return 0x0e;
        }
        return VAL_CLEAR_BCD;
    }

    return font_7seg[(c - 0x20) & 0x7f];
}

///////////////////////////////////////////////////////////////////////////////

esp_err_t max7219_init_desc(max7219_t *dev, spi_host_device_t host, uint32_t clock_speed_hz, gpio_num_t cs_pin)
{
    CHECK_ARG(dev);

    memset(&dev->spi_cfg, 0, sizeof(dev->spi_cfg));
    dev->spi_cfg.spics_io_num = cs_pin;
    dev->spi_cfg.clock_speed_hz = clock_speed_hz;
    dev->spi_cfg.mode = 0;
    dev->spi_cfg.queue_size = 1;
    dev->spi_cfg.flags = SPI_DEVICE_NO_DUMMY;

    return spi_bus_add_device(host, &dev->spi_cfg, &dev->spi_dev);
}

esp_err_t max7219_free_desc(max7219_t *dev)
{
    CHECK_ARG(dev);

    return spi_bus_remove_device(dev->spi_dev);
}

esp_err_t max7219_init(max7219_t *dev)
{
    CHECK_ARG(dev);
    if (!dev->cascade_size || dev->cascade_size > MAX7219_MAX_CASCADE_SIZE)
    {
        ESP_LOGE(TAG, "Invalid cascade size %d", dev->cascade_size);
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t max_digits = dev->cascade_size * ALL_DIGITS;
    if (dev->digits > max_digits)
    {
        ESP_LOGE(TAG, "Invalid digits count %d, max %d", dev->digits, max_digits);
        return ESP_ERR_INVALID_ARG;
    }
    if (!dev->digits)
        dev->digits = max_digits;

    // Shutdown all chips
    CHECK(max7219_set_shutdown_mode(dev, true));
    // Disable test
    CHECK(send(dev, ALL_CHIPS, REG_DISPLAY_TEST));
    // Set max scan limit
    CHECK(send(dev, ALL_CHIPS, REG_SCAN_LIMIT | (ALL_DIGITS - 1)));
    // Set normal decode mode & clear display
    CHECK(max7219_set_decode_mode(dev, false));
    // Set minimal brightness
    CHECK(max7219_set_brightness(dev, 0));
    // Wake up
    CHECK(max7219_set_shutdown_mode(dev, false));

    return ESP_OK;
}

esp_err_t max7219_set_decode_mode(max7219_t *dev, bool bcd)
{
    CHECK_ARG(dev);

    dev->bcd = bcd;
    CHECK(send(dev, ALL_CHIPS, REG_DECODE_MODE | (bcd ? 0xff : 0)));
    CHECK(max7219_clear(dev));

    return ESP_OK;
}

esp_err_t max7219_set_brightness(max7219_t *dev, uint8_t value)
{
    CHECK_ARG(dev);
    CHECK_ARG(value <= MAX7219_MAX_BRIGHTNESS);

    CHECK(send(dev, ALL_CHIPS, REG_INTENSITY | value));

    return ESP_OK;
}

esp_err_t max7219_set_shutdown_mode(max7219_t *dev, bool shutdown)
{
    CHECK_ARG(dev);

    CHECK(send(dev, ALL_CHIPS, REG_SHUTDOWN | !shutdown));

    return ESP_OK;
}

esp_err_t max7219_set_digit(max7219_t *dev, uint8_t digit, uint8_t val)
{
    CHECK_ARG(dev);
    if (digit >= dev->digits)
    {
        ESP_LOGE(TAG, "Invalid digit: %d", digit);
        return ESP_ERR_INVALID_ARG;
    }

    if (dev->mirrored)
        digit = dev->digits - digit - 1;

    uint8_t c = digit / ALL_DIGITS;
    uint8_t d = digit % ALL_DIGITS;

    ESP_LOGV(TAG, "Chip %d, digit %d val 0x%02x", c, d, val);

    CHECK(send(dev, c, (REG_DIGIT_0 + ((uint16_t)d << 8)) | val));

    return ESP_OK;
}

esp_err_t max7219_clear(max7219_t *dev)
{
    CHECK_ARG(dev);

    uint8_t val = dev->bcd ? VAL_CLEAR_BCD : VAL_CLEAR_NORMAL;
    for (uint8_t i = 0; i < ALL_DIGITS; i++)
        CHECK(send(dev, ALL_CHIPS, (REG_DIGIT_0 + ((uint16_t)i << 8)) | val));

    return ESP_OK;
}

esp_err_t max7219_draw_text_7seg(max7219_t *dev, uint8_t pos, const char *s)
{
    CHECK_ARG(dev && s);

    while (s && pos < dev->digits)
    {
        uint8_t c = get_char(dev, *s);
        if (*(s + 1) == '.')
        {
            c |= 0x80;
            s++;
        }
        CHECK(max7219_set_digit(dev, pos, c));
        pos++;
        s++;
    }

    return ESP_OK;
}

esp_err_t max7219_draw_image_8x8(max7219_t *dev, uint8_t pos, const void *image)
{
    CHECK_ARG(dev && image);

    for (uint8_t i = pos, offs = 0; i < dev->digits && offs < 8; i++, offs++)
        max7219_set_digit(dev, i, *((uint8_t *)image + offs));

    return ESP_OK;
}

static uint64_t* get_char_imageMap(char c)
{
    switch (c)
    {
        case '0':
            return (uint64_t *)numbers_8by8;
        case '1':
            return (uint64_t *)numbers_8by8 + 1;
        case '2':
            return (uint64_t *)numbers_8by8 + 2;
        case '3':
            return (uint64_t *)numbers_8by8 + 3;
        case '4':
            return (uint64_t *)numbers_8by8 + 4;
        case '5':
            return (uint64_t *)numbers_8by8 + 5;
        case '6':
            return (uint64_t *)numbers_8by8 + 6;
        case '7':
            return (uint64_t *)numbers_8by8 + 7;
        case '8':
            return (uint64_t *)numbers_8by8 + 8;
        case '9':
            return (uint64_t *)numbers_8by8 + 9;
        case 'A':
            return (uint64_t *)letters_8by8;
        case 'B':
            return (uint64_t *)letters_8by8 + 1;
        case 'C':
            return (uint64_t *)letters_8by8 + 2;
        case 'D':
            return (uint64_t *)letters_8by8 + 3;
        case 'E':
            return (uint64_t *)letters_8by8 + 4;
        case 'F':
            return (uint64_t *)letters_8by8 + 5;
        case 'G':
            return (uint64_t *)letters_8by8 + 6;
        case 'H':
            return (uint64_t *)letters_8by8 + 7;
        case 'I':
            return (uint64_t *)letters_8by8 + 8;
        case 'J':
            return (uint64_t *)letters_8by8 + 9;
        case 'K':
            return (uint64_t *)letters_8by8 + 10;
        case 'L':
            return (uint64_t *)letters_8by8 + 11;
        case 'M':
            return (uint64_t *)letters_8by8 + 12;
        case 'N':
            return (uint64_t *)letters_8by8 + 13;
        case 'O':
            return (uint64_t *)letters_8by8 + 14;
        case 'P':
            return (uint64_t *)letters_8by8 + 15;
        case 'Q':
            return (uint64_t *)letters_8by8 + 16;
        case 'R':
            return (uint64_t *)letters_8by8 + 17;
        case 'S':
            return (uint64_t *)letters_8by8 + 18;
        case 'T':
            return (uint64_t *)letters_8by8 + 19;
        case 'U':
            return (uint64_t *)letters_8by8 + 20;
        case 'V':
            return (uint64_t *)letters_8by8 + 21;
        case 'W':
            return (uint64_t *)letters_8by8 + 22;
        case 'X':
            return (uint64_t *)letters_8by8 + 23;
        case 'Y':
            return (uint64_t *)letters_8by8 + 24;
        case 'Z':
            return (uint64_t *)letters_8by8 + 25;
        case 'a':
            return (uint64_t *)letters_8by8 + 26;
        case 'b':
            return (uint64_t *)letters_8by8 + 27;
        case 'c':
            return (uint64_t *)letters_8by8 + 28;
        case 'd':
            return (uint64_t *)letters_8by8 + 29;
        case 'e':
            return (uint64_t *)letters_8by8 + 30;
        case 'f':
            return (uint64_t *)letters_8by8 + 31;
        case 'g':
            return (uint64_t *)letters_8by8 + 32;
        case 'h':
            return (uint64_t *)letters_8by8 + 33;
        case 'i':
            return (uint64_t *)letters_8by8 + 34;
        case 'j':
            return (uint64_t *)letters_8by8 + 35;
        case 'k':
            return (uint64_t *)letters_8by8 + 36;
        case 'l':
            return (uint64_t *)letters_8by8 + 37;
        case 'm':
            return (uint64_t *)letters_8by8 + 38;
        case 'n':
            return (uint64_t *)letters_8by8 + 39;
        case 'o':
            return (uint64_t *)letters_8by8 + 40;
        case 'p':
            return (uint64_t *)letters_8by8 + 41;
        case 'q':
            return (uint64_t *)letters_8by8 + 42;
        case 'r':
            return (uint64_t *)letters_8by8 + 43;
        case 's':
            return (uint64_t *)letters_8by8 + 44;
        case 't':
            return (uint64_t *)letters_8by8 + 45;
        case 'u':
            return (uint64_t *)letters_8by8 + 46;
        case 'v':
            return (uint64_t *)letters_8by8 + 47;
        case 'w':
            return (uint64_t *)letters_8by8 + 48;
        case 'x':
            return (uint64_t *)letters_8by8 + 49;
        case 'y':
            return (uint64_t *)letters_8by8 + 50;
        case 'z':
            return (uint64_t *)letters_8by8 + 51;
        case ' ':
            return (uint64_t *)letters_8by8 + 52;
        case '+':
            return (uint64_t *)symbols_8by8;
        case '-':
            return (uint64_t *)symbols_8by8 + 1;
        case '*':
            return (uint64_t *)symbols_8by8 + 2;
        case '/':
            return (uint64_t *)symbols_8by8 + 3;
        case '%':
            return (uint64_t *)symbols_8by8 + 4;
        case '=':
            return (uint64_t *)symbols_8by8 + 5;
        case '~':
            return (uint64_t *)symbols_8by8 + 6;
        case '^':
            return (uint64_t *)symbols_8by8 + 7;
        case '<':
            return (uint64_t *)symbols_8by8 + 8;
        case '>':
            return (uint64_t *)symbols_8by8 + 9;
        case '(':
            return (uint64_t *)symbols_8by8 + 10;
        case ')':
            return (uint64_t *)symbols_8by8 + 11;
        case '[':
            return (uint64_t *)symbols_8by8 + 12;
        case ']':
            return (uint64_t *)symbols_8by8 + 13;
        case '{':
            return (uint64_t *)symbols_8by8 + 14;
        case '}':
            return (uint64_t *)symbols_8by8 + 15;
        case '.':
            return (uint64_t *)symbols_8by8 + 16;
        case ':':
            return (uint64_t *)symbols_8by8 + 17;
        case ';':
            return (uint64_t *)symbols_8by8 + 18;
        case ',':
            return (uint64_t *)symbols_8by8 + 19;
        case '!':
            return (uint64_t *)symbols_8by8 + 20;
        case '?':
            return (uint64_t *)symbols_8by8 + 21;
        case '@':
            return (uint64_t *)symbols_8by8 + 22;
        case '&':
            return (uint64_t *)symbols_8by8 + 23;
        case '$':
            return (uint64_t *)symbols_8by8 + 24;
        case '#':
            return (uint64_t *)symbols_8by8 + 25;
        case '"':
            return (uint64_t *)symbols_8by8 + 26;
        case '\\':
            return (uint64_t *)symbols_8by8 + 27;
        case '\'':
            return (uint64_t *)symbols_8by8 + 28;
        case '`':
            return (uint64_t *)symbols_8by8 + 29;        
        default:
            return (uint64_t *)letters_8by8 + 52; // Default case
    }
}

esp_err_t max7219_draw_char_8x8(max7219_t *dev, uint8_t pos, char c)
{
    CHECK_ARG(dev && c);
    return max7219_draw_image_8x8(dev,pos,get_char_imageMap(c));
}

esp_err_t max7219_draw_string_8x8(max7219_t *dev,char s[])
{
    CHECK_ARG(dev && s);
    int length = strlen(s);
    uint64_t images[length];
    uint64_t *value;
    for (int i = 0; i < length; i++) {
         value = get_char_imageMap(s[i]);
         images[i] = (uint64_t)*value;
    }
    size_t offs = 0;
    size_t symbols_size = sizeof(images) - sizeof(uint64_t) * dev->cascade_size;
    for (uint8_t j=0; j< (length*8); j++)
    {
        for (uint8_t i=0; i< dev->cascade_size; i++)
        {
            max7219_draw_image_8x8(dev,i*8,(uint8_t *)images + i*8 + offs);
        } 
        vTaskDelay(pdMS_TO_TICKS(100));
        if (++offs == symbols_size)
        {
            offs = 0;
            break;
        }
    }
    return ESP_OK;
}