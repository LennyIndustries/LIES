/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/encrypt.hpp"

// Constructor (Public)
encrypt::encrypt(std::vector <char> image, std::vector <char> message)
{
	this->inputImage = std::move(image);
	this->text = std::move(message);
//	this->returnImage = nullptr;
//	this->headerData = nullptr;
//	this->imageData = nullptr;
	
	std::cout << "Encrypt created\n" << "inputImage = " << cryptLib::printableVector(this->inputImage) << "\ntext = " << cryptLib::printableVector(this->text) << std::endl;
	encryptImage();
}

// Public
// Functions
void encrypt::encryptImage()
{
	cryptLib::getImageData(this->inputImage, this->headerData, this->imageData);
	
	std::ofstream image ("outputImage.bmp", std::ios::out | std::ofstream::binary);
	
	std::copy(this->headerData.begin(), this->headerData.end(), std::ostreambuf_iterator<char>(image));
	std::copy(this->imageData.begin(), this->imageData.end(), std::ostreambuf_iterator<char>(image));
	
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
// Destructor (?)