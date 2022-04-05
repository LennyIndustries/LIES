/*
 * Created by Leander @ Lenny Industries on 18/03/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
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
#include <cstdio>
#include <openssl/opensslv.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <botan/botan.h>
#include <QtCore/qglobal.h>

// Definitions

// Error codes
#define ERR_0 0 // No error

// Functions

int main(int argc, char **argv)
{
	printf("Hello World - Server\n");
	
	printf("OpenSSL Version: %i\n", OPENSSL_VERSION_NUMBER);
	
	printf("Botan Version: %i\n", BOTAN_VERSION_CODE);

	printf("QT Version: %i\n", QT_VERSION);
	
	printf("Log testing\n");
	lilog myLog("test.log");
	
	myLog.clearLogFile();
	
	myLog.log(1, __FILE__, __LINE__, "Test %i", 0);
	LOG(myLog, 1, "Test %i", 1);
	LOG(myLog, 2, "Test %i", 2);
	LOG(myLog, 3, "Test %i", 3);
	LOG(myLog, 4, "Test %i", 4);
	
	while (true) {};
	
//	myLog.close();
//	myLog.kill();
	
	return ERR_0;
}