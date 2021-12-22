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
 *    @file HttpClient.h
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief sends HTTP requests to a remote REST service
 *
 */

#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <resolv.h>
#include <netdb.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>


#include "HttpResponse.h"
#include "RSACryptor.h"
#include "CryptoToken.h"

namespace entropyservice {

class HttpClient {
public:
	HttpClient(std::string hostName, int port, bool isSecure, std::string tlAuthToken,
			bool isStreamEncrypted, RSACryptor *pubKeyCryptor);
	virtual ~HttpClient();
	bool connectToHost();
	std::string getLastErrorMessage() const;
	void closeConnection();
	bool isConntected() const { return isSocketCreated; }
	bool sendGetRequest(std::string resource, CryptoToken *cryptoToken);
	HttpResponse retrieveResponse(CryptoToken *cryptoToken);
private:
	std::string hostName;
	std::string tlAuthToken;
	in_port_t port;
	bool isSecure;
	std::string lastErrorMessage;
	int fd;
	bool isSocketCreated;
	SSL_CTX *ctx;
	SSL *ssl;
	const SSL_METHOD *method;
	bool isStreamEncrypted;
	RSACryptor *pubKeyCryptor;
private:
	void createSocket();
};

} /* namespace entropyservice */

#endif /* HTTPCLIENT_H_ */
