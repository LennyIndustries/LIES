/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/decrypt.hpp"

// Constructor (Public)
decrypt::decrypt(std::vector <char> image, lilog *log)
{
	this->myLog = log;
	this->inputImage = std::move(image);
	
	LOG(myLog, 1, "Decrypt created");
	std::cout << "Decrypt created\n";
	decryptImage();
}

// Public
// Functions
void decrypt::decryptImage()
{
	// Get image data
	cryptLib::getImageData(this->inputImage, this->headerData, this->imageData);
	// Get info from header
	// http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
	int reserved = *(int *) &this->headerData[6];
	int with = *(int *) &this->headerData[18];
	int height = *(int *) &this->headerData[22];
	int bitPerPixel = *(short int *) &this->headerData[28];
	// with * height = TOTAL PIXELS; * bitsPerPixel = TOTAL BITS
	int maxChars = (with * height * bitPerPixel);
	std::cout << "Reserved = " << reserved << std::endl;
	std::cout << "With = " << with << std::endl;
	std::cout << "Height = " << height << std::endl;
	std::cout << "Bit per pixel = " << bitPerPixel << std::endl;
	std::cout << "Total bits in image = " << maxChars << std::endl;
	// Checking image compatibility
	if (reserved != ENCRYPTED) // The image has not been encrypted by LIES
	{
		LOG(myLog, 2, "Image not encrypted by LIES");
		cryptLib::colorPrint("Image not encrypted by LIES", ERRORCLR);
		return;
	}
	// Getting text
	std::vector <char> tmpTextVector;
	unsigned int counter = 0;
	unsigned char indexer = 0;
	char storage = '\0';
	unsigned char mask = 0x01;
	do
	{
		for (indexer = 0; indexer < 8; indexer++)
		{
			storage <<= 1;
			storage = (char) (storage | (imageData[indexer + (counter * 8)] & mask));
		}
		counter++;
	} while ((storage != ETX) && ((indexer + (counter * 8)) <= maxChars));
	
	if ((storage != ETX) || (counter > maxChars))
	{
		LOG(myLog, 2, "Failed to find ETX");
		cryptLib::colorPrint("Failed to find ETX", ERRORCLR);
		return;
	}
	
	for (unsigned int i = 0; i < counter; i++)
	{
		storage = '\0';
		for (char j = 0; j < 8; j++)
		{
			storage <<= 1;
			storage = (char) (storage | (imageData[j + (i * 8)] & mask));
		}
		tmpTextVector.push_back(storage);
	}
	
	if (tmpTextVector[0] != STX)
	{
		LOG(myLog, 2, "Failed to find STX");
		cryptLib::colorPrint("Failed to find STX", ERRORCLR);
		return;
	}
	
	std::copy(tmpTextVector.begin() + 1, tmpTextVector.end() - 1, std::back_inserter(this->returnText));
	
	// Save text for testing
	std::ofstream text("outputImage.txt", std::ios::out);
	std::copy(this->returnText.begin(), this->returnText.end(), std::ostreambuf_iterator <char>(text));
	text.close();
}

// Getters / Setters
void decrypt::setImage(std::vector <char> image)
{
	this->inputImage = std::move(image);
}

std::vector <char> decrypt::getText()
{
	return returnText;
}

// Protected
// Private
// Destructor (Public)
decrypt::~decrypt() = default;