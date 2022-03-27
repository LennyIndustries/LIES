/*
 * Created by Leander @ Lenny Industries on 18/03/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

// Libraries
#include "include/lilog.hpp"
#include <cstdio>
#include <openssl/opensslv.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <botan/botan.h>
//#include <QtCore/qglobal.h>

// Definitions

// Error codes
#define ERR_0 0 // No error

// Functions

int main(int argc, char **argv)
{
	printf("Hello World - Server\n");
	
	printf("OpenSSL Version: %i\n", OPENSSL_VERSION_NUMBER);
	
	printf("Botan Version: %i\n", BOTAN_VERSION_CODE);

//	printf("QT Version: %i\n", QT_VERSION);
	
	char *logFileName = nullptr;
	strcpy(logFileName, "test.log");
	lilog myLog(logFileName);
	
	lilog::sayHello();
	
	myLog.clearLogFile();
	
	myLog.log(1, __FILE__, __LINE__, "Test %i", 0);
	LOG(myLog, 1, "Test %i", 1);
	LOG(myLog, 2, "Test %i", 2);
	LOG(myLog, 3, "Test %i", 3);
	LOG(myLog, 4, "Test %i", 4);
	
	return ERR_0;
}