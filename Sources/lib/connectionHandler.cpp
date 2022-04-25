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
	this->function = function;
	this->message = message;
	this->uuid = uuid;
	
	this->functionID = functionSolver();
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
char connectionHandler::functionSolver()
{
	// Steps through possible functions and returns a number for them
	return 0;
}

// Destructor (Private)
connectionHandler::~connectionHandler()
= default;
