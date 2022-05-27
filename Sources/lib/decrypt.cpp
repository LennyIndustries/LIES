/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/decrypt.hpp"

#include <utility>

// Constructor (Public)
decrypt::decrypt(lilog *log)
{
	this->myLog = log;
	
	this->inputText.clear();
	this->inputImage.clear();
	this->returnData.clear();
	this->passwd = "";
	this->runOption = 0;
	this->hash.clear();
	
	LOG(myLog, 1, "Decrypt created");
	std::cout << "Decrypt created\n";
}

// Public
// Functions
void decrypt::run()
{
	/*
	 * Valid options:
	 * Text -> CRC32 of text (1)
	 * Image -> Partial decryption (2)
	 * Text & Passwd -> Text decryption (5)
	 * Image & Passwd -> Full decryption (6)
	 */
	this->runOption = 0;
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
		case 2: // Partial decryption
			this->decryptImage();
			std::copy(this->inputText.begin(), this->inputText.end(), std::back_inserter(this->returnData));
			break;
		case 5: // Text decryption
			this->decryptText();
			std::copy(this->inputText.begin(), this->inputText.end(), std::back_inserter(this->returnData));
			break;
		case 6: // Full decryption
			this->decryptImage();
			this->decryptText();
			std::copy(this->inputText.begin(), this->inputText.end(), std::back_inserter(this->returnData));
			break;
		default:
			std::string s = "Invalid";
			std::copy(s.begin(), s.end(), std::back_inserter(this->returnData));
	}
}

void decrypt::decryptImage()
{
	LOG(this->myLog, 1, "Decrypting text from image");
	cryptLib::colorPrint("Decrypting text from image", WAITMSG);
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
	if (reserved == 0) // The image has not been encrypted by LIES
	{
		LOG(myLog, 2, "Image not encrypted by LIES");
		cryptLib::colorPrint("Image not encrypted by LIES", ERRORCLR);
		return;
	}
	else
	{
		std::copy(this->headerData.begin() + 6, this->headerData.begin() + 10, std::back_inserter(this->hash));
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
	
	std::copy(tmpTextVector.begin() + 1, tmpTextVector.end() - 1, std::back_inserter(this->inputText));
	
	// Save text for testing
	std::ofstream text("outputImage.txt", std::ios::out | std::ofstream::trunc);
	std::copy(this->inputText.begin(), this->inputText.end(), std::ostreambuf_iterator <char>(text));
	text.close();
}

void decrypt::decryptText()
{
	LOG(this->myLog, 1, "Decrypting text with password");
	cryptLib::colorPrint("Decrypting text with password", WAITMSG);

//	this->hash = cryptLib::generateHash(this->inputText);
	
	const Botan::BigInt n = 1000000000000000;
	std::vector <uint8_t> tweak; // tweak (salt) based on text hash
	tweak.clear();
	if (!this->hash.empty()) // If there is a hash
	{
		std::cout << "Beware, here be hashes" << std::endl;
		tweak = Botan::unlock(this->hash);
	}
	
	std::cout << "Test: 01\n";
	std::unique_ptr <Botan::PBKDF> pbkdf(Botan::PBKDF::create("PBKDF2(SHA-256)"));
	// Decryption
	std::cout << "Test: 02\n";
	std::unique_ptr <Botan::Cipher_Mode> decrypt = Botan::Cipher_Mode::create("AES-256/SIV", Botan::DECRYPTION);
	std::cout << "Test: 03\n";
	Botan::secure_vector <uint8_t> key = pbkdf->pbkdf_iterations(decrypt->maximum_keylength(), this->passwd, tweak.data(), tweak.size(), 100000);
	std::cout << "Test: 04\n";
	decrypt->set_key(key);
	std::cout << "Test: 05\n";
	Botan::secure_vector <uint8_t> dataVector(this->inputText.data(), this->inputText.data() + this->inputText.size());
	std::cout << "Test: 06\n";
	decrypt->finish(dataVector);
	std::cout << "Test: 07\n";
	std::copy(dataVector.begin(), dataVector.end(), std::back_inserter(this->inputText));
	std::cout << "Test: 08\n";
}

// Getters / Setters
void decrypt::setImage(std::vector <char> setTo)
{
	this->inputImage = std::move(setTo);
}

void decrypt::setText(std::vector <char> setTo)
{
	this->inputText = std::move(setTo);
}

void decrypt::setPasswd(std::string setTo)
{
	this->passwd = std::move(setTo);
}

std::vector <char> decrypt::getData()
{
	return returnData;
}

// Protected
// Private
// Destructor (Public)
decrypt::~decrypt() = default;