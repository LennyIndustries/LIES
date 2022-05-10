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
//	this->text = nullptr;
//	this->image = nullptr;
	
//	std::cout << "Solving message: " << cryptLib::printableVector(this->message) << std::endl;
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
	else
	{
		std::cout << "ERROR: Failed to get FID\n";
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
		colonPosition = cryptLib::vectorFind(rest, ':'); //rest.find(':');
		std::cout << "Found ':' at: " << colonPosition << std::endl;
		
		if (colonPosition != std::string::npos)
		{
			storage = cryptLib::subVector(rest, 0, colonPosition); //rest.substr(0, colonPosition);
//			std::cout << "Storage: " << cryptLib::printableVector(storage) << std::endl;
			rest = cryptLib::subVector(rest, colonPosition + 1); //rest.substr(colonPosition + 1);
//			std::cout << "Rest: " << cryptLib::printableVector(rest) << std::endl;
//			rest.erase(0, 1);
//			std::cout << "Rest after erase: " << rest << std::endl;
		}
		else
		{
			storage = rest;
		}
		
		// Set command & argument
		equalsPosition = cryptLib::vectorFind(storage, '='); //storage.find('=');
		std::cout << "Found '=' at: " << equalsPosition << std::endl;
		if (equalsPosition != std::string::npos)
		{
			this->messageCommand = cryptLib::subVector(storage, 0, equalsPosition); //storage.substr(0, equalsPosition);
//			std::cout << "Setting messageCommand = \"" << cryptLib::printableVector(this->messageCommand) << "\" from substring\n";
		}
		else
		{
			this->messageCommand = storage;
//			std::cout << "Setting messageCommand = \"" << cryptLib::printableVector(this->messageCommand) << "\" from storage\n";
		}
		
		// Filter command & argument
		if ((cryptLib::vectorCompare(messageCommand, "image")) && (equalsPosition != std::string::npos) && (imageLength > 0)) // messageCommand == "image"
		{
			std::vector <char> storageSubstr = cryptLib::subVector(this->message, equalsPosition + 1, this->imageLength); //storage.substr(equalsPosition + 1);
//			std::cout << (equalsPosition + 1 + this->imageLength) << " < " << this->message.size() << "\n---\n";
//			if ((equalsPosition + 1 + this->imageLength) < this->message.size())
//			{
//				rest = cryptLib::subVector(this->message, (equalsPosition + 1 + this->imageLength));
//				std::cout << "Rest = " << rest.data() << "\nStorage = " << storage.data() << std::endl;
//			}
//			else
//			{
//				rest.clear();
//			}
			
			std::cout << "Rest = " << rest.data() << "\nStorage = " << storage.data() << "\n----------\n";
			
//			std::cout << "Setting storageSubstr = \"" << cryptLib::printableVector(storageSubstr) << "\" from storage\n";
			std::copy(storageSubstr.begin(), storageSubstr.end(), std::back_inserter(this->image));
//			std::cout << this->image.size() << std::endl;
//			std::cout << "Setting image = \"" << cryptLib::printableVector(this->image) << "\" : " << this->image.size() << std::endl;
		}
		else if ((cryptLib::vectorCompare(messageCommand, "text")) && (equalsPosition != std::string::npos)) // messageCommand == "text"
		{
			std::size_t startOfTextPosition = cryptLib::vectorFind(message, char(STX)); //message.find(STX);
			std::size_t endOfTextPosition = cryptLib::vectorFind(message, char(ETX)); //message.find(ETX);
//			std::cout << "startOfTextPosition: " << startOfTextPosition << "\nendOfTextPosition: " << endOfTextPosition << std::endl;
			
			if ((startOfTextPosition != std::string::npos) && (endOfTextPosition != std::string::npos))
			{
				storage = cryptLib::subVector(message, startOfTextPosition + 1, endOfTextPosition - startOfTextPosition - 1); //message.substr(startOfTextPosition + 1, endOfTextPosition - startOfTextPosition - 1);
//				std::cout << "Storage (text exception): " << cryptLib::printableVector(storage) << std::endl;
				rest = cryptLib::subVector(message, endOfTextPosition + 2); //message.substr(endOfTextPosition + 2);
//				std::cout << "Rest (text exception): " << cryptLib::printableVector(rest) << std::endl;
				
				std::copy(storage.begin(), storage.end(), std::back_inserter(this->text));
//				std::cout << "Setting text = \"" << cryptLib::printableVector(this->text) << "\" : " << this->text.size() << std::endl;
			}
			else
			{
				std::cout << "Failed to fetch message\n";
			}
		}
		else if ((cryptLib::vectorCompare(messageCommand, "encrypt")) && (this->functionID == 0)) //messageCommand == "encrypt"
		{
			this->functionID = 1;
//			std::cout << "Setting functionID = 1 (Encrypt)\n";
		}
		else if ((cryptLib::vectorCompare(messageCommand, "decrypt")) && (this->functionID == 0)) //messageCommand == "decrypt"
		{
			this->functionID = 2;
//			std::cout << "Setting functionID = 2 (Decrypt)\n";
		}
		else if ((cryptLib::vectorCompare(messageCommand, "imageLength")) && (this->imageLength == 0)) //messageCommand == "decrypt"
		{
			int tmpInt  = 0;
			std::vector <char> tmpVector;
			
			tmpVector.clear();
			tmpVector = cryptLib::subVector(storage, equalsPosition + 1);
			
			for (int i = 0; i < tmpVector.size(); i++)
			{
				tmpInt += (tmpVector[i] - '0') * (pow(10, (tmpVector.size() - (i + 1))));
			}
			this->imageLength = tmpInt;
			std::cout << "Image size = " << tmpInt << std::endl << tmpVector.size() << std::endl << tmpVector.data() << std::endl << storage.data() << "\n-----\n";
		}
		else
		{
			std::cout << "ERROR: Command not found, not complete or already set: " << cryptLib::printableVector(this->messageCommand) << std::endl;
		}
	}
}

// Destructor (Private)
connectionHandler::~connectionHandler() = default;
//{
//	free(this->image);
//	free(this->text);
//}
