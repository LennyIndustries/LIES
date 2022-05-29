/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/encrypt.hpp"

#include <utility>

// Constructor (Public)
encrypt::encrypt(lilog *log)
{
	this->myLog = log;
	
	this->inputText.clear();
	this->inputImage.clear();
	this->returnData.clear();
	this->passwd.clear();
	this->runOption = 0;
	this->hash.clear();
	
	LOG(myLog, 1, "Encrypt created");
	std::cout << "Encrypt created\n";
}

// Public
// Functions
void encrypt::run()
{
	/*
	 * Valid options:
	 * Text -> CRC32 of text (1)
	 * Image -> CRC32 of image (2)
	 * Text & Image -> Partial encryption (3)
	 * Text & Passwd -> Text encryption (5)
	 * Text & Image & Passwd -> Full encryption (7)
	 */
	if (!this->inputText.empty())
		this->runOption |= 0x1;
	if (!this->inputImage.empty())
		this->runOption |= 0x2;
	if (!this->passwd.empty())
		this->runOption |= 0x4;
	
	switch (this->runOption)
	{
		case 1: // CRC32 of text
			this->hash = cryptLib::generateHash(this->inputText);
			std::copy(this->hash.begin(), this->hash.end(), std::back_inserter(this->returnData));
			break;
		case 2: // CRC32 of image
			this->hash = cryptLib::generateHash(this->inputImage);
			std::copy(this->hash.begin(), this->hash.end(), std::back_inserter(this->returnData));
			break;
		case 3: // Partial encryption
			this->hash = cryptLib::generateHash(this->inputText);
			this->encryptImage();
			std::copy(this->inputImage.begin(), this->inputImage.end(), std::back_inserter(this->returnData));
			break;
		case 5: // Text encryption
			this->encryptText();
			std::copy(this->inputText.begin(), this->inputText.end(), std::back_inserter(this->returnData));
			break;
		case 7: // Full encryption
			this->encryptText();
			this->encryptImage();
			std::copy(this->inputImage.begin(), this->inputImage.end(), std::back_inserter(this->returnData));
			break;
		default:
			std::string s = "Invalid";
			std::copy(s.begin(), s.end(), std::back_inserter(this->returnData));
	}
}

void encrypt::encryptImage()
{
	LOG(this->myLog, 1, "Encrypting text in image");
	cryptLib::colorPrint("Encrypting text in image", WAITMSG);
	std::string error = "ERROR";
	// Text length
	unsigned short int textLength = this->inputText.size();
	if (textLength > MAX_CHARS)
	{
		error = "Too many characters";
		this->inputImage.clear();
		std::copy(error.begin(), error.end(), std::back_inserter(this->inputImage));
	}
	// Get image data
	cryptLib::getImageData(this->inputImage, this->headerData, this->imageData);
	// Get info from header
	// http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
	int reserved = *(int *) &this->headerData[6];
	int with = *(int *) &this->headerData[18];
	int height = *(int *) &this->headerData[22];
	int bitPerPixel = *(short int *) &this->headerData[28];
	// with * height = TOTAL PIXELS; * bitsPerPixel = TOTAL BITS; / 8 = TOTAL BYTES; / 8 = MAX CHARS IN IMAGE (8 bytes per char) - 2 * 8 (text length)
	int maxChars = (((with * height * bitPerPixel) / 8) / 8) - (2 * 8);
	std::cout << "Reserved = " << reserved << std::endl;
	std::cout << "With = " << with << std::endl;
	std::cout << "Height = " << height << std::endl;
	std::cout << "Bit per pixel = " << bitPerPixel << std::endl;
	std::cout << "Maximum characters in image = " << maxChars << std::endl;
	// Checking image and text compatibility
	if (bitPerPixel < 8) // If pixel depth is not enough the program will visibly alter the image
	{
		LOG(this->myLog, 2, "Pixel depth is too low: %i", bitPerPixel);
		cryptLib::colorPrint("Pixel depth is too low", ERRORCLR);
		error = "Pixel depth is too low";
		this->inputImage.clear();
		std::copy(error.begin(), error.end(), std::back_inserter(this->inputImage));
		return;
	}
	if (this->inputText.size() > maxChars) // The total amount of characters in the text can not be put in the image
	{
		LOG(this->myLog, 2, "(Encrypted) Character count too high: %i > %i", this->inputText.size(), maxChars);
		cryptLib::colorPrint("(Encrypted) Character count too high", ERRORCLR);
		error = "(Encrypted) Character count too high";
		this->inputImage.clear();
		std::copy(error.begin(), error.end(), std::back_inserter(this->inputImage));
		return;
	}
	if (reserved != 0) // The image has already been encrypted. We put the text hash in here
	{
		LOG(this->myLog, 2, "Image already encrypted or incompatible");
		cryptLib::colorPrint("Image already encrypted or incompatible", ERRORCLR);
		error = "Image already encrypted or incompatible";
		this->inputImage.clear();
		std::copy(error.begin(), error.end(), std::back_inserter(this->inputImage));
		return;
	}
	// Write the text to the image, prep
	std::vector <uint8_t> textToWrite; // Vector with all the text and size
	std::string textLengthString = std::to_string(textLength); // Text size
	if (textLengthString.length() == 1)
	{
		textLengthString = "0" + textLengthString;
	}
	std::copy(textLengthString.begin(), textLengthString.end(), std::back_inserter(textToWrite));
	std::copy(this->inputText.begin(), this->inputText.end(), std::back_inserter(textToWrite)); // Text
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
	// Setting hash in header
	std::vector <uint8_t> tmpHash = Botan::unlock(this->hash);
	std::cout << "Inserting hash into header\n";
	this->headerData[6] = tmpHash[0];
	this->headerData[7] = tmpHash[1];
	this->headerData[8] = tmpHash[2];
	this->headerData[9] = tmpHash[3];
	// Write it all back
	this->inputImage.clear();
	std::copy(this->headerData.begin(), this->headerData.end(), std::back_inserter(this->inputImage)); // Copy the header
	std::copy(this->imageData.begin(), this->imageData.end(), std::back_inserter(this->inputImage)); // Copy the image
}

void encrypt::encryptText()
{
	LOG(this->myLog, 1, "Encrypting text with password");
	cryptLib::colorPrint("Encrypting text with password", WAITMSG);
	
	this->hash = cryptLib::generateHash(this->inputText);
	
	const Botan::BigInt n = 1000000000000000;
	std::vector <uint8_t> tweak = Botan::unlock(this->hash); // tweak (salt) based on text hash
	if (this->runOption == 5) // If only encrypting text, do not use hash as salt, it is unknown at decrypt
	{
		std::cout << "Cleaning salt\n";
		tweak.clear();
	}
	
	std::unique_ptr <Botan::PBKDF> pbkdf(Botan::PBKDF::create("PBKDF2(SHA-256)"));
	// Encryption
	std::unique_ptr <Botan::Cipher_Mode> encryption = Botan::Cipher_Mode::create("AES-256/SIV", Botan::ENCRYPTION);
	Botan::secure_vector <uint8_t> key = pbkdf->pbkdf_iterations(encryption->maximum_keylength(), cryptLib::printableVector(this->passwd), tweak.data(), tweak.size(), 100000);
	encryption->set_key(key);
	Botan::secure_vector <uint8_t> dataVector(this->inputText.data(), this->inputText.data() + this->inputText.size());
	encryption->finish(dataVector);
	this->inputText.clear();
	std::copy(dataVector.begin(), dataVector.end(), std::back_inserter(this->inputText));
}

// Getters / Setters
void encrypt::setImage(std::vector <uint8_t> setTo)
{
	std::copy(setTo.begin(), setTo.end(), std::back_inserter(this->inputImage));
}

void encrypt::setText(std::vector <uint8_t> setTo)
{
	std::copy(setTo.begin(), setTo.end(), std::back_inserter(this->inputText));
}

void encrypt::setPasswd(std::vector <uint8_t> setTo)
{
	this->passwd = std::move(setTo);
}

std::vector <uint8_t> encrypt::getData()
{
	return this->returnData;
}

// Protected
// Private
// Destructor (Public)
encrypt::~encrypt() = default;