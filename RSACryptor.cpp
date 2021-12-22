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
 *    @file RSACryptor.cpp
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief utilizes RSA provider for encryption/decryption with private and public keys
 *
 */

#include "RSACryptor.h"

namespace entropyservice {

void RSACryptor::initialize() {
	rsa = NULL;
	bignum = NULL;
	padding = RSA_PKCS1_PADDING;
	initializedKey = false;
}

/**
 *
 * Construct using key file
 *
 * @param keyFileName a pointer to a file with either private or public
 * @param isPublic true if key is public, false if the key is private
 *
 */
RSACryptor::RSACryptor(const char *keyFileName, bool isPublic) {
	initialize();

	if (keyFileName == NULL) {
		return;
	}

	FILE *fp = NULL;

	if (isPublic) {
		fp = fopen(keyFileName, "r");
		if (fp != NULL) {
			if (PEM_read_RSAPublicKey(fp, &rsa, NULL, NULL) == NULL) {
				fclose(fp);
				return;
			} else {
				fclose(fp);
				initializedKey = true;
				return;
			}
		}
	}

	fp = fopen(keyFileName, "r");
	if (fp != NULL) {
		if (PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL) == NULL) {
			fclose(fp);
			return;
		}
		fclose(fp);
		initializedKey = true;
		return;
	}
}

/**
 * Encrypt bytes with RSA using public key
 *
 * @param toEncrypt bytes to encrypt
 * @param toEncryptSizeBytes number of bytes to encrypt
 * @param encrypted encrypted bytes
 * @param encryptedSizeBytes a pointer to store the number of encrypted bytes
 *
 * @return true if encryption was successful
 */
bool RSACryptor::encryptWithPublicKey(unsigned char *toEncrypt, int toEncryptSizeBytes,	unsigned char *encrypted, int *encryptedSizeBytes) {

	if (!initializedKey || toEncrypt == NULL || encrypted == NULL) {
		return false;
	}

    int result = RSA_public_encrypt(toEncryptSizeBytes, (unsigned char*)toEncrypt, encrypted, rsa, padding);
	if (result == -1) {
		return false;
	}
	*encryptedSizeBytes = result;
	return true;
}

/**
 * Encrypt bytes with RSA using private key
 *
 * @param toEncrypt bytes to encrypt
 * @param toEncryptSizeBytes number of bytes to encrypt
 * @param encrypted encrypted bytes
 * @param encryptedSizeBytes a pointer to store the number of encrypted bytes
 *
 * @return true if encryption was successful
 */
bool RSACryptor::encryptWithPrivateKey(unsigned char *toEncrypt, int toEncryptSizeBytes, unsigned char *encrypted, int *encryptedSizeBytes) {

	if (!initializedKey || toEncrypt == NULL || encrypted == NULL) {
		return false;
	}

    int result = RSA_private_encrypt(toEncryptSizeBytes, toEncrypt, encrypted, rsa, padding);
	if (result == -1) {
		return false;
	}
	*encryptedSizeBytes = result;
	return true;
}

/**
 * Decrypt bytes with RSA using private key
 *
 * @param toDecrypt bytes to decrypt
 * @param toDecryptSizeBytes number of bytes to decrypt
 * @param decrypted decrypted bytes
 * @param decryptedSizeBytes a pointer to store the number of decrypted bytes
 *
 * @return true if decryption was successful
 */
bool RSACryptor::decryptWithPrivateKey(unsigned char *toDecrypt, int toDecryptSizeBytes, unsigned char *decrypted, int *decryptedSizeBytes) {
	if (!initializedKey || toDecrypt == NULL || decrypted == NULL) {
		return false;
	}

    int result = RSA_private_decrypt(toDecryptSizeBytes, toDecrypt, decrypted, rsa, padding);
	if (result == -1) {
		return false;
	}
	*decryptedSizeBytes = result;
	return true;
}

/**
 * Decrypt bytes with RSA using public key
 *
 * @param toDecrypt bytes to decrypt
 * @param toDecryptSizeBytes number of bytes to decrypt
 * @param decrypted decrypted bytes
 * @param decryptedSizeBytes a pointer to store the number of decrypted bytes
 *
 * @return true if decryption was successful
 */
bool RSACryptor::decryptWithPublicKey(unsigned char *toDecrypt, int toDecryptSizeBytes, unsigned char *decrypted, int *decryptedSizeBytes) {
	if (!initializedKey || toDecrypt == NULL || decrypted == NULL) {
		return false;
	}

    int result = RSA_public_decrypt(toDecryptSizeBytes, toDecrypt, decrypted, rsa, padding);
	if (result == -1) {
		return false;
	}
	*decryptedSizeBytes = result;
	return true;
}

/**
 *
 * Construct using a new key
 *
 * @param keySize RSA key size
 *
 */
RSACryptor::RSACryptor(int keySize) {
	initialize();
	creteNewKey(keySize);
}

/**
 *
 * Construct using a new RSA key
 *
 */
RSACryptor::RSACryptor() {
	initialize();
	creteNewKey(2048);
}

/**
 * Export private key to a file
 *
 * @param fileName location of the file to store the private key
 *
 * @return true if export was successful
 */
bool RSACryptor::exportPrivateKeyToFile(const char *fileName) {
	if (!initializedKey || fileName == NULL) {
		return false;
	}

	BIO *bp = BIO_new_file(fileName, "w+");
	if (PEM_write_bio_RSAPrivateKey(bp, rsa, NULL, NULL, 0, NULL, NULL) != 1) {
		BIO_free_all(bp);
		return false;
	}
	BIO_free_all(bp);
	return true;
}

/**
 * Export public key to a file
 *
 * @param fileName location of the file to store the public key
 *
 * @return true if export was successful
 */
bool RSACryptor::exportPublicKeyToFile(const char *fileName) {
	if (!initializedKey || fileName == NULL) {
		return false;
	}

	BIO *bp = BIO_new_file(fileName, "w+");
	if (PEM_write_bio_RSAPublicKey(bp, rsa) != 1) {
		BIO_free_all(bp);
		return false;
	}
	BIO_free_all(bp);
	return true;
}

/**
 *
 * Create a new key pair
 *
 * @param keySize RSA key size
 *
 */
void RSACryptor::creteNewKey(int keySize) {
	initializedKey = false;

	unsigned long e = RSA_F4;

	bignum = BN_new();

	if (BN_set_word(bignum, e) != 1) {
		return;
	}

	rsa = RSA_new();
	if (RSA_generate_key_ex(rsa, keySize, bignum, NULL) != 1) {
		return;
	}

	initializedKey = true;
}

/**
 *
 * Check to see if the instance is initialized
 *
 * @return true if initialized
 *
 */
bool RSACryptor::isInitialized() const {
	return initializedKey;
}

/**
 * De-allocate resources.
 */
RSACryptor::~RSACryptor() {
	if (rsa) {
		RSA_free(rsa);
	}
	if (bignum) {
		BN_free(bignum);
	}
}

} /* namespace entropyservice */
