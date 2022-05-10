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
#include "encrypt.hpp"
#include "decrypt.hpp"

#include <cstring>
#include <iostream>
//#include <cstdlib>
#include <vector>
#include <iterator>
#include <cmath>

// Definitions
#define STX 2 // Start of Text
#define ETX 3 // End of Text

class connectionHandler
{
public:
	// Con- Destructor
	static connectionHandler *create(std::vector <char> &function, std::vector <char> &message, unsigned int uuid);
	void kill();
	// Getters / Setters
	[[nodiscard]] unsigned int getUUID() const;
protected:
private:
	// Con- Destructor
	connectionHandler(std::vector <char> &function, const std::vector <char> &message, unsigned int uuid);
	~connectionHandler();
	// Functions
	void messageSolver();
	// Variables
	std::vector <char> function;
	std::vector <char> message;
	unsigned int uuid; // Unused, meant for multithreading
	
	char functionID;
	std::vector <char> messageCommand;
	std::vector <char> messageArgument;
	
	std::vector <char> text;
	std::vector <char> image;
	int imageLength;
//	char *text;
//	char *image;
	
	std::string output;
};


#endif //LIES_CONNECTIONHANDLER_HPP
