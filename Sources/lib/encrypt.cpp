/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/encrypt.hpp"

// Constructor (Public)
encrypt::encrypt(char *image, char *message)
{
	this->inputImage = image;
	this->text = message;
	this->returnImage = nullptr;
	this->headerData = nullptr;
	this->imageData = nullptr;
	
	std::cout << "Encrypt created\n" << "inputImage = " << inputImage << "\ntext = " << text << std::endl;
}

// Public
// Functions
void encrypt::encryptImage()
{
	if (!(cryptLib::getImageData(this->inputImage, &this->headerData, &this->imageData)))
	{
		std::cout << "Failed to get image data from: " << _fullpath(nullptr, this->inputImage, _MAX_PATH) << std::endl;
	}
}

// Getters / Setters
void encrypt::setImage(char *image)
{
	this->inputImage = image;
}

void encrypt::setText(char *message)
{
	this->text = message;
}

char *encrypt::getImage()
{
	return returnImage;
}

// Protected
// Private
// Destructor (?)