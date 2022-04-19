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

// Constructor (Public)
connectionHandler::connectionHandler(const std::string& function, const std::string& message, unsigned int uuid)
{
	this->uuid = uuid;
}

// Public
unsigned int connectionHandler::getUUID() const
{
	return uuid;
}

// Protected
// Private

// Destructor (?)