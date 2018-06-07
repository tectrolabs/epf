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
 *    @file RSACryptor.h
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief utilizes RSA provider for encryption/decryption with private and public keys
 *
 */
#ifndef RSACRYPTOR_H_
#define RSACRYPTOR_H_

#include <openssl/rsa.h>
#include <openssl/pem.h>

namespace entropyservice {

class RSACryptor {
public:
	RSACryptor();
	RSACryptor(int keySize);
	RSACryptor(const char *keyFileName, bool isPublic);
	bool isInitialized();
	bool exportPrivateKeyToFile(const char *fileName);
	bool exportPublicKeyToFile(const char *fileName);
	bool encryptWithPublicKey(unsigned char *toEncrypt, int toEncryptSizeBytes,	unsigned char *encrypted, int *encryptedSizeBytes);
	bool decryptWithPrivateKey(unsigned char *toDecrypt, int toDecryptSizeBytes,	unsigned char *decrypted, int *decryptedSizeBytes);
	bool encryptWithPrivateKey(unsigned char *toEncrypt, int toEncryptSizeBytes,	unsigned char *encrypted, int *encryptedSizeBytes);
	bool decryptWithPublicKey(unsigned char *toDecrypt, int toDecryptSizeBytes,	unsigned char *decrypted, int *decryptedSizeBytes);
	virtual ~RSACryptor();
private:
	void initialize();
	void creteNewKey(int keySize);
private:
    RSA *rsa;
    BIGNUM *bignum;
    bool initializedKey;
    int padding;
};

} /* namespace entropyservice */

#endif /* RSACRYPTOR_H_ */
