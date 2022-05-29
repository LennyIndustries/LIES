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

// Definitions
#define MAX_CHARS 8190 // 8190 chars takes up 65520 bytes, to write to total in the image only 2 bytes are used

class encrypt : private cryptLib
{
public:
	// Con- Destructor
	encrypt(lilog *log);
	~encrypt();
	// Functions
	void run();
	// Getters / Setters
	void setImage(std::vector <uint8_t> setTo);
	void setText(std::vector <uint8_t> setTo);
	void setPasswd(std::vector <uint8_t> setTo);
	std::vector <uint8_t> getData();
protected:
private:
	// Functions
	void encryptImage();
	void encryptText();
	// Variables
	lilog *myLog;
	std::vector <uint8_t> inputText, inputImage, returnData, passwd;
	
	std::vector <uint8_t> headerData, imageData;
	
	char runOption;
	Botan::secure_vector<uint8_t> hash;
};


#endif //LIES_ENCRYPT_HPP
