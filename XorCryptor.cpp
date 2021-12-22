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
 *    @file XorCryptor.cpp
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief encrypts an array of bytes using XOR with a key
 *
 */

#include "XorCryptor.h"

namespace entropyservice {

/**
 * Crypt requested bytes
 * @param bytes painter to byte array to crypt
 * @param byteCount how many bytes to crypt
 * @param cryptoKey pinter to the crypto key
 * @keySize cryptoKeySize key size
 *
 */
bool XorCryptor::crypt(unsigned char *bytes, int byteCount, const unsigned char *cryptoKey, int cryptoKeySize) {
	if (bytes == NULL || byteCount <= 1 || cryptoKey == NULL || cryptoKeySize < 1) {
		return false;
	}
	int keyIndex = 0;
	for(int i = 0; i < byteCount; i++) {
		bytes[i] ^= cryptoKey[keyIndex++];
		if (keyIndex >= cryptoKeySize) {
			keyIndex = 0;
		}
	}
	return true;
}

XorCryptor::XorCryptor() {
}

XorCryptor::~XorCryptor() {
}

} /* namespace entropyservice */
