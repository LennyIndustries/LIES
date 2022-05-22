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

//#include <cstdio>
#include <iostream>
#include <cstring>
#include <vector>
#include <Windows.h>
#include <botan/botan.h>
#include <botan/uuid.h>
#include <botan/rsa.h>
#include <botan/pkcs8.h>
#include <botan/pk_keys.h>
#include <botan/pubkey.h>
#include <botan/hex.h>
#include <botan/secmem.h>

// Definitions
#define ENCRYPTED 0x18 // CAN (Cancel) :: 0001 1000 :: 24
#define STX 0x2 // STX (Start Of Text) :: 0000 0010 :: 2
#define ETX 0x3 // ETX (End Of Text) :: 0000 0011 :: 3
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
	static std::string printableVector(const std::vector <char> &vectorToPrint);
	static std::size_t vectorFind(const std::vector <char> &searchVector, char searchChar);
	static std::vector <char> subVector(const std::vector <char> &startVector, std::size_t startPos, std::size_t charCount = std::string::npos);
	static bool vectorCompare(const std::vector <char> &vector, const std::string &string);
	static void colorPrint(const std::string &message, char color = 0x7);
protected:
	// Functions
	static void getImageData(std::vector <char> &image, std::vector <char> &headerReturn, std::vector <char> &dataReturn);
private:
};


#endif //LIES_CRYPTLIB_HPP
