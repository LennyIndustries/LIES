/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/cryptLib.hpp"

// Constructor (?)
// Public
// Protected
char cryptLib::getImageData(char *imageP, char **headerReturn, char **dataReturn)
{
	// File
	FILE *image = nullptr;
	// Image data
	char *headerData = nullptr;
	int imageFileSize = 0;
	int dataOffset = 0;
	// Return data
	*headerReturn = nullptr;
	*dataReturn = nullptr;
	
	// Getting data from image
	// Opening image
	image = fopen(_fullpath(nullptr, imageP, _MAX_PATH), "rb");
	// Checking if image is opened
	if (image == nullptr)
	{
		std::cout << "Failed to open image: " << _fullpath(nullptr, imageP, _MAX_PATH) << std::endl;
		return 0;
	}
	// Allocating memory for headerData
	headerData = static_cast<char *>(malloc(54));
	// Checking if memory allocation was successful
	if (headerData == nullptr)
	{
		std::cout << "Failed to allocate memory.\n";
		fclose(image);
		return 0;
	}
	// Filling memory
	memset(reinterpret_cast<wchar_t *>(headerData), 0, 54);
	// Reading header data
	fread(headerData, sizeof(unsigned char), 54, image);
	// Getting header data
	imageFileSize = *(int *) &headerData[2];
	dataOffset = *(int *) &headerData[10];
	// Freeing memory
	free(headerData);
	
	// Getting full header & image data
	// Allocating memory for headerReturn & dataReturn
	*headerReturn = static_cast<char *>(malloc(sizeof(char) * (dataOffset)));
	*dataReturn = static_cast<char *>(malloc(sizeof(char) * (imageFileSize - dataOffset)));
	// Checking if memory allocation was successful
	if ((*dataReturn == nullptr) || (*headerReturn == nullptr))
	{
		printf("Failed to allocate memory.\nTerminating program.\n");
		fclose(image);
		return 0;
	}
	// Filling memory
	memset(reinterpret_cast<wchar_t *>(*headerReturn), 0, sizeof(char) * (dataOffset));
	memset(reinterpret_cast<wchar_t *>(*dataReturn), 0, sizeof(char) * (imageFileSize - dataOffset));
	// Reading image data
	fseek(image, 0, SEEK_SET); // Start at the beginning of the file then leave the pointer there
	fread(*headerReturn, sizeof(unsigned char), dataOffset, image);
	fread(*dataReturn, sizeof(unsigned char), imageFileSize - dataOffset, image);
	// Closing image
	fclose(image);
	
	return 1;
}

// Private
// Destructor (?)