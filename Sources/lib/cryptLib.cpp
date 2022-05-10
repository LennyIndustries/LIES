/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/cryptLib.hpp"

// Constructor (?)
// Public
std::string cryptLib::printableVector(const std::vector <char> &vectorToPrint)
{
	std::string returnString;
	for (char i: vectorToPrint)
	{
		returnString += i;
	}
	return returnString;
}

std::size_t cryptLib::vectorFind(const std::vector <char>& searchVector, char searchChar)
{
	for (int i = 0; i < searchVector.size(); i++)
	{
		if (searchVector[i] == searchChar)
		{
			return i;
		}
	}
	return std::string::npos;
}

std::vector <char> cryptLib::subVector(const std::vector <char>& startVector, std::size_t startPos, std::size_t charCount)
{
	std::vector <char> returnVector;
	
	if (charCount == std::string::npos)
	{
		for (std::size_t i = startPos; i < startVector.size(); i++)
		{
			returnVector.push_back(startVector[i]);
		}
	}
	else
	{
		for (std::size_t i = startPos; i < startPos + charCount; i++)
		{
			returnVector.push_back(startVector[i]);
		}
	}
	
	return returnVector;
}

bool cryptLib::vectorCompare(const std::vector <char>& vector, const std::string& string)
{
//	char tempCharArray[string.length()];
//	strcpy(tempCharArray, string.c_str());
	std::vector <char> tempVector(string.begin(), string.end());
	return (vector == tempVector);
}

// Protected
void cryptLib::getImageData(std::vector <char> &image, std::vector <char> &headerReturn, std::vector <char> &dataReturn)
{
	// File
//	FILE *image = nullptr; // Change to istream
	// Image data
	std::vector <char> headerData;
//	int imageFileSize = 0;
	int dataOffset = 0;
	// Return data
//	*headerReturn = nullptr;
//	*dataReturn = nullptr;
	
	// Getting data from image
	// Opening image
//	image = fopen(_fullpath(nullptr, imageP, _MAX_PATH), "rb");
//	// Checking if image is opened
//	if (image == nullptr)
//	{
//		std::cout << "Failed to open image: " << _fullpath(nullptr, imageP, _MAX_PATH) << std::endl;
//		return 0;
//	}
//	// Allocating memory for headerData
//	headerData = static_cast<char *>(malloc(54));
//	// Checking if memory allocation was successful
//	if (headerData == nullptr)
//	{
//		std::cout << "Failed to allocate memory.\n";
//		fclose(image);
//		return 0;
//	}
	// Filling memory
//	memset(reinterpret_cast<wchar_t *>(headerData), 0, 54);
	// Reading header data
	headerData = {image.begin(), image.begin() + 54};
//	std::cout << "headerData = \"" << cryptLib::printableVector(headerData) << "\"\n";
//	fread(headerData, sizeof(unsigned char), 54, image);
	// Getting header data
//	imageFileSize = *(int *) &headerData[2];
	dataOffset = *(int *) &headerData[10];
	// Freeing memory
	headerData.clear();
//	free(headerData);
	
	// Getting full header & image data
	// Allocating memory for headerReturn & dataReturn
//	*headerReturn = static_cast<char *>(malloc(sizeof(char) * (dataOffset)));
//	*dataReturn = static_cast<char *>(malloc(sizeof(char) * (imageFileSize - dataOffset)));
	// Checking if memory allocation was successful
//	if ((*dataReturn == nullptr) || (*headerReturn == nullptr))
//	{
//		printf("Failed to allocate memory.\nTerminating program.\n");
//		fclose(image);
//		return 0;
//	}
	// Filling memory
//	memset(reinterpret_cast<wchar_t *>(*headerReturn), 0, sizeof(char) * (dataOffset));
//	memset(reinterpret_cast<wchar_t *>(*dataReturn), 0, sizeof(char) * (imageFileSize - dataOffset));
	// Reading image data
//	std::copy(storageSubstr.begin(), storageSubstr.end(), std::back_inserter(this->image));
	std::copy(image.begin(), image.begin() + dataOffset, std::back_inserter(headerReturn));
	std::copy(image.begin() + dataOffset, image.end(), std::back_inserter(dataReturn));
//	fseek(image, 0, SEEK_SET); // Start at the beginning of the file then leave the pointer there
//	fread(*headerReturn, sizeof(unsigned char), dataOffset, image);
//	fread(*dataReturn, sizeof(unsigned char), imageFileSize - dataOffset, image);
	// Closing image
//	fclose(image);
}

// Private
// Destructor (?)