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
#include "util-aead.h"
#include "util-load-store.h"
#include "util-rotate.h"
#include <string.h>

aead_cipher_t const gimli24_cipher = {
    "GIMLI-24",
    GIMLI24_KEY_SIZE,
    GIMLI24_NONCE_SIZE,
    GIMLI24_TAG_SIZE,
    AEAD_FLAG_LITTLE_ENDIAN,
    gimli24_aead_encrypt,
    gimli24_aead_decrypt
};

/* Apply the SP-box to a specific column in the state array */
#define GIMLI24_SP(col) \
    do { \
        x = leftRotate24(state[(col)]); \
        y = leftRotate9(state[(col) + 4]); \
        z = state[(col) + 8]; \
        state[(col) + 8] = x ^ (z << 1) ^ ((y & z) << 2); \
        state[(col) + 4] = y ^ x ^ ((x | z) << 1); \
        state[(col)] = z ^ y ^ ((x & y) << 3); \
    } while (0)

/* Internal function, exported to support unit tests and future
 * high-level operations that need access to the raw permutation */
void gimli24_permute(uint32_t state[12])
{
    uint32_t x, y, z;
    unsigned round;

    /* Convert state from little-endian if the platform is not little-endian */
#if !defined(LW_UTIL_LITTLE_ENDIAN)
    for (round = 0; round < 12; ++round)
        state[round] = le_load_word32((const unsigned char *)(&(state[round])));
#endif

    /* Unroll and perform the rounds 4 at a time */
    for (round = 24; round > 0; round -= 4) {
        /* Round 0: SP-box, small swap, add round constant */
        GIMLI24_SP(0);
        GIMLI24_SP(1);
        GIMLI24_SP(2);
        GIMLI24_SP(3);
        x = state[0];
        y = state[2];
        state[0] = state[1] ^ 0x9e377900U ^ round;
        state[1] = x;
        state[2] = state[3];
        state[3] = y;

        /* Round 1: SP-box only */
        GIMLI24_SP(0);
        GIMLI24_SP(1);
        GIMLI24_SP(2);
        GIMLI24_SP(3);

        /* Round 2: SP-box, big swap */
        GIMLI24_SP(0);
        GIMLI24_SP(1);
        GIMLI24_SP(2);
        GIMLI24_SP(3);
        x = state[0];
        y = state[1];
        state[0] = state[2];
        state[1] = state[3];
        state[2] = x;
        state[3] = y;

        /* Round 3: SP-box only */
        GIMLI24_SP(0);
        GIMLI24_SP(1);
        GIMLI24_SP(2);
        GIMLI24_SP(3);
    }

    /* Convert state to little-endian if the platform is not little-endian */
#if !defined(LW_UTIL_LITTLE_ENDIAN)
    for (round = 0; round < 12; ++round)
        le_store_word32(((unsigned char *)(&(state[round]))), state[round]);
#endif
}

/**
 * \brief Number of bytes of input or output data to process per block.
 */
#define GIMLI24_BLOCK_SIZE 16

/**
 * \brief Structure of the GIMLI-24 state as both an array of words
 * and an array of bytes.
 */
typedef union
{
    uint32_t words[12];     /**< Words in the state */
    uint8_t bytes[48];      /**< Bytes in the state */

} gimli24_state_t;

/**
 * \brief Absorbs data into a GIMLI-24 state.
 *
 * \param state The state to absorb the data into.
 * \param data Points to the data to be absorbed.
 * \param len Length of the data to be absorbed.
 */
static void gimli24_absorb
    (gimli24_state_t *state, const unsigned char *data, unsigned long long len)
{
    unsigned temp;
    while (len >= GIMLI24_BLOCK_SIZE) {
        lw_xor_block(state->bytes, data, GIMLI24_BLOCK_SIZE);
        gimli24_permute(state->words);
        data += GIMLI24_BLOCK_SIZE;
        len -= GIMLI24_BLOCK_SIZE;
    }
    temp = (unsigned)len;
    lw_xor_block(state->bytes, data, temp);
    state->bytes[temp] ^= 0x01; /* Padding */
    state->bytes[47] ^= 0x01;
    gimli24_permute(state->words);
}

/**
 * \brief Encrypts a block of data with a GIMLI-24 state.
 *
 * \param state The state to encrypt with.
 * \param dest Points to the destination buffer.
 * \param src Points to the source buffer.
 * \param len Length of the data to encrypt from \a src into \a dest.
 */
static void gimli24_encrypt
    (gimli24_state_t *state, unsigned char *dest,
     const unsigned char *src, unsigned long long len)
{
    unsigned temp;
    while (len >= GIMLI24_BLOCK_SIZE) {
        lw_xor_block_2_dest(dest, state->bytes, src, GIMLI24_BLOCK_SIZE);
        gimli24_permute(state->words);
        dest += GIMLI24_BLOCK_SIZE;
        src += GIMLI24_BLOCK_SIZE;
        len -= GIMLI24_BLOCK_SIZE;
    }
    temp = (unsigned)len;
    lw_xor_block_2_dest(dest, state->bytes, src, temp);
    state->bytes[temp] ^= 0x01; /* Padding */
    state->bytes[47] ^= 0x01;
    gimli24_permute(state->words);
}

/**
 * \brief Decrypts a block of data with a GIMLI-24 state.
 *
 * \param state The state to decrypt with.
 * \param dest Points to the destination buffer.
 * \param src Points to the source buffer.
 * \param len Length of the data to decrypt from \a src into \a dest.
 */
static void gimli24_decrypt
    (gimli24_state_t *state, unsigned char *dest,
     const unsigned char *src, unsigned long long len)
{
    unsigned temp;
    while (len >= GIMLI24_BLOCK_SIZE) {
        lw_xor_block_swap(dest, state->bytes, src, GIMLI24_BLOCK_SIZE);
        gimli24_permute(state->words);
        dest += GIMLI24_BLOCK_SIZE;
        src += GIMLI24_BLOCK_SIZE;
        len -= GIMLI24_BLOCK_SIZE;
    }
    temp = (unsigned)len;
    lw_xor_block_swap(dest, state->bytes, src, temp);
    state->bytes[temp] ^= 0x01; /* Padding */
    state->bytes[47] ^= 0x01;
    gimli24_permute(state->words);
}

int gimli24_aead_encrypt
    (unsigned char *c, unsigned long long *clen,
     const unsigned char *m, unsigned long long mlen,
     const unsigned char *ad, unsigned long long adlen,
     const unsigned char *nsec,
     const unsigned char *npub,
     const unsigned char *k)
{
    gimli24_state_t state;
    (void)nsec;

    /* Set the length of the returned ciphertext */
    *clen = mlen + GIMLI24_TAG_SIZE;

    /* Format the initial GIMLI state from the nonce and the key */
    memcpy(state.words, npub, GIMLI24_NONCE_SIZE);
    memcpy(state.words + 4, k, GIMLI24_KEY_SIZE);

    /* Permute the initial state */
    gimli24_permute(state.words);

    /* Absorb the associated data */
    gimli24_absorb(&state, ad, adlen);

    /* Encrypt the plaintext to produce the ciphertext */
    gimli24_encrypt(&state, c, m, mlen);

    /* Generate the authentication tag at the end of the ciphertext */
    memcpy(c + mlen, state.bytes, GIMLI24_TAG_SIZE);
    return 0;
}

int gimli24_aead_decrypt
    (unsigned char *m, unsigned long long *mlen,
     unsigned char *nsec,
     const unsigned char *c, unsigned long long clen,
     const unsigned char *ad, unsigned long long adlen,
     const unsigned char *npub,
     const unsigned char *k)
{
    gimli24_state_t state;
    (void)nsec;

    /* Validate the ciphertext length and set the return "mlen" value */
    if (clen < GIMLI24_TAG_SIZE)
        return -1;
    *mlen = clen - GIMLI24_TAG_SIZE;

    /* Format the initial GIMLI state from the nonce and the key */
    memcpy(state.words, npub, GIMLI24_NONCE_SIZE);
    memcpy(state.words + 4, k, GIMLI24_KEY_SIZE);

    /* Permute the initial state */
    gimli24_permute(state.words);

    /* Absorb the associated data */
    gimli24_absorb(&state, ad, adlen);

    /* Decrypt the ciphertext to produce the plaintext */
    gimli24_decrypt(&state, m, c, *mlen);

    /* Check the authentication tag at the end of the packet */
    return lw_check_tag(state.bytes, c + *mlen, GIMLI24_TAG_SIZE, 0);
}

int gimli24_hash
    (unsigned char *out, const unsigned char *in, unsigned long long inlen)
{
    gimli24_state_t state;

    /* Initialize the hash state to all zeroes */
    memset(&state, 0, sizeof(state));

    /* Absorb the input */
    gimli24_absorb(&state, in, inlen);

    /* Generate the output hash */
    memcpy(out, state.bytes, GIMLI24_HASH_SIZE / 2);
    gimli24_permute(state.words);
    memcpy(out, state.bytes + GIMLI24_HASH_SIZE / 2, GIMLI24_HASH_SIZE / 2);
    return 0;
}
