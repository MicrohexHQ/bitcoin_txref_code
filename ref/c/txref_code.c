/* Copyright (c) 2017 Jonas Schnelli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "segwit_addr.h"
#include "txref_code.h"

/* the Bech32 human readable part for tx-ref codes */
static const char txref_hrp[] = "tx";

int btc_txref_encode(
    char *output,
    char magic,
    int block_height,
    int tx_pos
) {
    /* ensure we stay in boundaries */
    if (block_height > 0x1FFFFF || tx_pos > 0x1FFF || magic > 0x1F) {
        return -1;
    }

    /* Bech32 requires a array of 5bit chunks */
    uint8_t short_id[8] = {0};

    /* set the magic */
    short_id[0] = magic;

    /* make sure the version bit is 0 */
    short_id[1] &= ~(1 << 0);

    short_id[1] |= ((block_height & 0xF) << 1);
    short_id[2] |= ((block_height & 0x1F0) >> 4);
    short_id[3] |= ((block_height & 0x3E00) >> 9);
    short_id[4] |= ((block_height & 0x7C000) >> 14);
    short_id[5] |= ((block_height & 0x180000) >> 19);

    short_id[5] |= ((tx_pos & 0x7) << 2);
    short_id[6] |= ((tx_pos & 0xF8) >> 3);
    short_id[7] |= ((tx_pos & 0x1F00) >> 8);

    /* Bech32 encode the 8x5bit packages */
    int res = bech32_encode(output, txref_hrp, short_id, 8);
    return res;
}

int btc_txref_decode(
    const char *txref_id,
    char *magic,
    int *block_height,
    int *tx_pos
) {

    /* max 17 chars are allowed for now */
    if (strlen(txref_id) > 17) {
        return -1;
    }

    /* Bech32 decode */
    size_t outlen = 0;
    uint8_t buf[9] = {0};
    char tp[11];
    int res = bech32_decode(tp, buf, &outlen, txref_id);
    if (!res) {
        return res;
    }

    /* make sure the human readable part matches */
    if (strncmp(tp, txref_hrp, 2) != 0) {
        return -1;
    }

    /* set the magic */
    *magic = buf[0];

    /* set the block height */
    *block_height = (buf[1] >> 1);
    *block_height |= (buf[2] << 4);
    *block_height |= (buf[3] << 9);
    *block_height |= (buf[4] << 14);
    *block_height |= ((buf[5] & 0x03) << 19);

    /* set the tx position */
    *tx_pos = ((buf[5] & 0x1C) >> 2);
    *tx_pos |= (buf[6] << 3);
    *tx_pos |= (buf[7] << 8);

    return 1;
}
