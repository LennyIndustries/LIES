/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * Helps out with encryption and decryption
 */

#ifndef LIES_CRYPTLIB_HPP
#define LIES_CRYPTLIB_HPP

// Libraries
#include "include/lilog.hpp"

#include <iostream>
#include <cstring>
#include <vector>
#include <Windows.h>
#include <utility>
#include <fstream>
// Botan
#include <botan/botan.h>
#include <botan/uuid.h>
#include <botan/rsa.h>
#include <botan/pkcs8.h>
#include <botan/pk_keys.h>
#include <botan/pubkey.h>
#include <botan/hex.h>
#include <botan/secmem.h>
#include <botan/hash.h>
#include <botan/fpe_fe1.h>
#include <botan/pbkdf.h>
#include <botan/cipher_mode.h>

// Definitions
// Console colours
#define DEFAULTCLR 0xe
#define ERRORCLR 0xc
#define MSGCLR 0xa
#define ALTMSGCLR 0x9
#define WAITMSG 0xd

class cryptLib
{
public:
	// Functions
	static std::string printableVector(const std::vector <uint8_t> &vectorToPrint);
	static std::size_t vectorFind(const std::vector <uint8_t> &searchVector, char searchChar);
	static std::vector <uint8_t> subVector(const std::vector <uint8_t> &startVector, std::size_t startPos, std::size_t charCount = std::string::npos);
	static bool vectorCompare(const std::vector <uint8_t> &vector, const std::string &string);
	static void colorPrint(const std::string &message, char color = 0x7);
	static Botan::secure_vector<uint8_t> generateHash(const std::vector <uint8_t>& hashThis);
protected:
	// Functions
	static void getImageData(std::vector <uint8_t> &image, std::vector <uint8_t> &headerReturn, std::vector <uint8_t> &dataReturn);
private:
};


#endif //LIES_CRYPTLIB_HPP
