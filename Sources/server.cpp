/*
 * Created by Leander @ Lenny Industries on 18/03/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * LIES: Lenny Industries Encryption Service
 * Can encrypt and decrypt text into bmp images over benternet.
 * The server handles all requests from clients.
 */

/*
 * Bulk of program, get a message over benternet and solves the encryption / decryption.
 * Later add options to add extra encryption (RSA, ...).
 * Add checksum to info received over benternet.
 * Multithreading server, process multiple clients at once.
 * Possible other stuff.
 */

// Libraries
#include "include/lilog.hpp"
#include "include/inputHandler.hpp"
#include "include/connectionHandler.hpp"

#include <cstdio>
#include <openssl/opensslv.h>
#include <botan/botan.h>
#include <QtCore/qglobal.h>
#include <zmq.hpp>
#include <botan/uuid.h>

// Definitions
#define LOCALHOST "tcp://localhost:24042"
#define LOCAL "tcp://192.168.1.8:24042"
#define INTERNET "tcp://benternet.pxl-ea-ict.be:24042"
#define MSG_PREFIX "LennyIndustries|LIES|"
#define PASSWD "LennyIndustriesAdmin" // Safety first ;p

// Error codes
#define ERR_R_EXIT -1 // Remote exit request
#define ERR_0 0 // No error
#define ERR_1 1 // Invalid network option

int main(int argc, char **argv)
{
	// Input Arguments
	inputHandler myInputHandler(argc, argv);
	
	std::string logName = myInputHandler.getCmdOption("-log");
	if (logName.empty()) // Default log name
	{
		logName = "LIES.log";
	}
	
	// Starting log
	auto *myLog = lilog::create(logName, true);
	LOG(myLog, 1, "Starting program");
	
	// Connecting benternet
	try
	{
		zmq::context_t context(1);
		
		//Incoming messages come in here
		zmq::socket_t subscriber(context, ZMQ_SUB);
		if (myInputHandler.cmdOptionExists("-localhost"))
		{
			std::cout << "Connecting to: " << LOCALHOST << std::endl;
			subscriber.connect(LOCALHOST);
		}
		else if (myInputHandler.cmdOptionExists("-local"))
		{
			std::cout << "Connecting to: " << LOCAL << std::endl;
			subscriber.connect(LOCAL);
		}
		else if (myInputHandler.cmdOptionExists("-internet"))
		{
			std::cout << "Connecting to: " << INTERNET << std::endl;
			subscriber.connect(INTERNET);
		}
		else
		{
			LOG(myLog, 3, "Invalid network option");
			std::cout << "No valid network option given.\n";
			std::cout << "Valid options:\n'-localhost': uses localhost address, you need to host the broker yourself\n";
			std::cout << "'-local': uses the local ip of the broker, you need to be on the same network\n'-internet': connects over the internet";
			return ERR_1;
		}
		
		subscriber.setsockopt(ZMQ_SUBSCRIBE, MSG_PREFIX, strlen(MSG_PREFIX)); // LennyIndustries|ProjectName|Function|Message
		
		auto *msg = new zmq::message_t();
		std::string msgStr, function, message, subMsgStr;
		std::size_t pos;
		while (subscriber.handle() != nullptr)
		{
			subscriber.recv(msg);
			msgStr = std::string(static_cast<char *>(msg->data()), msg->size());
			
			subMsgStr = msgStr.substr(strlen(MSG_PREFIX));
//			std::cout << subMsgStr << std::endl;
			pos = subMsgStr.find('|');
//			std::cout << pos << std::endl;
			
			function = subMsgStr.substr(0, pos);
			message = subMsgStr.substr(pos + 1);
			
			char str_f[function.length() + 1];
			char str_m[message.length() + 1];
			std::vector<char> functionVector(function.begin(), function.end());
			std::vector<char> messageVector(message.begin(), message.end());
			std::strcpy(str_f, function.c_str());
			std::strcpy(str_m, message.c_str());
			
//			LOG(myLog, 1, "Received message; function: %s; message: %s.", str_f, str_m);
			
//			std::cout << "Received message: " << msgStr << "\nFunction: " << function << "\nMessage: " << message << std::endl;
			
			if (function != "exit")
			{
				auto *myConnectionHandler = connectionHandler::create(functionVector, messageVector, 0);
			}
			else
			{
				if (message == PASSWD)
				{
					return ERR_R_EXIT;
				}
			}
		}
	}
	catch (zmq::error_t &ex)
	{
		std::cerr << "Caught an exception : " << ex.what();
	}
	
	return ERR_0;
}