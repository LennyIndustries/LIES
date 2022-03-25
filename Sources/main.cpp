/*
 * Created by Leander @ Lenny Industries on 18/03/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

// Libraries
#include "lilog.hpp"
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
	printf("Hello World\n");
	
	printf("OpenSSL Version: %i\n", OPENSSL_VERSION_NUMBER);
	
	printf("Botan Version: %i\n", BOTAN_VERSION_CODE);
	
//	printf("QT Version: %i-%i-%i\n", QT_VERSION);
	
	return ERR_0;
}