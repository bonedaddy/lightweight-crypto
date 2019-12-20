/*
 * Copyright (C) 2019 Southern Storm Software, Pty Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "gimli24.h"
#include "util-load-store.h"
#include "test-cipher.h"
#include <stdio.h>
#include <string.h>

/* Known-answer test vectors for GIMLI-24 NIST submission */
static aead_cipher_test_vector_t const gimli24_1 = {
    "Test Vector 1",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0},                                                /* ad */
    0,                                                  /* ad_len */
    {0x14, 0xDA, 0x9B, 0xB7, 0x12, 0x0B, 0xF5, 0x8B,    /* ciphertext */
     0x98, 0x5A, 0x8E, 0x00, 0xFD, 0xEB, 0xA1, 0x5B},
    {0},                                                /* plaintext */
    0                                                   /* plaintext_len */
};
static aead_cipher_test_vector_t const gimli24_2 = {
    "Test Vector 2",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0},                                                /* ad */
    0,                                                  /* ad_len */
    {0x7F, 0x8A, 0x2C, 0xF4, 0xF5, 0x2A, 0xA4, 0xD6,    /* ciphertext */
     0xB2, 0xE7, 0x41, 0x05, 0xC3, 0x0A, 0x27, 0x77,
     0xB9, 0xB7, 0x50, 0x24, 0x94, 0x52, 0x8B, 0x51,
     0x60, 0xF5, 0xEE, 0x0F, 0x65, 0xC3, 0xA7, 0xB4},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* plaintext */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    16                                                  /* plaintext_len */
};
static aead_cipher_test_vector_t const gimli24_3 = {
    "Test Vector 3",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* ad */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    32,                                                 /* ad_len */
    {0x76, 0x6B, 0x3B, 0x5E, 0x77, 0x88, 0x27, 0x2D,    /* ciphertext */
     0x39, 0xED, 0xAD, 0x2B, 0xCE, 0xBA, 0xF4, 0x16,
     0x06, 0xE6, 0x20, 0x76, 0xA0, 0xFD, 0x14, 0x94,
     0xB9, 0x95, 0x27, 0xBF, 0x45, 0xDC, 0x13, 0x8F,
     0x1A, 0x96, 0x06, 0xDB, 0x25, 0x59, 0x37, 0xB6,
     0x8E, 0x02, 0xFE, 0xC8, 0x3E, 0x2C, 0x54, 0xB9},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* plaintext */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    32                                                  /* plaintext_len */
};

/* Test vectors from https://gimli.cr.yp.to/impl.html for the permutation */
static uint8_t const gimli24_input[48] = {
    0x00, 0x00, 0x00, 0x00, 0xba, 0x79, 0x37, 0x9e,
    0x7a, 0xf3, 0x6e, 0x3c, 0x46, 0x6d, 0xa6, 0xda,
    0x24, 0xe7, 0xdd, 0x78, 0x1a, 0x61, 0x15, 0x17,
    0x2e, 0xdb, 0x4c, 0xb5, 0x66, 0x55, 0x84, 0x53,
    0xc8, 0xcf, 0xbb, 0xf1, 0x5a, 0x4a, 0xf3, 0x8f,
    0x22, 0xc5, 0x2a, 0x2e, 0x26, 0x40, 0x62, 0xcc
};
static uint8_t const gimli24_output[48] = {
    0x5a, 0xc8, 0x11, 0xba, 0x19, 0xd1, 0xba, 0x91,
    0x80, 0xe8, 0x0c, 0x38, 0x68, 0x2c, 0x4c, 0xd2,
    0xea, 0xff, 0xce, 0x3e, 0x1c, 0x92, 0x7a, 0x27,
    0xbd, 0xa0, 0x73, 0x4f, 0xd8, 0x9c, 0x5a, 0xda,
    0xf0, 0x73, 0xb6, 0x84, 0xf7, 0x2f, 0xe5, 0x34,
    0x49, 0xef, 0x2b, 0x9e, 0xd6, 0xb8, 0x1b, 0xf4
};

/* Import the internal permuation function so that we can test it directly */
extern void gimli24_permute(uint32_t state[12]);

static void test_gimli24_permutation(void)
{
    uint32_t state[12];

    printf("    Permutation ... ");
    fflush(stdout);

    memcpy(state, gimli24_input, sizeof(gimli24_input));
    gimli24_permute(state);
    if (memcmp(state, gimli24_output, sizeof(gimli24_output)) != 0) {
        printf("failed\n");
        test_exit_result = 1;
    } else {
        printf("ok\n");
    }
}

void test_gimli24(void)
{
    test_aead_cipher_start(&gimli24_cipher);

    test_aead_cipher(&gimli24_cipher, &gimli24_1);
    test_aead_cipher(&gimli24_cipher, &gimli24_2);
    test_aead_cipher(&gimli24_cipher, &gimli24_3);

    test_gimli24_permutation();

    test_aead_cipher_end(&gimli24_cipher);
}
