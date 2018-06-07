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
 *    @file SHA256.cpp
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief hashes an array of bytes using SHA256
 *
 */

#include "SHA256.h"

namespace entropyservice {

/**
 * @param bytesToHash a pointer to byte array to hash
 * @param byteCount how many bytes to hash
 * @return true if hashing completed successfully
 */
bool SHA256::hash(unsigned char *bytesToHash, int byteCount) {

	SHA256_CTX context;

	if (SHA256_Init(&context) != 1) {
		return false;
	}

	// Add salt
	if (SHA256_Update(&context, (unsigned char*)"2093457209837", 13) != 1) {
		return false;
	}

	// Hash the byte array
	if (SHA256_Update(&context, (unsigned char*)bytesToHash, byteCount) != 1) {
		return false;
	}

	// Finalize hashing
	if (SHA256_Final(md, &context) != 1) {
		return false;
	}

	return true;
}

/**
 * @return message digest
 */
unsigned char * SHA256::getMessageDigest() {
	return md;
}

/**
 * @return message digest size in bytes
 */
int SHA256::getMessageDigestSize() {
	return SHA256_DIGEST_LENGTH;
}

SHA256::SHA256() {
	// TODO Auto-generated constructor stub

}

SHA256::~SHA256() {
	// TODO Auto-generated destructor stub
}

} /* namespace entropyservice */
