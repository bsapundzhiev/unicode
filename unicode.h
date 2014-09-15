/*  unicode.c unicode conversion routines
 *
 * Copyright (C) 2014 Borislav Sapundzhiev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 */
#ifndef _UNICODE_H
#define _UNICODE_H

typedef unsigned short	utf16_t;
typedef unsigned int	utf32_t;

typedef enum {UTF_8, UTF_8BOM, UTF_16LE, UTF_16BE, UTF_32LE, UTF_32BE} ByteOrder;
unsigned int UTF8GetBom(ByteOrder order);
ByteOrder UTF8detectBom(const unsigned int bom);
char *UTF8EncodeUTF16(utf16_t* input, size_t size, ByteOrder order);
utf32_t *UTF8DecodeUTF32(const char *input, ByteOrder order);
char* UTF8EncodeUTF32(utf32_t *input, size_t size, ByteOrder order);
utf16_t* UTF8DecodeUTF16(const char *input, ByteOrder order);

#endif