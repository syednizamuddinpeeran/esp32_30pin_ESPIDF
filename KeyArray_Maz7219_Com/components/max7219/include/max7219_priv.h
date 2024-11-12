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
 * @file max7219_priv.h
 *
 * ESP-IDF driver for MAX7219/MAX7221
 * Serially Interfaced, 8-Digit LED Display Drivers
 *
 * Ported from esp-open-rtos
 *
 * Copyright (c) 2017, 2018 Ruslan V. Uss <unclerus@gmail.com>
 *
 * BSD Licensed as described in the file LICENSE
 */
#ifndef __MAX7219_PRIV_H__
#define __MAX7219_PRIV_H__


const uint64_t numbers_8by8[] = {
  0x007e8181817e0000,
  0x00000001ff410000,
  0x0000718985834100,
  0x00006e9191814200,
  0x0000ff4424140c00,
  0x00008e919191f200,
  0x00004e9191917e00,
  0x0000e09088878000,
  0x00006e9191916e00,
  0x00007e8989897200
} ;

const uint64_t letters_8by8[] = {
    0x003f7f48487f3f00,
  0x00367f49497f7f00,
  0x00226341417f3e00,
  0x003e7f41417f7f00,
  0x00414949497f7f00,
  0x00404848487f7f00,
  0x00266745417f3e00,
  0x007f7f08087f7f00,
  0x0000417f7f410000,
  0x00407e7f41070600,
  0x004163361c7f7f00,
  0x00010101017f7f00,
  0x7f7f3018307f7f00,
  0x7f7f0c18307f7f00,
  0x003e7f41417f3e00,
  0x00387c44447f7f00,
  0x003d7f46427e3c00,
  0x00317b4e4c7f7f00,
  0x00266f49497b3200,
  0x0060407f7f406000,
  0x007f7f01017f7e00,
  0x007c7e03037e7c00,
  0x7f7f060c067f7f00,
  0x63771c081c776300,
  0x0070780f0f787000,
  0x006171594d474300,
  0x000f1f1515170200,
  0x3e66663e06060600,
  0x000a1b11111f0e00,
  0x007f7f09090f0600,
  0x000c1d15151f0e00,
  0x002064447f3f0400,
  0x003e3f25253d1800,
  0x00070f08087f7f00,
  0x0000012f2f010000,
  0x00005e5f01070600,
  0x00111b0e047f7f00,
  0x0000007f7f000000,
  0x1f1f0c070c1f1f00,
  0x000f1f18181f1f00,
  0x000e1f11111f0e00,
  0x00183c24243f3f00,
  0x03013f3f243c1800,
  0x000c1c10101f1f00,
  0x0012151515150900,
  0x0008083f3f080800,
  0x001f1f01011f1e00,
  0x183c666600000000,
  0x1e1f010f011f1e00,
  0x00111b0e0e1b1100,
  0x001e1f05051d1800,
  0x0000191d17130000,
  0x0000000000000000
};

const uint64_t symbols_8by8[] = {
  0x0008083e08080000,
  0x0000080808080000,
  0x082a3e1c3e2a0800,
  0x00060c1830600000,
  0x002333180c666200,
  0x0000141414140000,
  0x0010081810180800,
  0x0810204020100800,
  0x004163361c080000,
  0x0000081c36634100,
  0x0041633e1c000000,
  0x0000001c3e634100,
  0x0041417f7f000000,
  0x0000007f7f414100,
  0x004141773e080000,
  0x0000083e77414100,
  0x0000000000030300,
  0x0000003636000000,
  0x0000003637010000,
  0x000000000e0f0100,
  0x0000307d7d300000,
  0x0030784d4d602000,
  0x0004325a5a423c00,
  0x05237759517f2600,
  0x00242a6b2a120000,
  0x147f7f147f7f1400,
  0x0060700070600000,
  0x0002060c18302000,
  0x0010706000000000,
  0x0000007078080000
};

static const uint8_t font_7seg[] = {
    /*  ' '   !     "     #     $     %     &     '     (     )     */
        0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x02, 0x4e, 0x78,
    /*  *     +     ,     -     .     /     0     1     2     3     */
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x7e, 0x30, 0x6d, 0x79,
    /*  4     5     6     7     8     9     :     ;     <     =     */
        0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b, 0x00, 0x00, 0x0d, 0x09,
    /*  >     ?     @     A     B     C     D     E     F     G     */
        0x19, 0x65, 0x00, 0x77, 0x1f, 0x4e, 0x3d, 0x4f, 0x47, 0x5e,
    /*  H     I     J     K     L     M     N     O     P     Q     */
        0x37, 0x06, 0x38, 0x57, 0x0e, 0x76, 0x15, 0x1d, 0x67, 0x73,
    /*  R     S     T     U     V     W     X     Y     Z     [     */
        0x05, 0x5b, 0x0f, 0x1c, 0x3e, 0x2a, 0x49, 0x3b, 0x6d, 0x4e,
    /*  \     ]     ^     _     `     a     b     c     d     e     */
        0x00, 0x78, 0x00, 0x08, 0x02, 0x77, 0x1f, 0x4e, 0x3d, 0x4f,
    /*  f     g     h     i     j     k     l     m     n     o     */
        0x47, 0x5e, 0x37, 0x06, 0x38, 0x57, 0x0e, 0x76, 0x15, 0x1d,
    /*  p     q     r     s     t     u     v     w     x     y     */
        0x67, 0x73, 0x05, 0x5b, 0x0f, 0x1c, 0x3e, 0x2a, 0x49, 0x3b,
    /*  z     {     |     }     ~     */
        0x6d, 0x4e, 0x06, 0x78, 0x00
};


#endif /* __MAX7219_PRIV_H__ */