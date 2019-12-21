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

#include "ascon128.h"
#include "test-cipher.h"
#include <stdio.h>
#include <string.h>

/* Known-answer test vectors for the ASCON-128 NIST submission */
static aead_cipher_test_vector_t const ascon128_1 = {
    "Test Vector 1",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0},                                                /* ad */
    0,                                                  /* ad_len */
    {0xE3, 0x55, 0x15, 0x9F, 0x29, 0x29, 0x11, 0xF7,    /* ciphertext */
     0x94, 0xCB, 0x14, 0x32, 0xA0, 0x10, 0x3A, 0x8A},
    {0},                                                /* plaintext */
    0                                                   /* plaintext_len */
};
static aead_cipher_test_vector_t const ascon128_2 = {
    "Test Vector 2",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0},                                                /* ad */
    0,                                                  /* ad_len */
    {0xBC, 0x82, 0x0D, 0xBD, 0xF7, 0xA4, 0x63, 0x1C,    /* ciphertext */
     0x5B, 0x29, 0x88, 0x4A, 0xD6, 0x91, 0x75, 0xC3,
     0xF5, 0x8E, 0x28, 0x43, 0x6D, 0xD7, 0x15, 0x56,
     0xD5, 0x8D, 0xFA, 0x56, 0xAC, 0x89, 0x0B, 0xEB},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* plaintext */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    16                                                  /* plaintext_len */
};
static aead_cipher_test_vector_t const ascon128_3 = {
    "Test Vector 3",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* ad */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    32,                                                 /* ad_len */
    {0xB9, 0x6C, 0x78, 0x65, 0x1B, 0x62, 0x46, 0xB0,    /* ciphertext */
     0xC3, 0xB1, 0xA5, 0xD3, 0x73, 0xB0, 0xD5, 0x16,
     0x8D, 0xCA, 0x4A, 0x96, 0x73, 0x4C, 0xF0, 0xDD,
     0xF5, 0xF9, 0x2F, 0x8D, 0x15, 0xE3, 0x02, 0x70,
     0x27, 0x9B, 0xF6, 0xA6, 0xCC, 0x3F, 0x2F, 0xC9,
     0x35, 0x0B, 0x91, 0x5C, 0x29, 0x2B, 0xDB, 0x8D},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* plaintext */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    32                                                  /* plaintext_len */
};

/* Known-answer test vectors for the ASCON-128a NIST submission */
static aead_cipher_test_vector_t const ascon128a_1 = {
    "Test Vector 1",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0},                                                /* ad */
    0,                                                  /* ad_len */
    {0x7A, 0x83, 0x4E, 0x6F, 0x09, 0x21, 0x09, 0x57,    /* ciphertext */
     0x06, 0x7B, 0x10, 0xFD, 0x83, 0x1F, 0x00, 0x78},
    {0},                                                /* plaintext */
    0                                                   /* plaintext_len */
};
static aead_cipher_test_vector_t const ascon128a_2 = {
    "Test Vector 2",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0},                                                /* ad */
    0,                                                  /* ad_len */
    {0x6E, 0x49, 0x0C, 0xFE, 0xD5, 0xB3, 0x54, 0x67,    /* ciphertext */
     0x67, 0x35, 0x0C, 0xD8, 0x3C, 0x4A, 0xCF, 0xBD,
     0xB1, 0x0F, 0x61, 0x1B, 0x7D, 0x79, 0x27, 0x8B,
     0xD8, 0x06, 0x7F, 0xC1, 0xBC, 0xDF, 0x39, 0xBE},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* plaintext */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    16                                                  /* plaintext_len */
};
static aead_cipher_test_vector_t const ascon128a_3 = {
    "Test Vector 3",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* ad */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    32,                                                 /* ad_len */
    {0xA5, 0x52, 0x36, 0xAC, 0x02, 0x0D, 0xBD, 0xA7,    /* ciphertext */
     0x4C, 0xE6, 0xCC, 0xD1, 0x0C, 0x68, 0xC4, 0xD8,
     0x51, 0x44, 0x50, 0xA3, 0x82, 0xBC, 0x87, 0xC6,
     0x89, 0x46, 0xD8, 0x6A, 0x92, 0x1D, 0xD8, 0x8E,
     0x2A, 0xDD, 0xDF, 0xBB, 0xE7, 0x7D, 0x41, 0x12,
     0x83, 0x0E, 0x01, 0x96, 0x0B, 0x9D, 0x38, 0xD5},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* plaintext */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    32                                                  /* plaintext_len */
};

/* Known-answer test vectors for the ASCON-80pq NIST submission */
static aead_cipher_test_vector_t const ascon80pq_1 = {
    "Test Vector 1",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0},                                                /* ad */
    0,                                                  /* ad_len */
    {0xAB, 0xB6, 0x88, 0xEF, 0xA0, 0xB9, 0xD5, 0x6B,    /* ciphertext */
     0x33, 0x27, 0x7A, 0x2C, 0x97, 0xD2, 0x14, 0x6B},
    {0},                                                /* plaintext */
    0                                                   /* plaintext_len */
};
static aead_cipher_test_vector_t const ascon80pq_2 = {
    "Test Vector 2",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0},                                                /* ad */
    0,                                                  /* ad_len */
    {0x28, 0x46, 0x41, 0x80, 0x67, 0xCE, 0x93, 0x86,    /* ciphertext */
     0xB4, 0x7F, 0x05, 0x84, 0xBF, 0x9E, 0xEE, 0x3F,
     0x81, 0x8C, 0xA2, 0xB2, 0x64, 0xF3, 0xBB, 0xFC,
     0x40, 0xB7, 0x73, 0xD0, 0xEB, 0x81, 0xF5, 0x94},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* plaintext */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    16                                                  /* plaintext_len */
};
static aead_cipher_test_vector_t const ascon80pq_3 = {
    "Test Vector 3",
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* key */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* nonce */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* ad */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    32,                                                 /* ad_len */
    {0xCC, 0x4E, 0x07, 0xE5, 0xFB, 0x13, 0x42, 0x6E,    /* ciphertext */
     0xFF, 0xD1, 0x7B, 0x0F, 0x51, 0xA6, 0xA8, 0x30,
     0xBF, 0x48, 0x4C, 0x96, 0x51, 0xD7, 0x76, 0x79,
     0x97, 0x1E, 0x8E, 0xB4, 0xA8, 0xED, 0xB5, 0xA0,
     0x07, 0x82, 0xA9, 0x4C, 0x72, 0xB2, 0xB0, 0x2D,
     0x87, 0xDC, 0xF4, 0xAF, 0x75, 0xDB, 0x69, 0x96},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* plaintext */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F},
    32                                                  /* plaintext_len */
};

/* Known-answer test vectors for the ASCON-HASH NIST submission */
static aead_hash_test_vector_t const ascon_hash_1 = {
    "Test Vector 1",
    {0x73, 0x46, 0xBC, 0x14, 0xF0, 0x36, 0xE8, 0x7A,    /* hash */
     0xE0, 0x3D, 0x09, 0x97, 0x91, 0x30, 0x88, 0xF5,
     0xF6, 0x84, 0x11, 0x43, 0x4B, 0x3C, 0xF8, 0xB5,
     0x4F, 0xA7, 0x96, 0xA8, 0x0D, 0x25, 0x1F, 0x91},
    {0},                                                /* input */
    0                                                   /* input_len */
};
static aead_hash_test_vector_t const ascon_hash_2 = {
    "Test Vector 2",
    {0x9C, 0x52, 0x14, 0x28, 0x52, 0xBE, 0xB6, 0x65,
     0x49, 0x07, 0xCC, 0x23, 0xCC, 0x5B, 0x17, 0x10,
     0x75, 0xD4, 0x11, 0xCA, 0x80, 0x08, 0x2A, 0xAF,
     0xD7, 0xDD, 0x0D, 0x09, 0xBA, 0x0B, 0xBA, 0x1D},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},               /* input */
    6                                                   /* input_len */
};
static aead_hash_test_vector_t const ascon_hash_3 = {
    "Test Vector 3",
    {0x2C, 0xB1, 0x46, 0xAE, 0xBB, 0xB6, 0x58, 0x5B,    /* hash */
     0x11, 0xBF, 0x1A, 0x37, 0x1B, 0xAA, 0x6E, 0x3E,
     0x55, 0x10, 0x8C, 0x69, 0xB0, 0x83, 0x4F, 0x26,
     0x9F, 0x66, 0x2C, 0x59, 0xBC, 0xAA, 0x57, 0x00},
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* input */
     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
     0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E},
    31                                                  /* input_len */
};

void test_ascon128(void)
{
    test_aead_cipher_start(&ascon128_cipher);
    test_aead_cipher(&ascon128_cipher, &ascon128_1);
    test_aead_cipher(&ascon128_cipher, &ascon128_2);
    test_aead_cipher(&ascon128_cipher, &ascon128_3);
    test_aead_cipher_end(&ascon128_cipher);

    test_aead_cipher_start(&ascon128a_cipher);
    test_aead_cipher(&ascon128a_cipher, &ascon128a_1);
    test_aead_cipher(&ascon128a_cipher, &ascon128a_2);
    test_aead_cipher(&ascon128a_cipher, &ascon128a_3);
    test_aead_cipher_end(&ascon128a_cipher);

    test_aead_cipher_start(&ascon80pq_cipher);
    test_aead_cipher(&ascon80pq_cipher, &ascon80pq_1);
    test_aead_cipher(&ascon80pq_cipher, &ascon80pq_2);
    test_aead_cipher(&ascon80pq_cipher, &ascon80pq_3);
    test_aead_cipher_end(&ascon80pq_cipher);

    test_hash_start(&ascon_hash_algorithm);
    test_hash(&ascon_hash_algorithm, &ascon_hash_1);
    test_hash(&ascon_hash_algorithm, &ascon_hash_2);
    test_hash(&ascon_hash_algorithm, &ascon_hash_3);
    test_hash_end(&ascon_hash_algorithm);
}
