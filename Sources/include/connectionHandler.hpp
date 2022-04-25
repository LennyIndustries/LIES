/*
 * Created by Leander @ Lenny Industries on 19/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * Handles new incoming connections
 */

#ifndef LIES_CONNECTIONHANDLER_HPP
#define LIES_CONNECTIONHANDLER_HPP

// Libraries
#include <string>

// Definitions

class connectionHandler
{
public:
	// Con- Destructor
	static connectionHandler *create(const std::string &function, const std::string &message, unsigned int uuid);
	void kill();
	// Getters / Setters
	[[nodiscard]] unsigned int getUUID() const;
protected:
private:
	// Con- Destructor
	connectionHandler(const std::string &function, const std::string &message, unsigned int uuid);
	~connectionHandler();
	// Functions
	char functionSolver ();
	// Variables
	std::string function;
	std::string message;
	unsigned int uuid; // Unused, meant for multithreading
	
	char functionID;
};


#endif //LIES_CONNECTIONHANDLER_HPP
