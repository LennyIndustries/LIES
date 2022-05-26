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

class connectionHandler
{
public:
	// Con- Destructor
	static connectionHandler *create(std::vector <char> &function, std::vector <char> &message, lilog *log, zmq::socket_t *vent);
	void kill();
	// Getters / Setters
	[[nodiscard]] Botan::UUID getUUID() const;
protected:
private:
	// Con- Destructor
	connectionHandler(std::vector <char> &function, const std::vector <char> &message, lilog *log, zmq::socket_t *vent);
	~connectionHandler();
	// Functions
	void handle();
	bool messageSolver();
	bool handleTextLength(std::vector <char> &storage, size_t &equalsPosition);
	bool handleText(std::vector <char> &storage, std::vector <char> &rest, size_t &equalsPosition);
	
	bool handleImageLength(std::vector <char> &storage, size_t &equalsPosition);
	bool handleImage(std::vector <char> &storage, std::vector <char> &rest, size_t &equalsPosition);
	
	bool handleUuid(std::vector <char> &storage, size_t &equalsPosition);
	void handleKey(std::vector <char> &storage, size_t &equalsPosition);
	void handlePassword(std::vector <char> &storage, size_t &equalsPosition);
	
	void encryptCall();
	void decryptCall();
	// Variables
	lilog *myLog;
	zmq::socket_t *myVent;
	
	std::vector <char> function;
	std::vector <char> message;
	std::vector <char> key;
	std::string passwd;
	Botan::UUID uuid; // Unused, meant for multithreading

//	char functionID;
	std::vector <char> messageCommand;
	std::vector <char> messageArgument;
	
	std::vector <char> text;
	std::vector <char> image;
	int textLength;
	int imageLength;
//	char *text;
//	char *image;
	
	bool error;

//	std::string output;
};


#endif //LIES_CONNECTIONHANDLER_HPP
