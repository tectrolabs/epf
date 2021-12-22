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
 *    @file HttpResponse.h
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief retrieves and processes HTTP responses from a remote REST service
 *
 */

#ifndef HTTPRESPONSE_H_
#define HTTPRESPONSE_H_

#include <map>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include "CryptoToken.h"
#include "XorCryptor.h"
#include "SHA256.h"

namespace entropyservice {


class HttpResponse {
public:
	HttpResponse(int fd, bool isSecure, SSL *ssl, bool isStreamEncrypted, CryptoToken *cryptoToken);
	std::string getHeader(std::string headerName);
	bool isResponseAvailable() const;
	bool readContent(char *byteBuff, int byteCount);
	std::string getLastErrorMessage() const;
	int retrieveResponseCode();
private:
	std::map <std::string, std::string> headers;
	std::string lastErrorMessage;
	bool isAvailable;
	bool isSecure;
	int fd;
	SSL *ssl;
	bool isStreamEncrypted;
	CryptoToken *cryptoToken;

private:
	void parseResponse();
	void parseLine(std::string line, char delimiter);
	std::vector<std::string> split(std::string str, std::string token);

};

} /* namespace entropyservice */

#endif /* HTTPRESPONSE_H_ */
