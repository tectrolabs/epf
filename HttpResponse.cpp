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
 *    @file HttpResponse.cpp
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief retrieves and processes HTTP responses from a remote REST service
 *
 */

#include "HttpResponse.h"

namespace entropyservice {

/**
 * Constructor
 *
 * @param fd file descriptor (socket)
 * @param isSecure true when using SSL
 * @param ssl pointer to SSL structure
 * @param isStreamEncrypted
 * @param cryptoToken
 *
 */
HttpResponse::HttpResponse(int fd, bool isSecure, SSL *ssl, bool isStreamEncrypted, CryptoToken *cryptoToken) {
	this->fd = fd;
	this->isAvailable = false;
	this->isSecure = isSecure;
	this->ssl = ssl;
	this->isStreamEncrypted = isStreamEncrypted;
	this->cryptoToken = cryptoToken;
	parseResponse();
}

/**
 * Retrieve header string value given a header name
 *
 * @param headerName
 * @return header string value
 */
std::string HttpResponse::getHeader(std::string headerName) {
	return headers[headerName];
}

/**
 * Check to see if response is available
 *
 * @return true if response is available
 */
bool HttpResponse::isResponseAvailable() {
	return this->isAvailable;
}

/**
 * Read requested amount of bytes from the response body into specified buffer
 *
 * @param byteCount how many bytes to read
 * @param buteBuff pointer to destination bytes buffer
 *
 * @return true for successful operation
 */
bool HttpResponse::readContent(char *byteBuff, int byteCount) {
	if (!isResponseAvailable()) {
		return false;
	}

	int totalBytesRead = 0;

	while(totalBytesRead < byteCount) {
		int bytesRead;
		if (isSecure) {
			bytesRead = SSL_read(ssl, byteBuff + totalBytesRead, byteCount - totalBytesRead);
		} else {
			bytesRead = read(fd, byteBuff + totalBytesRead, byteCount - totalBytesRead);
		}
		if (bytesRead < 0) {
			lastErrorMessage = "Error when reading HTTP response body";
			return false;
		}
		if (bytesRead == 0) {
			if (totalBytesRead != byteCount) {
				lastErrorMessage = "Incomplete HTTP response body";
				return false;
			}
			break;
		}
		totalBytesRead =  bytesRead + totalBytesRead;
		if (isStreamEncrypted) {
			XorCryptor cryptor = XorCryptor();
			cryptor.crypt((unsigned char*)byteBuff, byteCount, cryptoToken->getCripter(), cryptoToken->getCripterSize());
			// Verify byte stream finger print
			std::string expectedByteStreamHash = headers["TL-RESP-BYTEHASH"];
			if (expectedByteStreamHash.size() == 0) {
				lastErrorMessage = "Missing byte stream hash value";
				return false;
			}

			SHA256 sha;
			if (!sha.hash((unsigned char*)byteBuff, byteCount)) {
				lastErrorMessage = "Could not calculate hash value";
				return false;
			}
			BinHexConverter hexConverter;
			char hashTxt[1024];
			if (!hexConverter.toHex(sha.getMessageDigest(), sha.getMessageDigestSize(), hashTxt)) {
				lastErrorMessage = "Could not convert bytes to hash";
				return false;
			}

			std::string actualByteStreamHash = std::string(hashTxt);
			if (actualByteStreamHash.compare(expectedByteStreamHash) != 0) {
				lastErrorMessage = "Byte stream hash values don't match";
				return false;
			}

		}
	}

	return true;
}

/**
 * Parse the response, retrieve HTTP headers and response code
 */
void HttpResponse::parseResponse() {
	char c;
	char line[256];
	int i = 0;
	int newLineCount = 0;
	bool firstLine = true;

	// Read response headers
	while(true){
		int bytesRead;
		if (isSecure) {
			bytesRead = SSL_read(ssl, &c, 1);
		} else {
			bytesRead = read(fd, &c, 1);
		}
		if (bytesRead == -1 || i >= (int)sizeof(line)) {
			lastErrorMessage = "Error when reading HTTP response headers";
			return;
		}
		if (bytesRead == 0) {
			return;
		}
		line[i++] = c;
		if (c == '\n') {
			newLineCount++;
			if (newLineCount > 0) {
				// found the end of new line
				line[i] = '\0';
				if (firstLine) {
					firstLine = false;
					parseLine(line, '/');
				} else {
					parseLine(line, ':');
				}
			}
			if (newLineCount > 1) {
				// found end of headers
				break;
			}
			i = 0;
		} else {
			if (c != '\r') {
				// reset the new line counter
				newLineCount = 0;
			}
		}
	}
	isAvailable = true;
}

/**
 * Parse a line form the HTTP response and extract the header name and value
 */
void HttpResponse:: parseLine(std::string line, char delimiter) {

	std::string key, value;

	std::string::size_type begin = line.find_first_not_of(" \f\t\v");

	if (begin == std::string::npos)
		return;

	std::string::size_type end = line.find(delimiter, begin);
	key = line.substr(begin, end - begin);

	key.erase(key.find_last_not_of(" \f\t\v\n\r") + 1);

	if (key.empty()) {
		return;
	}

	begin = line.find_first_not_of(" \f\n\r\t\v", end + 1);
	end = line.find_last_not_of(" \f\n\r\t\v") + 1;

	value = line.substr(begin, end - begin);

	headers[key] = value;

}

/**
 * Retrieve the HTTP response code
 *
 * @return response code as integer
 */
int HttpResponse::retrieveResponseCode() {
	if (isResponseAvailable()) {
		std::string httpHeader = getHeader("HTTP");
		if (httpHeader.size() > 0) {
			std::vector<std::string> tokens = split(httpHeader, " ");
			if (tokens.size() >= 2) {
				std::string httpCode = tokens[1];
				return atoi(httpCode.c_str());
			}
		}
	}
	return 0;
}

/**
 * Get last known error message
 *
 * @return last error message
 */
std::string HttpResponse::getLastErrorMessage() {
	return lastErrorMessage;
}

/**
 * Split a string into a vector of tokens
 *
 * @param str string to split
 * @param token delimiter
 *
 * @return a vector of tokens
 */
std::vector<std::string> HttpResponse::split(std::string str, std::string token) {
	std::vector<std::string>result;
    while(str.size()){
        int index = str.find(token);
        if(index != (int)std::string::npos){
            result.push_back(str.substr(0,index));
            str = str.substr(index+token.size());
            if(str.size()==0)result.push_back(str);
        }else{
            result.push_back(str);
            str = "";
        }
    }
    return result;
}


} /* namespace entropyservice */
