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
 *    @file epf.cpp
 *    @author Andrian Belinski
 *    @date 06/1/2018
 *    @version 1.0
 *
 *    @brief feeds the Linux entropy pool with true random bytes retrieved from a remote service
 *
 *    @section DESCRIPTION
 *
 *    This is a small program that feeds the entropy pool in Linux OS with true random bytes retrieved
 *    from a remote HTTP service. It requires 'sudo' access permissions to run.
 *
 *    A command line argument is used to specify where the file with configuration properties is located.
 */

#include <iostream>
#include <stack>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/random.h>

#include "Configuration.h"
#include "HttpClient.h"
#include "HttpResponse.h"
#include "RSACryptor.h"
#include "XorCryptor.h"

using namespace entropyservice;

// Define property name for retrieving the entropy service host name from configuration file
#define ENTROPY_HOST_PROPERTY_NAME "entropy.host"

// Define property name for retrieving the entropy service port from configuration file
#define ENTROPY_PORT_PROPERTY_NAME "entropy.port"

// Define property name for retrieving the host entropy service resource name from configuration file
#define ENTROPY_RESOURCE_PROPERTY_NAME "entropy.resource"

// Define property name for retrieving the byte stream encryption flag (true/false) from configuration file
#define ENTROPY_RESOURCE_BYTESTREAM_ENCRYPT_PROPERTY_NAME "entropy.resource.bytestream.encrypt"

// Define property name for retrieving the path to the RSA public key from configuration file
#define ENTROPY_RESOURCE_BYTESTREAM_RSA_FILE_PROPERTY_NAME "entropy.resource.bytestream.encrypt.pubkey.rsa.file"

// Define property name for retrieving the entropy service request size from configuration file
#define ENTROPY_REQUEST_SIZE_PROPERTY_NAME "entropy.request.byte.count"

// Define property name for retrieving the SSL (true/false) flag from configuration file
#define ENTROPY_HOST_SSL_ENABLED_PROPERTY_NAME "entropy.host.ssl.enabled"

// Define property name for retrieving the entropy service authentication token from configuration file
#define ENTROPY_AUTH_TOKEN_PROPERTY_NAME "entropy.auth.token"

// Define property name for retrieving the download thread heart beat period (in microseconds) from configuration file
#define ENTROPY_DWNLD_THREAD_PERIOD_USECS_PROPERTY_NAME "entropy.download.thread.period.usecs"

// Define property name for retrieving the feeder thread heart beat period (in microseconds) from configuration file
#define ENTROPY_FEEDER_THREAD_PERIOD_USECS_PROPERTY_NAME "entropy.feeder.thread.period.usecs"

// Define property name for retrieving the maximum number of bytes in the double ended queues from configuration file
#define ENTROPY_MAX_DEQ_SIZE_BYTES_PROPERTY_NAME "entropy.feeder.max.deq.size.bytes"

// Define number of threads for feeding the entropy pool
#define NUM_THREADS 1

// Define maximum number of bytes per request when connecting to entropy service
#define MAX_REQUEST_BYTES 10000

// Location of the Linux entropy pool
#define KERNEL_ENTROPY_POOL_NAME "/dev/random"

// Location of the kernel entropy pool size
#define KERNEL_POOLSIZE_LOCATOIN "/proc/sys/kernel/random/poolsize"

// Maximum accepted size of the kernel entropy pool in bytes
#define MAX_POOL_SIZE_BYTES (1024 * 64)	// 64 KB

// Maximum number of bytes in the double ended queue below
int maxDeqSizeBytes;

// Double ended queue as a dynamic storage for downloaded random bytes
std::deque<uint8_t> deq1;

// Double ended queue as a dynamic storage for random bytes for feeding the entropy pool
std::deque<uint8_t> deq2;

// Mutex for thread synchronization
pthread_mutex_t tMutex = PTHREAD_MUTEX_INITIALIZER;

// Reference to entropy client configuration
Configuration config;

// An array of threads, as a dynamic storage, for feeding the entropy pool
pthread_t entropyThreadArray[NUM_THREADS];

// A reference to the downloading thread
pthread_t downloadThread;

// Used to signal all threads that an error has been detected
volatile bool isError = false;

// Entropy pool size in bytes
int entropyPoolSizeBytes;

// Name of the property file for retrieving the configuration data
std::string propFileName;

// A pointer to the encryptor
RSACryptor *pubKeyCryptor;

// A flag to indicate if the byte stream should must be encrypted
bool isStreamEncrypted = false;

/**
 * A thread for populating dynamic storage with random bytes
 * downloaded from Entropy Service 
 *
 * @param arg - pointer to arguments passed to the thread
 * @return void*
 */
void *downloadBytes(void *arg) {
	char *threadName = (char*) arg;
	char rndBytes[MAX_REQUEST_BYTES];

	int heartBeatUsecs = config.getProperty(ENTROPY_DWNLD_THREAD_PERIOD_USECS_PROPERTY_NAME).getIntValue();
	std::string hostName = config.getProperty(ENTROPY_HOST_PROPERTY_NAME).getStringValue();
	int port = config.getProperty(ENTROPY_PORT_PROPERTY_NAME).getIntValue();
	std::string resource = config.getProperty(ENTROPY_RESOURCE_PROPERTY_NAME).getStringValue();
	std::string authToken = config.getProperty(ENTROPY_AUTH_TOKEN_PROPERTY_NAME).getStringValue();
	std::string requestSizeString = config.getProperty(ENTROPY_REQUEST_SIZE_PROPERTY_NAME).getStringValue();
	bool isSSL = config.getProperty(ENTROPY_HOST_SSL_ENABLED_PROPERTY_NAME).getBoolValue();
	int requestSize = config.getProperty(ENTROPY_REQUEST_SIZE_PROPERTY_NAME).getIntValue();
	if (requestSize > MAX_REQUEST_BYTES) {
		requestSize = MAX_REQUEST_BYTES;
	}
	resource.append(requestSizeString);

	while (!isError) {
		// Check to see if we need to download more bytes
		if ((int)deq1.size() < maxDeqSizeBytes / 2) {
			HttpClient httpCli(hostName, port, isSSL, authToken, isStreamEncrypted, pubKeyCryptor);
			bool isConnectionError = false;
			if (!httpCli.connectToHost()) {
				std::cerr << "Connection to host failed" << std::endl;
				isConnectionError = true;
			} else {
				CryptoToken cryptoToken = CryptoToken(pubKeyCryptor);
				if (!httpCli.sendGetRequest(resource, &cryptoToken)) {
					std::cerr << "Could not send request to host" << std::endl;
					isConnectionError = true;
				} else {
					HttpResponse resp = httpCli.retrieveResponse(&cryptoToken);
					if (!resp.isResponseAvailable()) {
						std::cerr << "Could not retrieve HTTP response from host" << std::endl;
						isConnectionError = true;
					} else {
						int httpCode = resp.retrieveResponseCode();
						if (httpCode != 200) {
							std::cerr << "Unexpected HTTP response code: " << httpCode << std::endl;
							isConnectionError = true;
						} else {
							if (!resp.readContent(rndBytes, requestSize)) {
								std::cerr << "Could not retrieve requested bytes" << std::endl;
								isConnectionError = true;
							} else {
								for (int i = 0; i < requestSize; i++) {
									deq1.push_back(rndBytes[i]);
								}
							}
						}
					}
				}
			}
			httpCli.closeConnection();
			if (isConnectionError) {
				usleep(1000 * 1000 * 15 );
			}
		}
		int rc = pthread_mutex_lock(&tMutex);
		if (rc) {
			std::cerr << "could not lock the mutex in thread: " << threadName << std::endl;
			isError = true;
			pthread_exit(NULL);
		}
		if ((int)deq2.size() < maxDeqSizeBytes / 2) {
			// deq2 is below 'water mark', add more random bytes from deq1
			for (int i = 0; i < (int) deq1.size(); i++) {
				deq2.push_back(deq1.front());
				deq1.pop_front();
			}
		}
		rc = pthread_mutex_unlock(&tMutex);
		if (rc) {
			std::cerr << "could not unlock the mutex in thread: " << threadName
					<< std::endl;
			isError = true;
			pthread_exit(NULL);
		}
		usleep(heartBeatUsecs);
	}
	pthread_exit(NULL);
}

/**
 * A thread for feeding the Linux entropy pool with random data downloaded
 * using Entropy Sector API
 *
 * @param arg - arguments passed to the thread
 * @return void*
 */
void *feedEntropyPool(void *arg) {
	int entropyAvailable; // A variable for checking the amount of the entropy available in the kernel pool
	struct {
		int entropy_count;
		int buf_size;
		unsigned char data[MAX_POOL_SIZE_BYTES];
	} entropy;

	char *threadName = (char*) arg;
	int heartBeatUsecs = config.getProperty(ENTROPY_FEEDER_THREAD_PERIOD_USECS_PROPERTY_NAME).getIntValue();
	int rndout = open(KERNEL_ENTROPY_POOL_NAME, O_WRONLY);
	if (rndout < 0) {
		std::cerr << "Cannot open " << KERNEL_ENTROPY_POOL_NAME << ":"
				<< rndout << std::endl;
		isError = true;
		pthread_exit(NULL);
	}

	// Check to see if we have privileges for feeding the entropy pool
	int result = ioctl(rndout, RNDGETENTCNT, &entropyAvailable);
	if (result < 0) {
		std::cerr
				<< "Cannot verify available entropy in the pool, make sure you run this utility with CAP_SYS_ADMIN capability"
				<< std::endl;
		close(rndout);
		isError = true;
		pthread_exit(NULL);
	}
	std::cout << "Feeding the " << KERNEL_ENTROPY_POOL_NAME
		<< " kernel entropy pool of size " << (entropyPoolSizeBytes * 8) << " bits. Initial amount of entropy bits in the pool: "
		<< entropyAvailable << " ..." << std::endl;
	while (!isError) {
		// Lock the mutex
		int rc = pthread_mutex_lock(&tMutex);
		if (rc) {
			std::cerr << "could not lock the mutex in thread: " << threadName
					<< std::endl;
			isError = true;
			pthread_exit(NULL);
		}
		// Check to see if we need more entropy
		ioctl(rndout, RNDGETENTCNT, &entropyAvailable);
		if (entropyAvailable < (entropyPoolSizeBytes * 8)  / 2
				&& deq2.size() > 0) {
			// entropy pool level is below 'water mark', add more random bytes from deq2
			int addMoreBytes = entropyPoolSizeBytes	- (entropyAvailable >> 3);
			if (addMoreBytes > (int) deq2.size()) {
				addMoreBytes = deq2.size();
			}
			// Fill the entropy pool structure
			for (int i = 0; i < addMoreBytes; i++) {
				entropy.data[i] = deq2.front();
				deq2.pop_front();
			}
			entropy.buf_size = addMoreBytes;
			// Estimate the amount of entropy
			entropy.entropy_count = entropyAvailable + (addMoreBytes << 3);

			// Push the entropy out to the pool
			result = ioctl(rndout, RNDADDENTROPY, &entropy);
			if (result < 0) {
				std::cerr << "Cannot add more entropy to the pool, error: "
						<< result << std::endl;
				close(rndout);
				isError = true;
				pthread_exit(NULL);
			}
		}
		// Unlock the mutex
		rc = pthread_mutex_unlock(&tMutex);
		if (rc) {
			std::cerr << "could not unlock the mutex in thread: " << threadName
					<< std::endl;
			isError = true;
			pthread_exit(NULL);
		}
		usleep(heartBeatUsecs);
	}
	close(rndout);
	pthread_exit(NULL);
}

/**
 * Display usage message
 *
 */
void displayUsage() {
	std::cout << "***************************************************************"	<< std::endl;
	std::cout << "   TectroLabs - Entropy Sector - Entropy Pool Feeder Ver 1.0 "	        << std::endl;
	std::cout << "***************************************************************"	<< std::endl;
	std::cout <<"Usage: epf <path to epf.properties configuration file>" << std::endl;
	std::cout << std::endl;
}

/**
 * Parse arguments for extracting command line parameters
 *
 * @param argc
 * @param argv
 * @return int - 0 when run successfully
 */
int processArguments(int argc, const char **argv) {
	if (argc < 2) {
		std::cerr << std::endl << "Missing command line arguments" << std::endl << std::endl;
		displayUsage();
		return -1;
	}
	propFileName = argv[1];
	return 0;
}

/**
 * Retrieve the size in bytes of the kernel entropy pool
 *
 * @return true if entropy pool size retrieved successfully
 */
bool getEntropyPoolSize() {
	bool status = false;
	FILE *fp;
	char *line = NULL;
	size_t len = 0;

	fp = fopen(KERNEL_POOLSIZE_LOCATOIN, "r");
	if (fp == NULL) {
		return false;
	}

	if (getline(&line, &len, fp) != -1) {
		entropyPoolSizeBytes = atoi(line) / 8;
		if (entropyPoolSizeBytes > MAX_POOL_SIZE_BYTES) {
			entropyPoolSizeBytes = MAX_POOL_SIZE_BYTES;
		}
		status = true;
	}
	if (line != NULL) {
		free(line);
	}
	fclose(fp);
	return status;
}

/**
 * Validate configuration properties from the file
 *
 * @return true if configuration is valid
 */
bool isConfigurationValid() {
	std::string errString = "Could not find property ";
	if (config.getProperty(ENTROPY_HOST_PROPERTY_NAME).getStringValue().size() == 0) {
		std::cerr << errString << ENTROPY_HOST_PROPERTY_NAME << std::endl;
		return false;
	}

	if (config.getProperty(ENTROPY_PORT_PROPERTY_NAME).getStringValue().size() == 0) {
		std::cerr << errString << ENTROPY_PORT_PROPERTY_NAME << std::endl;
		return false;
	}

	if (!config.getProperty(ENTROPY_PORT_PROPERTY_NAME).isInteger()) {
		std::cerr << ENTROPY_PORT_PROPERTY_NAME << " is not an integer number" << std::endl;
		return false;
	}

	if (config.getProperty(ENTROPY_RESOURCE_PROPERTY_NAME).getStringValue().size() == 0) {
		std::cerr << errString << ENTROPY_RESOURCE_PROPERTY_NAME << std::endl;
		return false;
	}

	if (config.getProperty(ENTROPY_RESOURCE_BYTESTREAM_ENCRYPT_PROPERTY_NAME).getStringValue().size() == 0) {
		std::cerr << errString << ENTROPY_RESOURCE_BYTESTREAM_ENCRYPT_PROPERTY_NAME << std::endl;
		return false;
	}

	if (!config.getProperty(ENTROPY_RESOURCE_BYTESTREAM_ENCRYPT_PROPERTY_NAME).isBoolean()) {
		std::cerr << ENTROPY_RESOURCE_BYTESTREAM_ENCRYPT_PROPERTY_NAME << " is not a boolean" << std::endl;
		return false;
	}

	if (config.getProperty(ENTROPY_RESOURCE_BYTESTREAM_ENCRYPT_PROPERTY_NAME).getBoolValue() == true) {
			if(config.getProperty(ENTROPY_RESOURCE_BYTESTREAM_RSA_FILE_PROPERTY_NAME).getStringValue().size() == 0) {
				std::cerr << errString << ENTROPY_RESOURCE_BYTESTREAM_RSA_FILE_PROPERTY_NAME << std::endl;
				return false;
			}
			// Validate public key
			std::string pubKeyFileName = config.getProperty(ENTROPY_RESOURCE_BYTESTREAM_RSA_FILE_PROPERTY_NAME).getStringValue();
			pubKeyCryptor = new RSACryptor(pubKeyFileName.c_str(), true);
			if (!pubKeyCryptor->isInitialized()) {
				std::cerr << errString << "Could not use public key file: " << pubKeyFileName << std::endl;
				return false;
			}
			isStreamEncrypted = true;
	}

	if (config.getProperty(ENTROPY_REQUEST_SIZE_PROPERTY_NAME).getStringValue().size() == 0) {
		std::cerr << errString << ENTROPY_REQUEST_SIZE_PROPERTY_NAME << std::endl;
		return false;
	}

	if (!config.getProperty(ENTROPY_REQUEST_SIZE_PROPERTY_NAME).isInteger()) {
		std::cerr << ENTROPY_REQUEST_SIZE_PROPERTY_NAME << " is not an integer number" << std::endl;
		return false;
	}

	if (config.getProperty(ENTROPY_HOST_SSL_ENABLED_PROPERTY_NAME).getStringValue().size() == 0) {
		std::cerr << errString << ENTROPY_HOST_SSL_ENABLED_PROPERTY_NAME << std::endl;
		return false;
	}

	if (!config.getProperty(ENTROPY_HOST_SSL_ENABLED_PROPERTY_NAME).isBoolean()) {
		std::cerr << ENTROPY_HOST_SSL_ENABLED_PROPERTY_NAME << " is not a boolean" << std::endl;
		return false;
	}

	if (config.getProperty(ENTROPY_DWNLD_THREAD_PERIOD_USECS_PROPERTY_NAME).getStringValue().size() == 0) {
		std::cerr << errString << ENTROPY_DWNLD_THREAD_PERIOD_USECS_PROPERTY_NAME << std::endl;
		return false;
	}

	if (!config.getProperty(ENTROPY_DWNLD_THREAD_PERIOD_USECS_PROPERTY_NAME).isInteger()) {
		std::cerr << ENTROPY_DWNLD_THREAD_PERIOD_USECS_PROPERTY_NAME << " is not an integer number" << std::endl;
		return false;
	}

	if (config.getProperty(ENTROPY_FEEDER_THREAD_PERIOD_USECS_PROPERTY_NAME).getStringValue().size() == 0) {
		std::cerr << errString << ENTROPY_FEEDER_THREAD_PERIOD_USECS_PROPERTY_NAME << std::endl;
		return false;
	}

	if (!config.getProperty(ENTROPY_FEEDER_THREAD_PERIOD_USECS_PROPERTY_NAME).isInteger()) {
		std::cerr << ENTROPY_FEEDER_THREAD_PERIOD_USECS_PROPERTY_NAME << " is not an integer number" << std::endl;
		return false;
	}

	if (config.getProperty(ENTROPY_MAX_DEQ_SIZE_BYTES_PROPERTY_NAME).getStringValue().size() == 0) {
		std::cerr << errString << ENTROPY_MAX_DEQ_SIZE_BYTES_PROPERTY_NAME << std::endl;
		return false;
	}

	if (!config.getProperty(ENTROPY_MAX_DEQ_SIZE_BYTES_PROPERTY_NAME).isInteger()) {
		std::cerr << ENTROPY_MAX_DEQ_SIZE_BYTES_PROPERTY_NAME << " is not an integer number" << std::endl;
		return false;
	}
	maxDeqSizeBytes = config.getProperty(ENTROPY_MAX_DEQ_SIZE_BYTES_PROPERTY_NAME).getIntValue();

	return true;
}

/**
 * Main entry
 *
 * @param argc number of arguments
 * @rapam argv pointer to parameters
 *
 * @return return code
 */
int main(int argc, const char **argv) {

	int status = processArguments(argc, argv);
	if (status) {
		return status;
	}

	// Load configuration from properties file
	if (!config.loadFromFile((char*)propFileName.c_str())) {
		std::cerr << "Could not load configuration from propFileName" << std::endl;
		return -1;
	}

	if (!isConfigurationValid()) {
		return -1;
	}

	if (!getEntropyPoolSize()) {
		std::cerr << "Cannot get the size of the kernel entropy pool " << KERNEL_POOLSIZE_LOCATOIN << status << std::endl;
		return -1;
	}

	// Create the download thread
	pthread_create(&downloadThread, NULL, downloadBytes,
			(void*) "download thread");

	// Create threads for feeding the entropy pool
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_create(&entropyThreadArray[i], NULL, feedEntropyPool,
				(void*) "pool thread");
	}

	// Wait for feeding threads to finish
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(entropyThreadArray[i], NULL);
	}

	// If we got to this point then something went wrong
	// Shutdown the downloadBytes thread
	isError = true;

	// Wait for downloadBytes thread to finish
	pthread_join(downloadThread, NULL);

	return -1;
}

