/**
 *   Copyright (c) 2018 TectroLabs L.L.C.
 *
 *    Permission is hereby granted, free of charge, to any person obtaining
 *    a copy of this software and associated documentation files (the "Software"),
 *    to deal in the Software without restriction, including without limitation
 *    the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *    and/or sell copies of the Software, and to permit persons to whom the Software
 *    is furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 *    OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 *    @file BinHexConverter.cpp
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief converts bytes to HEX text and HEX text to bytes
 */

#include "BinHexConverter.h"

namespace entropyservice {

/**
 *
 * Convert bytes to HEX text
 *
 * @param inputBytes a pointer to byte array to be converted to HEX
 * @param inputBytesSize the size of the byte array
 * @param outputHex a pointer to resulting HEX text byte array of size inputBytesSize*2+1
 *
 * @return true for successful conversion
 *
 */
bool BinHexConverter::toHex(unsigned char *inputBytes, int inputBytesSize, char *outputHex) {
	if (inputBytes == NULL || inputBytesSize <= 0 || outputHex == NULL) {
		return false;
	}

	char *out = outputHex;
	for (int i = 0; i < inputBytesSize; i++) {
		sprintf(out, "%02X", inputBytes[i]);
		out += 2;
	}
	*out = '\0';
	return true;
}

/**
 *
 * Convert HEX text to bytes.
 *
 * @param inputHexChars a pointer to HEX text to convert to bytes
 * @param inputHexCharSize the size of the HEX text
 * @param outputBytes a pointer to resulting byte array of size inputHexCharSize/2
 *
 * @return true for successful conversion
 *
 */
bool BinHexConverter::toBin(char *inputHexChars, int inputHexCharSize, unsigned char *outputBytes) {
	if (inputHexChars == NULL || inputHexCharSize <= 0 || outputBytes == NULL) {
		return false;
	}
	unsigned char *out = outputBytes;
	for (int i = 0; i < inputHexCharSize; i += 2) {
		char high = tolower(inputHexChars[i]);
		char lower = tolower(inputHexChars[i+1]);
		unsigned char byte;
		if (!toInt(high, lower, &byte)) {
			return false;
		}
		*out++ = byte;
	}
	return true;
}

/**
 *
 * Convert a hex pair to byte
 *
 * @param high char
 * @param lower char
 * @param byte a pointer to resulting converted byte
 *
 * @return true for successful conversion
 *
 */
bool BinHexConverter::toInt(char high, char lower, unsigned char *byte) {
	unsigned char c1 = 0;
	unsigned char c2 = 0;
	if (high >= '0' && high <= '9') {
		c1 |= (high - 0x30);
	} else {
		if (high >= 'a' && high <= 'f') {
			c1 |= (high - 87);
		} else {
			return false;
		}
	}

	if (lower >= '0' && lower <= '9') {
		c2 |= (lower - 0x30);
	} else {
		if (lower >= 'a' && lower <= 'f') {
			c2 |= (lower - 87);
		} else {
			return false;
		}
	}
	*byte = (c1 << 4) | c2;
	return true;
}

BinHexConverter::BinHexConverter() {
}

BinHexConverter::~BinHexConverter() {
}

} /* namespace entropyservice */
