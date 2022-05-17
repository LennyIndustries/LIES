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
connectionHandler::connectionHandler(std::vector <char> &function, const std::vector <char> &message, unsigned int uuid)
{
	// Passed values
	this->function = function;
	this->message = message;
	this->uuid = uuid;
	// Default values
	this->functionID = 0;
	this->imageLength = 0;
	
	messageSolver();
	
	if (functionID == 1)
	{
		// Encrypt
		encrypt myEncrypt = encrypt(this->image, this->text);
	}
	else if (functionID == 2)
	{
		// Decrypt
	}
	else if (functionID == -1)
	{
		std::cout << "A critical error occurred, aborting.\n";
		return;
	}
	else
	{
		std::cout << "ERROR: Failed to get FID\n";
		return;
		// Terminate
	}
}

// Public
// "Constructor"
connectionHandler *connectionHandler::create(std::vector <char> &function, std::vector <char> &message, unsigned int uuid)
{
	return new connectionHandler(function, message, uuid);
}

// "Destructor"
void connectionHandler::kill()
{
	delete this;
}

unsigned int connectionHandler::getUUID() const
{
	return uuid;
}

// Protected
// Private
void connectionHandler::messageSolver()
{
	// Steps through possible functions and returns a number for them
	std::vector <char> storage;
	std::vector <char> rest;
	std::size_t colonPosition = 0;
	std::size_t equalsPosition;
	
	// Find command & argument
	rest = this->message;
	while (colonPosition != std::string::npos)
	{
		colonPosition = cryptLib::vectorFind(rest, ':');
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
		if ((cryptLib::vectorCompare(messageCommand, "image")) && (equalsPosition != std::string::npos) && (imageLength > 0))
		{
			std::cout << "Command: Image\nImage length: " << this->imageLength << std::endl;
			std::vector <char> tempRestVector;
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
				std::cout << "CRITICAL ERROR : image length does not match vector length!\n" << this->imageLength << " != " << cryptLib::subVector(storage, equalsPosition + 1).size() << std::endl;
				this->functionID = -1;
				return; // Do something to stop the program from continuing.
			}
			std::vector <char> storageSubstr = cryptLib::subVector(storage, equalsPosition + 1, this->imageLength);
			std::copy(storageSubstr.begin(), storageSubstr.end(), std::back_inserter(this->image));
			
			tempRestVector.clear();
			tempRestVector = cryptLib::subVector(storage, (equalsPosition + 1 + this->imageLength));
			rest.clear();
			if (!tempRestVector.empty())
			{
				std::cout << "Rest not empty\n";
				std::copy(tempRestVector.begin(), tempRestVector.end(), std::back_inserter(rest));
			}
			else
			{
				std::cout << "Rest empty\n";
				return;
			}
		}
		else if ((cryptLib::vectorCompare(messageCommand, "text")) && (equalsPosition != std::string::npos))
		{
			std::cout << "Command: Text\n";
			std::size_t startOfTextPosition = cryptLib::vectorFind(message, char(STX));
			std::size_t endOfTextPosition = cryptLib::vectorFind(message, char(ETX));
			std::cout << "startOfTextPosition: " << startOfTextPosition << "\nendOfTextPosition: " << endOfTextPosition << std::endl;
			
			if ((startOfTextPosition != std::string::npos) && (endOfTextPosition != std::string::npos))
			{
				storage = cryptLib::subVector(message, startOfTextPosition + 1, endOfTextPosition - startOfTextPosition - 1);
				rest = cryptLib::subVector(message, endOfTextPosition + 2);
				std::copy(storage.begin(), storage.end(), std::back_inserter(this->text));
			}
			else
			{
				std::cout << "Failed to fetch message\n";
			}
		}
		else if ((cryptLib::vectorCompare(messageCommand, "encrypt")) && (this->functionID == 0))
		{
			std::cout << "Command: Encrypt\n";
			this->functionID = 1;
			std::cout << "Setting functionID = 1 (Encrypt)\n";
		}
		else if ((cryptLib::vectorCompare(messageCommand, "decrypt")) && (this->functionID == 0))
		{
			std::cout << "Command: Decrypt\n";
			this->functionID = 2;
			std::cout << "Setting functionID = 2 (Decrypt)\n";
		}
		else if ((cryptLib::vectorCompare(messageCommand, "imageLength")) && (this->imageLength == 0))
		{
			std::cout << "Command: Image length\n";
//			int tmpInt  = 0;
			std::vector <char> tmpVector;
			
			tmpVector.clear();
			tmpVector = cryptLib::subVector(storage, equalsPosition + 1);
			
//			for (int i = 0; i < tmpVector.size(); i++)
//			{
//				tmpInt += (tmpVector[i] - '0') * (pow(10, (tmpVector.size() - (i + 1))));
//			}
//			this->imageLength = tmpInt;
			this->imageLength = std::stoi(cryptLib::printableVector(tmpVector));
			std::cout << "Image length: " << this->imageLength << std::endl;
		}
		else
		{
			std::cout << "ERROR: Command not found, not complete or already set: " << cryptLib::printableVector(this->messageCommand) << std::endl;
		}
	}
}

// Destructor (Private)
connectionHandler::~connectionHandler() = default;
