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

class encrypt
{
public:
	encrypt(char *image, char *message);
	// Functions
	void encryptImage();
	// Getters / Setters
	void setImage(char *image);
	void setText(char *message);
	char *getImage();
protected:
private:
	// Variables
	char *inputImage;
	char *text;
	char *returnImage;
	
	char *headerData, *imageData;
};


#endif //LIES_ENCRYPT_HPP
