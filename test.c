#if (_WIN32)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "unicode.h"

void unicode_test_file(const char* fileName, ByteOrder order)
{
    FILE *file;
    char *buffer, *result = NULL;
    unsigned long fileLen, offset=0;
    unsigned char bytesize = 1;
	unsigned int bom;
    ByteOrder myorder = 0;

    file = fopen(fileName, "rb");
    if (!file) {
        fprintf(stderr, "can't open file %s", fileName);
        exit(1);
    }

    printf("reading file %s\n", fileName);
    fread(&bom, 1, 4, file);
    myorder = UTF8detectBom(bom);
    switch (myorder) {
    case UTF_8:
        offset = 0;
        break;
    case UTF_8BOM:
        offset = 3;
        break;
    case UTF_16LE:
    case UTF_16BE:
        bytesize = 2;
        offset = 2 ;
        break;
    case UTF_32LE:
    case UTF_32BE:
        bytesize = 4;
        offset = 4;
        break;
    }

    fseek(file, offset, SEEK_END);
    fileLen = ftell(file);
    fseek(file, offset, SEEK_SET);

    buffer =(char*) malloc(fileLen + 1);
    if (!buffer) {
        fprintf(stderr, "Memory error!");
        fclose(file);
        exit(1);
    }
    
    fread(buffer, 1, fileLen, file);
    fclose(file);
	buffer[fileLen] = '\0';

    if (bytesize == 2) {
        result = UTF8EncodeUTF16((utf16_t*)buffer, (fileLen / sizeof(utf16_t)) - 2, myorder);
    } else if (bytesize == 4) {
        result = UTF8EncodeUTF32((utf32_t*)buffer, (fileLen / sizeof(utf32_t)), myorder);
	} else {
	
		buffer[fileLen - offset];
	}

    if (result != NULL) {
        printf("'%s'\n", result);
        free(result);
    } else {

        printf("'%s'\n", buffer);
    }

    free(buffer);
}

int main()
{
    unicode_test_file("test/test_utf8_no_bom.txt", UTF_8);
    unicode_test_file("test/test_utf8.txt", UTF_8BOM);
    unicode_test_file("test/test_ucs2_LE.txt", UTF_16LE);
    unicode_test_file("test/test_ucs2_BE.txt", UTF_16BE);
    unicode_test_file("test/test_utf32_LE.txt", UTF_32LE);
    unicode_test_file("test/test_utf32_BE.txt", UTF_32BE);

#ifdef _WIN32
    _CrtDumpMemoryLeaks();
    system("pause");
#endif
    return 0;
}