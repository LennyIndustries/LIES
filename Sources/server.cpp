/*
 * Created by Leander @ Lenny Industries on 18/03/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * LIES: Lenny Industries Encryption Service
 * Can encrypt and decrypt text into bmp images over benternet.
 * The server handles all requests from clients.
 */

/*
 * Bulk of program, get a message over benternet and solves the encryption / decryption.
 * Later add options to add extra encryption (RSA, ...).
 * Add checksum to info received over benternet.
 * Multithreading server, process multiple clients at once.
 * Possible other stuff.
 */

// Libraries
#include "include/lilog.hpp"
#include "include/inputHandler.hpp"

#include <cstdio>
#include <openssl/opensslv.h>
#include <botan/botan.h>
#include <QtCore/qglobal.h>
#include <zmq.h>

// Definitions

// Error codes
#define ERR_0 0 // No error

int main(int argc, char **argv)
{
	inputHandler myInputHandler(argc, argv); // InputHandler test
	if (myInputHandler.cmdOptionExists("-hello"))
	{
		int zmq_major, zmq_minor, zmq_patch;
		zmq_version(&zmq_major, &zmq_minor, &zmq_patch);
		
		printf("Hello World - Server\n");
		printf("ZMQ Version: %i.%i.%i\n", zmq_major, zmq_minor, zmq_patch);
		printf("OpenSSL Version: %i\n", OPENSSL_VERSION_NUMBER);
		printf("Botan Version: %i\n", BOTAN_VERSION_CODE);
		printf("QT Version: %i\n", QT_VERSION);
	}
	
	std::string logName = myInputHandler.getCmdOption("-log");
	if (logName.empty())
	{
		logName = "test.log";
	}
	
	printf("Log testing\n");
	auto *myLog = lilog::create(logName);
	
	myLog->log(1, __FILE__, __LINE__, "Test %i", 0);
//	myLog->clearLogFile();
	LOG(myLog, 1, "Test %i", 1);
	LOG(myLog, 2, "Test %i", 2);
	LOG(myLog, 3, "Test %i", 3);
	LOG(myLog, 4, "Test %i", 4);

	myLog->close();
	myLog->open();
	myLog->kill();
	
	printf("Done\n");
	
	return ERR_0;
}