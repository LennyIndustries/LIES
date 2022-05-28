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

class encrypt : private cryptLib
{
public:
	// Con- Destructor
	encrypt(lilog *log);
	~encrypt();
	// Functions
	void run();
	// Getters / Setters
	void setImage(std::vector <char> setTo);
	void setText(std::vector <char> setTo);
	void setPasswd(std::string setTo);
	std::vector <char> getData();
protected:
private:
	// Functions
	void encryptImage();
	void encryptText();
	// Variables
	lilog *myLog;
	std::vector <char> inputText, inputImage, returnData;
	std::string passwd;
	
	std::vector <char> headerData, imageData;
	
	char runOption;
	Botan::secure_vector<uint8_t> hash;
};


#endif //LIES_ENCRYPT_HPP
