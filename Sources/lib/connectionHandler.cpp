/*
 * Created by Leander @ Lenny Industries on 19/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * Find out what has been requested and solve the message
 * Keep thread alive until everything has been handled, then close connection and thread
 */

#include "include/connectionHandler.hpp"

// Constructor (Private)
connectionHandler::connectionHandler(std::vector<char> &function, const std::vector<char> &message, lilog *log)
{
	// Passed values
	this->myLog = log;
	this->function = function;
	this->message = message;
	// Default values
	this->error = false;
	this->imageLength = 0;
	this->uuid = 0;
	
	messageSolver();
	
	if (this->text.empty() || this->image.empty())
	{
		LOG(myLog, 2, "Failed to set text or image.");
		std::cout << "Failed to set text or image.\n";
		error = true;
	}
	else if ((this->image[0] != 'B') || (this->image[1] != 'M'))
	{
		LOG(myLog, 2, "BMP file error.");
		std::cout << "BMP file error.\n";
		error = true;
	}
	
	if (this->error)
	{
		LOG(myLog, 3, "A critical error occurred, aborting.");
		std::cout << "A critical error occurred, aborting.\n";
		return;
	}
	else if (cryptLib::printableVector(this->function) == "encrypt?")
	{
		// Encrypt
		encrypt myEncrypt = encrypt(this->image, this->text);
		this->image.clear();
		this->image = myEncrypt.getImage();
	}
	else if (cryptLib::printableVector(this->function) == "decrypt?")
	{
		// Decrypt
		this->text.clear();
		// Get text
	}
	else
	{
		LOG(myLog, 3, "ERROR: Unknown function: %s", cryptLib::printableVector(function).c_str());
		std::cout << "ERROR: Unknown function\n";
		return;
	}
	
	// Send text and image back
}

// Public
// "Constructor"
connectionHandler *connectionHandler::create(std::vector<char> &function, std::vector<char> &message, lilog *log)
{
	return new connectionHandler(function, message, log);
}

// "Destructor"
void connectionHandler::kill()
{
	delete this;
}

unsigned int connectionHandler::getUUID() const
{
	return this->uuid;
}

// Protected
// Private
void connectionHandler::messageSolver()
{
	// Steps through possible functions and returns a number for them
	std::vector<char> storage;
	std::vector<char> rest;
	std::size_t colonPosition = 0;
	std::size_t equalsPosition;
	
	// Find command & argument
	rest = this->message;
	while (colonPosition != std::string::npos)
	{
		colonPosition = cryptLib::vectorFind(rest, ':');
		LOG(myLog, 1, "Found ':' at: %i", colonPosition);
		std::cout << "Found ':' at: " << colonPosition << std::endl;
		
		if (colonPosition != std::string::npos)
		{
			storage = cryptLib::subVector(rest, 0, colonPosition);
			rest = cryptLib::subVector(rest, colonPosition + 1);
		}
		else
		{
			storage = rest;
			rest.clear();
		}
		
		// Set command & argument
		equalsPosition = cryptLib::vectorFind(storage, '=');
		LOG(myLog, 1, "Found '=' at: %i", equalsPosition);
		std::cout << "Found '=' at: " << equalsPosition << std::endl;
		if (equalsPosition != std::string::npos)
		{
			this->messageCommand = cryptLib::subVector(storage, 0, equalsPosition);
		}
		else
		{
			this->messageCommand = storage;
		}
		
		// Filter command & argument
		if ((cryptLib::vectorCompare(this->messageCommand, "image")) && (equalsPosition != std::string::npos) && (this->imageLength > 0))
		{
			LOG(myLog, 1, "Command: Image");
			LOG(myLog, 1, "Image length: %i", this->imageLength);
			std::cout << "Command: Image\nImage length: " << this->imageLength << std::endl;
			std::vector<char> tempRestVector;
			if (!rest.empty()) // If there is something left add it back to storage
			{
				tempRestVector.clear();
				tempRestVector = rest;
				rest.clear();
				rest.push_back('='); // Add the '=' back since it was removed
				std::copy(tempRestVector.begin(), tempRestVector.end(), std::back_inserter(rest));
				std::copy(rest.begin(), rest.end(), std::back_inserter(storage));
			}
			
			if (this->imageLength != cryptLib::subVector(storage, equalsPosition + 1).size())
			{
				LOG(myLog, 2, "CRITICAL ERROR : image length does not match vector length!");
				std::cout << "CRITICAL ERROR : image length does not match vector length!\n" << this->imageLength << " != " << cryptLib::subVector(storage, equalsPosition + 1).size() << std::endl;
				this->error = true;
				return; // Do something to stop the program from continuing.
			}
			
			std::vector<char> storageSubstr = cryptLib::subVector(storage, equalsPosition + 1, this->imageLength);
			std::copy(storageSubstr.begin(), storageSubstr.end(), std::back_inserter(this->image));
			
			tempRestVector.clear();
			tempRestVector = cryptLib::subVector(storage, (equalsPosition + 1 + this->imageLength));
			rest.clear();
			if (!tempRestVector.empty())
			{
				LOG(myLog, 1, "Rest not empty");
				std::cout << "Rest not empty\n";
				std::copy(tempRestVector.begin(), tempRestVector.end(), std::back_inserter(rest));
			}
			else
			{
				LOG(myLog, 1, "Rest empty");
				std::cout << "Rest empty\n";
				return;
			}
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "text")) && (equalsPosition != std::string::npos))
		{
			LOG(myLog, 1, "Command: Text");
			std::cout << "Command: Text\n";
			std::size_t startOfTextPosition = cryptLib::vectorFind(this->message, char(STX));
			std::size_t endOfTextPosition = cryptLib::vectorFind(this->message, char(ETX));
			LOG(myLog, 1, "startOfTextPosition: %i", startOfTextPosition);
			LOG(myLog, 1, "endOfTextPosition: %i", endOfTextPosition);
			std::cout << "startOfTextPosition: " << startOfTextPosition << "\nendOfTextPosition: " << endOfTextPosition << std::endl;
			
			if ((startOfTextPosition != std::string::npos) && (endOfTextPosition != std::string::npos))
			{
				storage = cryptLib::subVector(this->message, startOfTextPosition + 1, endOfTextPosition - startOfTextPosition - 1);
				rest = cryptLib::subVector(this->message, endOfTextPosition + 2);
				std::copy(storage.begin(), storage.end(), std::back_inserter(this->text));
			}
			else
			{
				LOG(myLog, 2, "Failed to fetch message");
				std::cout << "Failed to fetch message\n";
			}
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "imageLength")) && (this->imageLength == 0))
		{
			LOG(myLog, 1, "Command: Image length");
			std::cout << "Command: Image length\n";
//			int tmpInt  = 0;
			std::vector<char> tmpVector;
			
			tmpVector.clear();
			tmpVector = cryptLib::subVector(storage, equalsPosition + 1);

//			for (int i = 0; i < tmpVector.size(); i++)
//			{
//				tmpInt += (tmpVector[i] - '0') * (pow(10, (tmpVector.size() - (i + 1))));
//			}
//			this->imageLength = tmpInt;
			this->imageLength = std::stoi(cryptLib::printableVector(tmpVector));
			LOG(myLog, 1, "Image length: %i", this->imageLength);
			std::cout << "Image length: " << this->imageLength << std::endl;
		}
		else if (cryptLib::vectorCompare(this->messageCommand, "uuid"))
		{
			LOG(myLog, 1, "Command: UUID");
			std::cout << "Command: UUID\n";
			
			std::vector<char> tmpVector;
			
			tmpVector.clear();
			tmpVector = cryptLib::subVector(storage, equalsPosition + 1);
			
			this->uuid = std::stoi(cryptLib::printableVector(tmpVector));
			LOG(myLog, 1, "UUID: %i", this->uuid);
			std::cout << "UUID: " << this->uuid << std::endl;
		}
		else
		{
			LOG(myLog, 2, "ERROR: Command not found, not complete or already set: %s", cryptLib::printableVector(this->messageCommand).c_str());
			std::cout << "ERROR: Command not found, not complete or already set: " << cryptLib::printableVector(this->messageCommand) << std::endl;
		}
	}
}

// Destructor (Private)
connectionHandler::~connectionHandler() = default;
