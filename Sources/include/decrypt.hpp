/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * Handles all decryption requests
 */

#ifndef LIES_DECRYPT_HPP
#define LIES_DECRYPT_HPP

// Libraries
#include "cryptLib.hpp"

#include <vector>
#include <utility>
#include <fstream>

// Definitions

class decrypt : private cryptLib
{
public:
	// Con- Destructor
	decrypt(std::vector <char> image, lilog *log);
	~decrypt();
	// Functions
	void decryptImage();
	// Getters / Setters
	void setImage(std::vector <char> image);
	std::vector <char> getText();
protected:
private:
	// Variables
	lilog *myLog;
	std::vector <char> inputImage;
	std::vector <char> returnText;
	std::vector <char> headerData, imageData;
};


#endif //LIES_DECRYPT_HPP
