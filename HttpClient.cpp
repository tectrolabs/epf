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
 *    @file HttpClient.cpp
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief sends HTTP requests to a remote REST service
 *
 */

#include "HttpClient.h"

namespace entropyservice {

/**
 *
 * Constructor
 *
 * @param hostName of the remote entropy service
 * @param port remote entropy service listening port
 * @param isSecure true when using SSL
 * @param tlAuthToken authorization token if available
 * @param isStreamEncrypted true if needs to encrypt the byte stream
 * @param pubKeyCryptor public byte stream cryptor
 */
HttpClient::HttpClient(std::string hostName, int port, bool isSecure, std::string tlAuthToken,
		bool isStreamEncrypted, RSACryptor *pubKeyCryptor) {
	this->hostName = hostName;
	this->port = port;
	this->isSecure = isSecure;
	this->tlAuthToken = tlAuthToken;
	this->isStreamEncrypted = isStreamEncrypted;
	this->pubKeyCryptor = pubKeyCryptor;
	fd = -1;
	isSocketCreated = false;
	ctx = NULL;
	ssl = NULL;
	method = NULL;
}

/**
 * Destructor
 *
 * To make sure the connection is closed
 */
HttpClient::~HttpClient() {
	closeConnection();
}

/**
 * Retrieve last known error message
 *
 * @return last error message
 */
std::string HttpClient::getLastErrorMessage() {
	return lastErrorMessage;
}

/**
 * Connect to remote host
 *
 * @return true if connection established successfully
 */
bool HttpClient::connectToHost() {

	closeConnection();

	if (hostName.size() == 0) {
		lastErrorMessage = "Host name cannot be empty";
		return false;
	}

	if (port == 0) {
		lastErrorMessage = "Port cannot be zero";
		return false;
	}

	if (isSecure) {
		if(SSL_library_init() < 0) {
			lastErrorMessage = "Could not initialize OpenSSL library";
			return false;
		}

		OpenSSL_add_all_algorithms();
	    SSL_load_error_strings();
		method = SSLv23_client_method();

		if ( (ctx = SSL_CTX_new(method)) == NULL) {
			lastErrorMessage = "Could not create a new SSL context";
			return false;
		}

		SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
		ssl = SSL_new(ctx);
	}

	createSocket();
	if (!isSocketCreated) {
		return false;
	}

	if (isSecure) {
		SSL_set_fd(ssl, fd);
		if ( SSL_connect(ssl) != 1 ) {
			lastErrorMessage = "Could not build a SSL session to remote host";
			return false;
		}
	}

	return true;
}

/**
 * Create a socket to remote entropy service
 */
void HttpClient::createSocket() {
	struct hostent *he;
	struct sockaddr_in addr;
	int sock;
	int on = 1;

	if ((he = gethostbyname(hostName.c_str())) == NULL) {
		this->isSocketCreated = false;
		lastErrorMessage = "Could not find the host";
		return;
	}

	bcopy(he->h_addr, &addr.sin_addr, he->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		this->isSocketCreated = false;
		lastErrorMessage = "Could not create a socket";
		return;
	}
	int sockoptStatus = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *) &on,	sizeof(int));

	if (sockoptStatus < 0) {
		this->isSocketCreated = false;
		lastErrorMessage = "setsockopt(...) call failed";
		return;
	}

	 struct timeval tv;
	 tv.tv_sec = 15;
	 tv.tv_usec = 0;
	 if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof tv)) {
		this->isSocketCreated = false;
		lastErrorMessage = "setsockopt(...) call for SO_RCVTIMEO failed";
		return;
	 }

	 tv.tv_sec = 5;
	 if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,  sizeof tv)) {
		this->isSocketCreated = false;
		lastErrorMessage = "setsockopt(...) call for SO_SNDTIMEO failed";
		return;
	 }

    if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		this->isSocketCreated = false;
		lastErrorMessage = "Could not connect to remote host";
		return;
	}
	this->isSocketCreated = true;
	lastErrorMessage = "";
	this->fd = sock;

}

/**
 * Close socket connection and associated resources
 */
void HttpClient::closeConnection() {

	if (ssl != NULL) {
		SSL_free(ssl);
		ssl = NULL;
	}

	if (isSocketCreated) {
		shutdown(fd, SHUT_RDWR);
		close(fd);
		isSocketCreated = false;
	}

	if (ctx != NULL) {
		SSL_CTX_free(ctx);
		ctx = NULL;
	}

	lastErrorMessage = "";

}

/**
 * Send HTTP GET 1.0 request to the remote entropy service
 *
 * @param resource HTTP resource
 * @param cryptoToken a pointer to CryptoToken to generate crypto token
 * @return true is request sent successfully
 */
bool HttpClient::sendGetRequest(std::string resource, CryptoToken *cryptoToken) {
	if (!isConntected()) {
		return false;
	}
	std::string cmd;
	cmd.append("GET ").append(resource).append(" HTTP/1.0\r\n");
	cmd.append("Host: ").append(hostName).append("\r\n");
	if (tlAuthToken.size() > 0) {
		cmd.append("tl-ent-sce-auth-token: ").append(tlAuthToken).append("\r\n");
	}
	if (isStreamEncrypted) {
		std::string cryptoTokenHex;
		if (!cryptoToken->createTokenAsText(cryptoTokenHex)) {
			lastErrorMessage = "Could not create crypto token";
			return false;
		}
		cmd.append("tl-ent-sce-crypto-token: ").append(cryptoTokenHex).append("\r\n");
	}
	cmd.append("\r\n");

	int bytesSent;
	if (isSecure) {
		bytesSent = SSL_write(ssl, cmd.c_str(), cmd.size());
	} else {
		bytesSent = write(fd, cmd.c_str(), cmd.size());
	}
	if (bytesSent == -1 || bytesSent != (int)cmd.size()) {
		lastErrorMessage = "Could not send HTTP GET request";
		return false;
	}
	return true;
}

/**
 * Retrieve HTTP response from the remote entropy service
 * @param cryptoToken
 *
 * @return HttpResponse instance
 */
HttpResponse HttpClient::retrieveResponse(CryptoToken *cryptoToken) {
	return HttpResponse(fd, isSecure, ssl, isStreamEncrypted, cryptoToken);
}

} /* namespace entropyservice */
