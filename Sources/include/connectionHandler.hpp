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
	static connectionHandler *create(std::vector <char> &function, std::vector <char> &message, lilog *log, zmq::socket_t *vent, std::string key);
	void kill();
	// Getters / Setters
	[[nodiscard]] Botan::UUID getUUID() const;
protected:
private:
	// Con- Destructor
	connectionHandler(std::vector <char> &function, const std::vector <char> &message, lilog *log, zmq::socket_t *vent, std::string key);
	~connectionHandler();
	// Functions
	void handle();
	bool messageSolver();
	
	bool handleTextLength(std::vector <char> &storage, size_t &equalsPosition);
	bool handleText(std::vector <char> &storage, std::vector <char> &rest, size_t &equalsPosition);
	
	bool handleImageLength(std::vector <char> &storage, size_t &equalsPosition);
	bool handleImage(std::vector <char> &storage, std::vector <char> &rest, size_t &equalsPosition);
	
	bool handleKeyLength(std::vector <char> &storage, size_t &equalsPosition);
	bool handleKey(std::vector <char> &storage, std::vector <char> &rest, size_t &equalsPosition);
	
	bool handlePasswordLength(std::vector <char> &storage, size_t &equalsPosition);
	bool handlePassword(std::vector <char> &storage, std::vector <char> &rest, size_t &equalsPosition);
	
	bool handleUuid(std::vector <char> &storage, size_t &equalsPosition);
	
	bool decryptKey();
	void decryptData();
	
	void encryptCall();
	void decryptCall();
	// Variables
	lilog *myLog;
	zmq::socket_t *myVent;
	std::string myKeyString;
	
	std::vector <char> function;
	std::vector <char> message;
	
	std::vector <char> messageCommand;
	std::vector <char> messageArgument;
	
	int textLength, imageLength, keyLength, passwdLength;
	std::vector <char> text, image, passwd;
	std::vector <unsigned char> key;
	
	Botan::UUID uuid;
	
	bool error;
//	bool encryptSetting;
	char options;
};


#endif //LIES_CONNECTIONHANDLER_HPP
