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
	static connectionHandler *create(std::vector <uint8_t> &function, std::vector <uint8_t> &message, lilog *log, zmq::socket_t *vent, std::string key, zmq::socket_t *rec);
	void kill();
	// Getters / Setters
	[[nodiscard]] Botan::UUID getUUID() const;
protected:
private:
	// Con- Destructor
	connectionHandler(std::vector <uint8_t> &function, const std::vector <uint8_t> &message, lilog *log, zmq::socket_t *vent, std::string key, zmq::socket_t *rec);
	~connectionHandler();
	// Functions
	void handle();
	bool messageHandler();
	
	void sendRequest(const std::string& request, std::vector <uint8_t> & putItHere);
	
	bool decryptKey();
	void decryptData();
	
	void encryptCall();
	void decryptCall();
	// Variables
	lilog *myLog;
	
	zmq::socket_t *myVent;
	zmq::socket_t *myRec;
	
	std::string myKeyString;
	
	std::vector <uint8_t> function, message, myPrefix, clientPrefix;
	std::vector <uint8_t> messageCommand, messageArgument;
	std::vector <uint8_t> text, image, passwd, key;
	
	Botan::UUID uuid;
	
	bool error;
	
	char options;
};


#endif //LIES_CONNECTIONHANDLER_HPP
