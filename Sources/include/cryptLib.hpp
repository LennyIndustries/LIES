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
//#include <cstdio>
#include <iostream>
#include <cstring>
#include <vector>

// Definitions

class cryptLib
{
public:
	// Functions
	static std::string printableVector(const std::vector<char> &vectorToPrint);
	static std::size_t vectorFind(const std::vector<char> &searchVector, char searchChar);
	static std::vector<char> subVector(const std::vector<char> &startVector, std::size_t startPos, std::size_t charCount = std::string::npos);
	static bool vectorCompare(const std::vector<char> &vector, const std::string &string);
protected:
	// Functions
	static void getImageData(std::vector<char> &image, std::vector<char> &headerReturn, std::vector<char> &dataReturn);
private:
};


#endif //LIES_CRYPTLIB_HPP
