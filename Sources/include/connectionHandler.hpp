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
#include "include/lilog.hpp"

#include <cstring>
#include <iostream>
//#include <cstdlib>
#include <vector>
#include <iterator>
#include <cmath>
#include <zmq.hpp>

// Definitions
#define STX 0x2 // STX (Start Of Text) :: 0000 0010 :: 2
#define ETX 0x3 // ETX (End Of Text) :: 0000 0011 :: 3

class connectionHandler
{
public:
	// Con- Destructor
	static connectionHandler *create(std::vector <char> &function, std::vector <char> &message, lilog *log, zmq::socket_t *vent);
	void kill();
	// Getters / Setters
	[[nodiscard]] unsigned int getUUID() const;
protected:
private:
	// Con- Destructor
	connectionHandler(std::vector <char> &function, const std::vector <char> &message, lilog *log, zmq::socket_t *vent);
	~connectionHandler();
	// Functions
	void messageSolver();
	// Variables
	lilog *myLog;
	zmq::socket_t *myVent;
	
	std::vector <char> function;
	std::vector <char> message;
	unsigned int uuid; // Unused, meant for multithreading

//	char functionID;
	std::vector <char> messageCommand;
	std::vector <char> messageArgument;
	
	std::vector <char> text;
	std::vector <char> image;
	int imageLength;
//	char *text;
//	char *image;
	
	bool error;

//	std::string output;
};


#endif //LIES_CONNECTIONHANDLER_HPP
