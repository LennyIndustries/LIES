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
connectionHandler::connectionHandler(const std::string &function, const std::string &message, unsigned int uuid)
{
	// Passed values
	this->function = function;
	this->message = message;
	this->uuid = uuid;
	// Default values
	this->functionID = 0;
	this->text = nullptr;
	this->image = nullptr;
	
	std::cout << "Solving message: " << this->message << std::endl;
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
connectionHandler *connectionHandler::create(const std::string &function, const std::string &message, unsigned int uuid)
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
	std::string storage;
	std::string rest;
	std::size_t colonPosition = 0;
	std::size_t equalsPosition;
	std::size_t startOfTextPosition;
	std::size_t endOfTextPosition;
	
	// Find command & argument
	rest = message;
	while (colonPosition != std::string::npos)
	{
		colonPosition = rest.find(':');
		std::cout << "Found ':' at: " << colonPosition << std::endl;
		
		if (colonPosition != std::string::npos)
		{
			storage = rest.substr(0, colonPosition);
			std::cout << "Storage: " << storage << std::endl;
			rest = rest.substr(colonPosition);
			std::cout << "Rest: " << rest << std::endl;
			rest.erase(0, 1);
			std::cout << "Rest after erase: " << rest << std::endl;
		}
		else
		{
			storage = rest;
		}
		
		// Set command & argument
		equalsPosition = storage.find('=');
		std::cout << "Found '=' at: " << equalsPosition << std::endl;
		if (equalsPosition != std::string::npos)
		{
			this->messageCommand = storage.substr(0, equalsPosition);
			std::cout << "Setting messageCommand = \"" << this->messageCommand << "\" from substring\n";
		}
		else
		{
			this->messageCommand = storage;
			std::cout << "Setting messageCommand = \"" << this->messageCommand << "\" from storage\n";
		}
		
		// Filter command & argument
		if ((messageCommand == "image") && (equalsPosition != std::string::npos))
		{
			this->image = (char *) malloc(storage.substr(equalsPosition + 1).size());
			std::strcpy(this->image, storage.substr(equalsPosition + 1).c_str());
			std::cout << "Setting image = \"" << this->image << "\"\n";
		}
		else if ((messageCommand == "text") && (equalsPosition != std::string::npos))
		{
			startOfTextPosition = message.find(STX);
			endOfTextPosition = message.find(ETX);
			std::cout << "startOfTextPosition: " << startOfTextPosition << "\nendOfTextPosition: " << endOfTextPosition << std::endl;
			
			if ((startOfTextPosition != std::string::npos) && (endOfTextPosition != std::string::npos))
			{
				storage = message.substr(startOfTextPosition + 1, endOfTextPosition - startOfTextPosition - 1);
				std::cout << "Storage (text exception): " << storage << std::endl;
				rest = message.substr(endOfTextPosition + 2);
				std::cout << "Rest (text exception): " << rest << std::endl;
				this->text = (char *) malloc(storage.size());
				std::strcpy(this->text, storage.c_str());
				std::cout << "Setting text = \"" << this->text << "\"\n";
			}
			else
			{
				std::cout << "Failed to fetch message\n";
			}
		}
		else if ((messageCommand == "encrypt") && (this->functionID == 0))
		{
			this->functionID = 1;
			std::cout << "Setting functionID = 1 (Encrypt)\n";
		}
		else if ((messageCommand == "decrypt") && (this->functionID == 0))
		{
			this->functionID = 2;
			std::cout << "Setting functionID = 2 (Decrypt)\n";
		}
		else
		{
			std::cout << "ERROR: Command not found, not complete or already set: " << messageCommand << std::endl;
		}
	}
}

// Destructor (Private)
connectionHandler::~connectionHandler()
{
	free(this->image);
	free(this->text);
}
