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
connectionHandler::connectionHandler(std::vector <char> &function, const std::vector <char> &message, lilog *log, zmq::socket_t *vent)
{
	// Passed values
	this->function = function;
	this->message = message;
	this->myLog = log;
	this->myVent = vent;
	// Default values
	this->error = false;
	this->imageLength = 0;
	this->uuid = 0;
	
	LOG(myLog, 1, "Calling messageSolver");
	messageSolver();
	
	if (this->text.empty() || this->image.empty())
	{
		LOG(myLog, 2, "Failed to set text or image");
		cryptLib::colorPrint("Failed to set text or image", ERRORCLR);
		error = true;
	}
	else if ((this->image[0] != 'B') || (this->image[1] != 'M'))
	{
		LOG(myLog, 2, "BMP file error");
		cryptLib::colorPrint("BMP file error", ERRORCLR);
		error = true;
	}
	else if (this->uuid == 0)
	{
		LOG(myLog, 2, "UUID not set");
		cryptLib::colorPrint("UUID not set", ERRORCLR);
		error = true;
	}
	
	if (this->error)
	{
		LOG(myLog, 3, "A critical error occurred, aborting");
		cryptLib::colorPrint("A critical error occurred, aborting", ERRORCLR);
		this->kill();
	}
	else if (cryptLib::printableVector(this->function) == "encrypt")
	{
		// Encrypt
		encrypt myEncrypt = encrypt(this->image, this->text, myLog); // Send image and text to be encrypted
		this->image.clear(); // Clear current image
		this->image = myEncrypt.getImage(); // Get encrypted image
		// Prep for sending
		std::vector <char> sendVector;
		std::string msgPrefix = "LennyIndustries|LIES|" + std::to_string(this->uuid) + '|';
		std::copy(msgPrefix.begin(), msgPrefix.end(), std::back_inserter(sendVector)); // Copy prefix
		std::copy(this->image.begin(), this->image.end(), std::back_inserter(sendVector)); // Copy image
		LOG(myLog, 1, "Sending image back");
		cryptLib::colorPrint("Sending image back", ALTMSGCLR);
		this->myVent->send(sendVector.data(), sendVector.size());
		this->kill();
	}
	else if (cryptLib::printableVector(this->function) == "decrypt")
	{
		// Decrypt
		this->text.clear();
		// Get text
		this->kill();
	}
	else
	{
		LOG(myLog, 3, "ERROR: Unknown function: %s", cryptLib::printableVector(function).c_str());
		cryptLib::colorPrint("ERROR: Unknown function: " + cryptLib::printableVector(function), ERRORCLR);
		this->kill();
	}
}

// Public
// "Constructor"
connectionHandler *connectionHandler::create(std::vector <char> &function, std::vector <char> &message, lilog *log, zmq::socket_t *vent)
{
	return new connectionHandler(function, message, log, vent);
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
		if ((cryptLib::vectorCompare(this->messageCommand, "image")) && (equalsPosition != std::string::npos) && (this->imageLength > 0))
		{
			cryptLib::colorPrint("Command: Image\nImage length: " + std::to_string(this->imageLength), MSGCLR);
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
				LOG(myLog, 2, "CRITICAL ERROR : image length does not match vector length");
				cryptLib::colorPrint("CRITICAL ERROR : image length does not match vector length", ERRORCLR);
				this->error = true;
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
		else if ((cryptLib::vectorCompare(this->messageCommand, "text")) && (equalsPosition != std::string::npos))
		{
			cryptLib::colorPrint("Command: Text", MSGCLR);
			std::size_t startOfTextPosition = cryptLib::vectorFind(this->message, STX);
			std::size_t endOfTextPosition = cryptLib::vectorFind(this->message, ETX);
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
				cryptLib::colorPrint("Failed to fetch message", ERRORCLR);
			}
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "imageLength")) && (this->imageLength == 0))
		{
			cryptLib::colorPrint("Command: Image length", MSGCLR);
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
		else if (cryptLib::vectorCompare(this->messageCommand, "uuid"))
		{
			cryptLib::colorPrint("Command: UUID", MSGCLR);
			
			std::vector <char> tmpVector;
			
			tmpVector.clear();
			tmpVector = cryptLib::subVector(storage, equalsPosition + 1);
			
			this->uuid = std::stoi(cryptLib::printableVector(tmpVector));
			std::cout << "UUID: " << this->uuid << std::endl;
		}
		else
		{
			LOG(myLog, 2, "ERROR: Command not found, not complete or already set: %s", cryptLib::printableVector(this->messageCommand).c_str());
			cryptLib::colorPrint("ERROR: Command not found, not complete or already set: " + cryptLib::printableVector(this->messageCommand), ERRORCLR);
		}
	}
}

// Destructor (Private)
connectionHandler::~connectionHandler() = default;
