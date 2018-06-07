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
 *    @file CryptoToken.cpp
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief generates a crypter key to be exchanged with a remote service
 */

#include "CryptoToken.h"

namespace entropyservice {

/**
 * Initialize byte array with random values
 *
 * @param byteArray bytes to initialize with random values
 * @param byteArraySize number of bytes to initialize
 */
void CryptoToken::initializeRandomBytes(unsigned char *byteArray, int byteArraySize) {
	for (int i = 0; i < byteArraySize; i++) {
		byteArray[i] = rand() % 256;
	}
}

/**
 * Generate token as ASCIIZ
 *
 * @param tokenText reference to the new token
 * @return true when token created successfully
 */
bool CryptoToken::createTokenAsText(std::string &tokenText) {
	// Encrypt the crypter with RSA public key
	unsigned char crypterEncrypted[300];
	int crypterEncryptedByteSize;
	if (!rsaCryptor->encryptWithPublicKey(key, sizeof(key), crypterEncrypted, &crypterEncryptedByteSize)) {
		return false;
	}

	char crypterEncryptedHex[600];
	BinHexConverter bhConv;
	if (!bhConv.toHex(crypterEncrypted, crypterEncryptedByteSize, crypterEncryptedHex)) {
		return false;
	}

	tokenText.append(crypterEncryptedHex);
	return true;
}

/**
 * Load ASCIIZ token
 *
 * @param tokenText reference to token to load
 * @return true when token loaded successfully
 */
bool CryptoToken::loadTokenFomText(std::string &tokenText) {
	if (tokenText.size() > 600 || tokenText.size() < 10) {
		return false;
	}
	BinHexConverter bhConv;
	unsigned char crypterEncrypted[300];
	int crypterEncryptedByteSize = tokenText.size() / 2;
	if (!bhConv.toBin((char*)tokenText.c_str(), (int)tokenText.size(), crypterEncrypted)) {
		return false;
	}

	// Decrypt key
	int decryptedSizeBytes;
	if (!rsaCryptor->decryptWithPrivateKey(crypterEncrypted, crypterEncryptedByteSize, key, &decryptedSizeBytes)) {
		return false;
	}
	return true;
}

/**
 * Get pointer to cripter structure
 *
 * @return pointer to cripter structure
 */
unsigned char *CryptoToken::getCripter() {
	return key;
}

/**
 * Initialize variables
 */
void CryptoToken::initialize() {
	initializeRandomBytes(key, sizeof(key));
}

/*
 * @param rsaCryptor
 */
CryptoToken::CryptoToken(RSACryptor *rsaCryptor) {
	initialize();
	this->rsaCryptor = rsaCryptor;
}

int CryptoToken::getCripterSize() {
	return sizeof(key);
}

CryptoToken::~CryptoToken() {
}

} /* namespace entropyservice */
