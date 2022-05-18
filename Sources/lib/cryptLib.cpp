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

std::size_t cryptLib::vectorFind(const std::vector <char> &searchVector, char searchChar)
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

std::vector <char> cryptLib::subVector(const std::vector <char> &startVector, std::size_t startPos, std::size_t charCount)
{
	std::vector <char> returnVector;
	if (startPos >= std::string::npos)
	{
		return returnVector;
	}
	
	if (charCount == std::string::npos)
	{
		for (std::size_t i = startPos; i < startVector.size(); i++)
		{
			returnVector.push_back(startVector[i]);
		}
		return returnVector;
	}
	
	for (std::size_t i = startPos; i < startPos + charCount; i++)
	{
		returnVector.push_back(startVector[i]);
	}
	
	return returnVector;
}

bool cryptLib::vectorCompare(const std::vector <char> &vector, const std::string &string)
{
	std::vector <char> tempVector(string.begin(), string.end());
	return (vector == tempVector);
}

void cryptLib::colorPrint(const std::string &message, char color)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
	std::cout << message << std::endl;
	SetConsoleTextAttribute(hConsole, DEFAULTCLR);
}

// Protected
void cryptLib::getImageData(std::vector <char> &image, std::vector <char> &headerReturn, std::vector <char> &dataReturn)
{
	std::vector <char> headerData;
	int dataOffset = 0;
	
	headerData = {image.begin(), image.begin() + 54};
	dataOffset = *(int *) &headerData[10];
	headerData.clear();
	
	std::copy(image.begin(), image.begin() + dataOffset, std::back_inserter(headerReturn));
	std::copy(image.begin() + dataOffset, image.end(), std::back_inserter(dataReturn));
}

// Private
// Destructor (?)