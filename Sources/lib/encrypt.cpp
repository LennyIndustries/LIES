/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/encrypt.hpp"

// Constructor (Public)
encrypt::encrypt(std::vector <char> image, std::vector <char> message, lilog *log)
{
	this->myLog = log;
	this->inputImage = std::move(image);
	this->text = std::move(message);
//	this->returnImage = nullptr;
//	this->headerData = nullptr;
//	this->imageData = nullptr;
	
	LOG(myLog, 1, "Encrypt created");
	std::cout << "Encrypt created\n";
	encryptImage();
}

// Public
// Functions
void encrypt::encryptImage()
{
	// Get image data
	cryptLib::getImageData(this->inputImage, this->headerData, this->imageData);
	// Get info from header
	// http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
//	int imageFileSize = *(int*) &this->headerData[2];
	int reserved = *(int *) &this->headerData[6];
//	int dataOffset = *(int*) &this->headerData[10];
	int with = *(int *) &this->headerData[18];
	int height = *(int *) &this->headerData[22];
	int bitPerPixel = *(int *) &this->headerData[28];
	// with * height = TOTAL PIXELS; * bitsPerPixel = TOTAL BITS; / 8 = TOTAL BYTES; / 8 = MAX CHARS IN IMAGE (8 bytes per char)
	int maxChars = ((with * height * bitPerPixel) / 8) / 8;
//	std::cout << "Image file size = " << imageFileSize << std::endl;
	std::cout << "Reserved = " << reserved << std::endl;
//	std::cout << "Data offset = " << dataOffset << std::endl;
	std::cout << "With = " << with << std::endl;
	std::cout << "Height = " << height << std::endl;
	std::cout << "Bit per pixel = " << bitPerPixel << std::endl;
	std::cout << "Maximum characters in image = " << maxChars << std::endl;
	// Checking image and text compatibility
	if (bitPerPixel < 8) // If pixel depth is not enough the program will visibly alter the image
	{
		LOG(myLog, 2, "Pixel depth is too low: %i", bitPerPixel);
		cryptLib::colorPrint("Pixel depth is too low", ERRORCLR);
		return;
	}
	if (text.size() > maxChars) // The total amount of characters in the text can not be put in the image
	{
		LOG(myLog, 2, "Character count too high: %i > %i", text.size(), maxChars);
		cryptLib::colorPrint("Character count too high", ERRORCLR);
		return;
	}
	if (reserved == ENCRYPTED) // The image has already been encrypted
	{
		LOG(myLog, 2, "Image already encrypted");
		cryptLib::colorPrint("Image already encrypted", ERRORCLR);
		return;
	}
	// Write the text to the image, prep
	std::vector <char> textToWrite; // Vector with all the text and tags
	textToWrite.push_back(STX); // Start tag
	std::copy(this->text.begin(), this->text.end(), std::back_inserter(textToWrite)); // Text
	textToWrite.push_back(ETX); // End tag
	// Variables
	unsigned char mask = 0x80;
	unsigned char storageText = 0x00;
	unsigned char storageImage = 0x00;
	// Encrypt the text in the image
	for (unsigned int i = 0; i < textToWrite.size(); i++) // Loop trough all the text
	{
		for (unsigned int j = 0; j < 8; j++) // Byte loop
		{
			storageText = (textToWrite[i] & (mask >> j)) >> (7 - j);
			storageImage = (this->imageData[j + (8 * i)] & ~(mask >> 7));
			this->imageData[j + (8 * i)] = (char) (storageImage | storageText);
		}
	}
	// Set ENCRYPT flag in header
	this->headerData[6] = ENCRYPTED;
	// Write it all to returnImage
	std::copy(this->headerData.begin(), this->headerData.end(), std::back_inserter(this->returnImage)); // Copy the header
	std::copy(this->imageData.begin(), this->imageData.end(), std::back_inserter(this->returnImage)); // Copy the image
	// Save image for testing
	std::ofstream image("outputImage.bmp", std::ios::out | std::ofstream::binary);
	std::copy(this->headerData.begin(), this->headerData.end(), std::ostreambuf_iterator <char>(image));
	std::copy(this->imageData.begin(), this->imageData.end(), std::ostreambuf_iterator <char>(image));
	image.close();
}

// Getters / Setters
void encrypt::setImage(std::vector <char> image)
{
	this->inputImage = std::move(image);
}

void encrypt::setText(std::vector <char> message)
{
	this->text = std::move(message);
}

std::vector <char> encrypt::getImage()
{
	return returnImage;
}

// Protected
// Private
// Destructor (Public)
encrypt::~encrypt() = default;