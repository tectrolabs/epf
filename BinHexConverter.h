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
 *    @file BinHexConverter.h
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief converts bytes to HEX text and HEX text to bytes
 */

#ifndef BINHEXCONVERTER_H_
#define BINHEXCONVERTER_H_

#include <stdio.h>
#include <ctype.h>

namespace entropyservice {

class BinHexConverter {
public:
	BinHexConverter();
	bool toHex(unsigned char *inputBytes, int inputBytesSize, char *outputHex);
	bool toBin(char *inputHexChars, int inputHexCharSize, unsigned char *outputBytes);
	virtual ~BinHexConverter();
private:
	bool toInt(char high, char lower, unsigned char *byte);
};

} /* namespace entropyservice */

#endif /* BINHEXCONVERTER_H_ */
