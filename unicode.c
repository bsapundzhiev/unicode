/*  unicode.c unicode conversion routines
 *
 * Copyright (C) 2014 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 * based on The Basics of UTF-8 article
 * http://www.codeguru.com/cpp/misc/misc/multi-lingualsupport/article.php/c10451/The-Basics-of-UTF8.htm
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "unicode.h"

#define	MASKBITS		0x3F
#define	MASKBYTE		0x80
#define	MASK2BYTES		0xC0
#define	MASK3BYTES		0xE0
#define	MASK4BYTES		0xF0
#define	MASK5BYTES		0xF8
#define	MASK6BYTES		0xFC

typedef unsigned char	byte;

/*
 * BOM  Encoding |	Representation
 */
#define BOM_UTF_8		0xEFBBBF00
#define BOM_UTF_16BE	0xFEFF0000
#define BOM_UTF_16LE	0xFFFE0000
#define BOM_UTF_32BE	0x0000FEFF
#define BOM_UTF_32LE	0xFFFE0000

#define LOW(x) (x & 0xFFFF)
#define HIGH(x) (x >> 16)
#define SWAP_UINT16(x) (((x) >> 8) | ((x) << 8))
#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

ByteOrder UTF8detectBom(unsigned char bom[4])
{
    int i;
    unsigned int boms[] = { BOM_UTF_8, BOM_UTF_16LE, BOM_UTF_16BE, BOM_UTF_32LE, BOM_UTF_32BE };
    unsigned int x = (bom[0] << 24) | (bom[1] << 16) | (bom[2] << 8) | bom[3] ;

    for (i =0; i < sizeof(boms) / sizeof(boms[0]); i++) {
        //utf_8
        if (boms[i]==UTF_8BOM && LOW(boms[i]) == (LOW(x) >> 8) && HIGH(x) == HIGH(boms[i]))
            return i + 1;
        //utf_16
        else if (HIGH(boms[i]) == HIGH(x) && LOW(x) != LOW(boms[i]))
            return i + 1;
        //utf_32
        else if (i > 2 && HIGH(boms[i]) == HIGH(x) && LOW(x) == LOW(boms[i]))
            return i + 1;
    }

    return UTF_8;
}

char *UTF8EncodeUTF16(utf16_t *input, size_t size, ByteOrder order)
{
    char* result;
    size_t i, cindex = 0;
    result = (char*) malloc(size  * sizeof(utf16_t) +1 );
    if (!result) return NULL;

    for (i=0; i < size; i++) {

        if (order == UTF_16BE)
            input[i] = SWAP_UINT16(input[i]);

        // 0xxxxxxx
        if (input[i] < 0x80) {
            result[cindex++] = ((char) input[i]);
        }
        // 110xxxxx 10xxxxxx
        else if (input[i] < 0x800) {
            result[cindex++] = ((byte)(MASK2BYTES | input[i] >> 6));
            result[cindex++] = ((byte)(MASKBYTE | (input[i] & MASKBITS)));
        }
        // 1110xxxx 10xxxxxx 10xxxxxx
        else if (input[i] < 0x10000) {
            result[cindex++] = ((byte)(MASK3BYTES | input[i] >> 12));
            result[cindex++] = ((byte)(MASKBYTE | (input[i] >> 6 & MASKBITS) ));
            result[cindex++] = ((byte)(MASKBYTE | (input[i] & MASKBITS) ));
        }
    }

    result[cindex] ='\0';
    return result;
}

utf16_t* UTF8DecodeUTF16(const char *input, ByteOrder order)
{
    int size = strlen(input);
    int rindex = 0, windex = 0;

    utf16_t *result = (utf16_t*) malloc(size * sizeof(utf16_t));
    if (!result) return NULL;

    while (rindex < size) {
        utf16_t ch;
        // 1110xxxx 10xxxxxx 10xxxxxx
        if ((input[rindex] & MASK3BYTES) == MASK3BYTES) {
            ch = ((input[rindex] & 0x0F) << 12) | ((input[rindex+1] & MASKBITS) << 6)
                 | (input[rindex+2] & MASKBITS);
            rindex += 3;
        }
        // 110xxxxx 10xxxxxx
        else if ((input[rindex] & MASK2BYTES) == MASK2BYTES) {
            ch = ((input[rindex] & 0x1F) << 6) | (input[rindex+1] & MASKBITS);
            rindex += 2;
        }
        // 0xxxxxxx
        else if (input[rindex] < MASKBYTE) {
            ch = input[rindex];
            rindex += 1;
        }

        result[windex] = (order == UTF_16LE) ? ch : SWAP_UINT16(ch);
    }

    result[windex] = '\0';
    return result;
}

char* UTF8EncodeUTF32(utf32_t *input, size_t size, ByteOrder order)
{
    char* result;
    size_t i, cindex = 0;
    result = (char*) malloc( size * sizeof(utf32_t) + 1);
    if (!result) return NULL;

    for (i=0; i < size; i++) {

        if (order == UTF_32BE)
            input[i] = SWAP_UINT32(input[i]);

        // 0xxxxxxx
        if (input[i] < 0x80) {
            result[cindex++]= ((byte)input[i]);
        }
        // 110xxxxx 10xxxxxx
        else if (input[i] < 0x800) {
            result[cindex++]= ((byte)(MASK2BYTES | input[i] >> 6 ));
            result[cindex++]= ((byte)(MASKBYTE | (input[i] & MASKBITS) ));
        }
        // 1110xxxx 10xxxxxx 10xxxxxx
        else if (input[i] < 0x10000) {
            result[cindex++]= ((byte)(MASK3BYTES | input[i] >> 12));
            result[cindex++]= ((byte)(MASKBYTE | (input[i] >> 6 & MASKBITS) ));
            result[cindex++]= ((byte)(MASKBYTE | (input[i] & MASKBITS) ));
        }
        // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if (input[i] < 0x200000) {
            result[cindex++]= ((byte)(MASK4BYTES | input[i] >> 18));
            result[cindex++]= ((byte)(MASKBYTE | (input[i] >> 12 & MASKBITS) ));
            result[cindex++]= ((byte)(MASKBYTE | (input[i] >> 6 & MASKBITS) ));
            result[cindex++]= ((byte)(MASKBYTE | (input[i] & MASKBITS) ));
        }
        // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if (input[i] < 0x4000000) {
            result[cindex++]=((byte)(MASK5BYTES | input[i] >> 24));
            result[cindex++]=((byte)(MASKBYTE | (input[i] >> 18 & MASKBITS) ));
            result[cindex++]=((byte)(MASKBYTE | (input[i] >> 12 & MASKBITS) ));
            result[cindex++]=((byte)(MASKBYTE | (input[i] >> 6 & MASKBITS) ));
            result[cindex++]=((byte)(MASKBYTE | (input[i] & MASKBITS) ));
        }
        // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if (input[i] < 0x8000000) {
            result[cindex++]=((byte)(MASK6BYTES | input[i] >> 30));
            result[cindex++]=((byte)(MASKBYTE | (input[i] >> 18 & MASKBITS) ));
            result[cindex++]=((byte)(MASKBYTE | (input[i] >> 12 & MASKBITS) ));
            result[cindex++]=((byte)(MASKBYTE | (input[i] >> 6 & MASKBITS) ));
            result[cindex++]=((byte)(MASKBYTE | (input[i] & MASKBITS) ));
        }
    }

    result[cindex]= '\0';
    return result;
}

utf32_t * UTF8DecodeUTF32(const char* input, ByteOrder order)
{
    int cindex =0;
    int i=0, size = strlen(input);
    utf32_t* result = (utf32_t*) malloc(size*sizeof(utf32_t));
    if (!result) return 0;

    while ( i < size ) {
        utf32_t ch;

        // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        if ((input[i] & MASK6BYTES) == MASK6BYTES) {
            ch = ((input[i] & 0x01) << 30) | ((input[i+1] & MASKBITS) << 24)
                 | ((input[i+2] & MASKBITS) << 18) | ((input[i+3]  & MASKBITS) << 12)
                 | ((input[i+4] & MASKBITS) << 6) | (input[i+5] & MASKBITS);
            i += 6;
        }
        // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if ((input[i] & MASK5BYTES) == MASK5BYTES) {
            ch = ((input[i] & 0x03) << 24) | ((input[i+1] & MASKBITS) << 18)
                 | ((input[i+2] & MASKBITS) << 12) | ((input[i+3] & MASKBITS) << 6)
                 | (input[i+4] & MASKBITS);
            i += 5;
        }
        // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        else if ((input[i] & MASK4BYTES) == MASK4BYTES) {
            ch = ((input[i] & 0x07) << 18) | ((input[i+1] & MASKBITS) << 12)
                 | ((input[i+2] & MASKBITS) << 6) | (input[i+3] & MASKBITS);
            i += 4;
        }
        // 1110xxxx 10xxxxxx 10xxxxxx
        else if ((input[i] & MASK3BYTES) == MASK3BYTES) {
            ch = ((input[i] & 0x0F) << 12) | ((input[i+1] & MASKBITS) << 6)
                 | (input[i+2] & MASKBITS);
            i += 3;
        }
        // 110xxxxx 10xxxxxx
        else if ((input[i] & MASK2BYTES) == MASK2BYTES) {
            ch = ((input[i] & 0x1F) << 6) | (input[i+1] & MASKBITS);
            i += 2;
        }
        // 0xxxxxxx
        else if (input[i] < MASKBYTE) {
            ch = input[i];
            i += 1;
        }

        result[cindex++] = (order == UTF_32LE) ? ch : SWAP_UINT32(ch);
    }

    result[cindex]='\0';
    return result;
}
