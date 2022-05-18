/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * Handles all encryption requests
 */

#ifndef LIES_ENCRYPT_HPP
#define LIES_ENCRYPT_HPP

// Libraries
#include "cryptLib.hpp"

#include <vector>
#include <utility>
#include <fstream>

// Definitions

class encrypt : private cryptLib
{
public:
	// Con- Destructor
	encrypt(std::vector <char> image, std::vector <char> message, lilog *log);
	~encrypt();
	// Functions
	void encryptImage();
	// Getters / Setters
	void setImage(std::vector <char> image);
	void setText(std::vector <char> message);
	std::vector <char> getImage();
protected:
private:
	// Variables
	lilog *myLog;
	std::vector <char> inputImage, returnImage;
	std::vector <char> text;
	std::vector <char> headerData, imageData;
};


#endif //LIES_ENCRYPT_HPP
