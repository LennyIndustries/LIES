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

// Definitions

class decrypt : private cryptLib
{
public:
	// Con- Destructor
	decrypt(lilog *log);
	~decrypt();
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
	void decryptImage();
	void decryptText();
	// Variables
	lilog *myLog;
	std::vector <char> inputText, inputImage, returnData;
	std::string passwd;
	
	std::vector <char> headerData, imageData;
	
	char runOption;
	Botan::secure_vector<uint8_t> hash;
};


#endif //LIES_DECRYPT_HPP
