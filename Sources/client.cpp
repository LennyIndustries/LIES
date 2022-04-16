/*
 * Created by Leander @ Lenny Industries on 25/03/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * LIES: Lenny Industries Encryption Service
 * Can encrypt and decrypt text into bmp images over benternet.
 * The client checks the input given and sends it to the server to process.
 */

// Libraries
#include "include/lilog.hpp"
#include "include/inputHandler.hpp"

#include <cstdio>
#include <openssl/opensslv.h>
#include <botan/botan.h>
#include <QtCore/qglobal.h>

// Definitions

// Error codes
#define ERR_0 0 // No error

int main(int argc, char **argv)
{
	printf("Hello World\nClient\n");
	
	printf("OpenSSL Version: %i\n", OPENSSL_VERSION_NUMBER);
	
	printf("Botan Version: %i\n", BOTAN_VERSION_CODE);

	printf("QT Version: %i\n", QT_VERSION);
	
	return ERR_0;
}
