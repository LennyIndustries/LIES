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
	
	LOG(this->myLog, 1, "Calling handle");
	handle();
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
void connectionHandler::handle()
{
	LOG(this->myLog, 1, "Calling messageSolver");
	if (!messageSolver())
	{
		LOG(this->myLog, 2, "Message solver failed");
		cryptLib::colorPrint("Message solver failed", ERRORCLR);
		this->error = true;
	}
	if (cryptLib::vectorCompare(this->function, "encrypt"))
	{
		if (this->text.empty() || this->image.empty())
		{
			LOG(this->myLog, 2, "Failed to set text or image");
			cryptLib::colorPrint("Failed to set text or image", ERRORCLR);
			this->error = true;
		}
	}
	if (cryptLib::vectorCompare(this->function, "decrypt"))
	{
		if (this->image.empty())
		{
			LOG(this->myLog, 2, "Failed to set text");
			cryptLib::colorPrint("Failed to set text", ERRORCLR);
			this->error = true;
		}
	}
	if ((this->image[0] != 'B') || (this->image[1] != 'M'))
	{
		LOG(this->myLog, 2, "BMP file error");
		cryptLib::colorPrint("BMP file error", ERRORCLR);
		this->error = true;
	}
	if (this->uuid == 0)
	{
		LOG(this->myLog, 2, "UUID not set");
		cryptLib::colorPrint("UUID not set", ERRORCLR);
		this->error = true;
	}
	
	if (this->error)
	{
		LOG(this->myLog, 3, "A critical error occurred, aborting");
		cryptLib::colorPrint("A critical error occurred, aborting", ERRORCLR);
		this->kill();
	}
	else if (cryptLib::vectorCompare(this->function, "encrypt"))
	{
		// Encrypt
		encryptCall();
		this->kill();
	}
	else if (cryptLib::vectorCompare(this->function, "decrypt"))
	{
		// Decrypt
		decryptCall();
		this->kill();
	}
	else
	{
		LOG(this->myLog, 3, "ERROR: Unknown function: %s", cryptLib::printableVector(function).c_str());
		cryptLib::colorPrint("ERROR: Unknown function: " + cryptLib::printableVector(function), ERRORCLR);
		this->kill();
	}
}

bool connectionHandler::messageSolver()
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
			if (!handleImage(storage, rest, equalsPosition))
				return false;
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "text")) && (equalsPosition != std::string::npos))
		{
			cryptLib::colorPrint("Command: Text", MSGCLR);
			if (!handleText(storage, rest))
				return false;
			
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "imageLength")) && (this->imageLength == 0))
		{
			cryptLib::colorPrint("Command: Image length", MSGCLR);
			if (!handleImageLength(storage, equalsPosition))
				return false;
		}
		else if (cryptLib::vectorCompare(this->messageCommand, "uuid"))
		{
			cryptLib::colorPrint("Command: UUID", MSGCLR);
			if (!handleUuid(storage, equalsPosition))
				return false;
		}
		else
		{
			LOG(this->myLog, 2, "ERROR: Command not found, not complete or already set: %s", cryptLib::printableVector(this->messageCommand).c_str());
			cryptLib::colorPrint("ERROR: Command not found, not complete or already set: " + cryptLib::printableVector(this->messageCommand), ERRORCLR);
			return false;
		}
	}
	return true;
}

bool connectionHandler::handleImage(std::vector <char> &storage, std::vector <char> &rest, size_t &equalsPosition)
{
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
		LOG(this->myLog, 2, "CRITICAL ERROR : image length does not match vector length");
		cryptLib::colorPrint("CRITICAL ERROR : image length does not match vector length", ERRORCLR);
		this->error = true;
		return false; // Do something to stop the program from continuing.
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
		return false;
	}
	return true;
}

bool connectionHandler::handleText(std::vector <char> &storage, std::vector <char> &rest)
{
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
		LOG(this->myLog, 2, "Failed to fetch message");
		cryptLib::colorPrint("Failed to fetch message", ERRORCLR);
		return false;
	}
	return true;
}

bool connectionHandler::handleImageLength(std::vector <char> &storage, size_t &equalsPosition)
{
	std::vector <char> tmpVector;
	
	tmpVector.clear();
	tmpVector = cryptLib::subVector(storage, equalsPosition + 1);
	
	try
	{
		this->imageLength = std::stoi(cryptLib::printableVector(tmpVector));
	}
	catch (const std::exception &e)
	{
		LOG(this->myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
		return false;
	}
	std::cout << "Image length: " << this->imageLength << std::endl;
	return true;
}

bool connectionHandler::handleUuid(std::vector <char> &storage, size_t &equalsPosition)
{
	std::vector <char> tmpVector;
	
	tmpVector.clear();
	tmpVector = cryptLib::subVector(storage, equalsPosition + 1);
	
	try
	{
		this->uuid = std::stoi(cryptLib::printableVector(tmpVector));
	}
	catch (const std::exception &e)
	{
		LOG(this->myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
		return false;
	}
	std::cout << "UUID: " << this->uuid << std::endl;
	return true;
}

void connectionHandler::encryptCall()
{
	encrypt myEncrypt = encrypt(this->image, this->text, this->myLog); // Send image and text to be encrypted
	this->image.clear(); // Clear current image
	this->image = myEncrypt.getImage(); // Get encrypted image
	// Prep for sending
	std::vector <char> sendVector;
	std::string msgPrefix = "LennyIndustries|LIES|" + std::to_string(this->uuid) + '|';
	std::copy(msgPrefix.begin(), msgPrefix.end(), std::back_inserter(sendVector)); // Copy prefix
	std::copy(this->image.begin(), this->image.end(), std::back_inserter(sendVector)); // Copy image
	LOG(this->myLog, 1, "Sending image back");
	cryptLib::colorPrint("Sending image back", ALTMSGCLR);
	this->myVent->send(sendVector.data(), sendVector.size());
}

void connectionHandler::decryptCall()
{
	decrypt myDecrypt = decrypt(this->image, this->myLog);
	this->text.clear(); // Clear current text
	this->text = myDecrypt.getText(); // Get decrypted text
	// Prep for sending
	std::vector <char> sendVector;
	std::string msgPrefix = "LennyIndustries|LIES|" + std::to_string(this->uuid) + '|';
	std::copy(msgPrefix.begin(), msgPrefix.end(), std::back_inserter(sendVector)); // Copy prefix
	sendVector.push_back(STX); // Start tag
	std::copy(this->text.begin(), this->text.end(), std::back_inserter(sendVector)); // Copy text
	sendVector.push_back(ETX); // End tag
	LOG(this->myLog, 1, "Sending text back");
	cryptLib::colorPrint("Sending text back", ALTMSGCLR);
	this->myVent->send(sendVector.data(), sendVector.size());
}

// Destructor (Private)
connectionHandler::~connectionHandler() = default;
