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
 *    @file CryptoToken.h
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief generates a crypter key to be exchanged with a remote service
 */

#ifndef CRYPTOTOKEN_H_
#define CRYPTOTOKEN_H_

#include <stdlib.h>
#include <string>
#include "RSACryptor.h"
#include "BinHexConverter.h"


namespace entropyservice {

class CryptoToken {
public:
	bool createTokenAsText(std::string &tokenText);
	bool loadTokenFomText(std::string &tokenText);
	explicit CryptoToken(RSACryptor *rsaCryptor);
	unsigned char* getCripter();
	int getCripterSize();
	virtual ~CryptoToken();
private:
	void initialize();
	void initializeRandomBytes(unsigned char *byteArray, int byteArraySize);
private:
	RSACryptor *rsaCryptor;
	unsigned char key[48];
};

} /* namespace entropyservice */

#endif /* CRYPTOTOKEN_H_ */
