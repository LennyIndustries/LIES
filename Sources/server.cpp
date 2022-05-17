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
#define LOCALHOST(port) "tcp://localhost:" port
#define LOCAL(port) "tcp://192.168.1.8:" port
#define INTERNET(port) "tcp://benternet.pxl-ea-ict.be:" port
#define MSG_PREFIX "LennyIndustries|LIES_Server|"
#define FILTER_CHAR '|'
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
		time_t now = time(nullptr); // https://www.tutorialspoint.com/cplusplus/cpp_date_time.htm
		tm *ltm = localtime(&now);
		std::string dateTime = "_" + std::to_string(1900 + ltm->tm_year) + "-" + std::to_string(ltm->tm_mon) + "-" + std::to_string(ltm->tm_wday);
		dateTime += "_" + std::to_string(ltm->tm_hour) + "-" + std::to_string(ltm->tm_min) + "-" + std::to_string(ltm->tm_sec);
		std::cout << "Log name: LIES" << dateTime << ".log\n";
		logName = "LIES" + dateTime + ".log";
	}
	else
	{
		std::cout << "Log name: " << logName << std::endl;
	}
	
	// Starting log
	auto *myLog = lilog::create(logName, true, true);
	LOG(myLog, 1, "Starting program");
	
	// Connecting benternet
	try
	{
		zmq::context_t context(1);
		
		//Incoming messages come in here
		zmq::socket_t subscriber(context, ZMQ_SUB);
		zmq::socket_t ventilator(context, ZMQ_PUSH);
		if (myInputHandler.cmdOptionExists("-localhost"))
		{
			LOG(myLog, 1, LOCALHOST("0"));
			std::cout << "Connecting to: " << LOCALHOST("0") << std::endl;
			subscriber.connect(LOCALHOST("24042"));
			ventilator.connect(LOCALHOST("24041"));
		}
		else if (myInputHandler.cmdOptionExists("-local"))
		{
			LOG(myLog, 1, LOCAL("0"));
			std::cout << "Connecting to: " << LOCAL("0") << std::endl;
			subscriber.connect(LOCAL("24042"));
			ventilator.connect(LOCAL("24041"));
		}
		else if (myInputHandler.cmdOptionExists("-internet"))
		{
			LOG(myLog, 1, INTERNET("0"));
			std::cout << "Connecting to: " << INTERNET("0") << std::endl;
			subscriber.connect(INTERNET("24042"));
			ventilator.connect(INTERNET("24041"));
		}
		else
		{
			LOG(myLog, 3, "Invalid network option");
			std::cout << "No valid network option given.\n";
			std::cout << "Valid options:\n'-localhost': uses localhost address, you need to host the broker yourself\n";
			std::cout << "'-local': uses the local ip of the broker, you need to be on the same network\n'-internet': connects over the internet";
			return ERR_1;
		}
		
		subscriber.set(zmq::sockopt::subscribe, MSG_PREFIX); // LennyIndustries|ProjectName|Function|Message
//		subscriber.setsockopt(ZMQ_SUBSCRIBE, MSG_PREFIX, strlen(MSG_PREFIX));
		
		auto *msg = new zmq::message_t();
		std::string msgStr, function, message, subMsgStr;
		std::size_t pos;
		unsigned int uuid;
		while (subscriber.handle() != nullptr)
		{
			subscriber.recv(msg);
			LOG(myLog, 1, "Incoming message");
			std::cout << "Incoming message\n";
			
			msgStr = std::string(static_cast<char *>(msg->data()), msg->size());
			
			subMsgStr = msgStr.substr(strlen(MSG_PREFIX));
			pos = subMsgStr.find(FILTER_CHAR);
			
			function = subMsgStr.substr(0, pos);
			message = subMsgStr.substr(pos + 1);
			
			std::vector<char> functionVector(function.begin(), function.end());
			std::vector<char> messageVector(message.begin(), message.end());
			
			LOG(myLog, 1, "Function: %s", function.c_str());
			std::cout << "Function: " << function << std::endl;
			LOG(myLog, 1, "Size of received message: %i", msg->size());
			std::cout << "Size of received message: " << msg->size() << std::endl;
			
			if ((function == "encrypt?") || (function == "decrypt?"))
			{
				auto *myConnectionHandler = connectionHandler::create(functionVector, messageVector, myLog);
			}
			else if (function == "key?")
			{
				// Send the public key
			}
			else if (function == "uuid?")
			{
				// Send UUID
			}
			else if (function == "releaseUuid?")
			{
				// Release reserved uuid
			}
			else if (function == "ping?")
			{
				// Send pong
				ventilator.send("LennyIndustries|LIES|pong!", 26);
				ventilator.close();
			}
			else if (function == "exit")
			{
				if (message == PASSWD)
				{
					return ERR_R_EXIT;
				}
			}
			else
			{
				std::cout << "Function unknown, ignoring message.\n";
			}
			
			std::cout << "Done.\n";
		}
	}
	catch (zmq::error_t &ex)
	{
		std::cerr << "Caught an exception : " << ex.what();
	}
	
	return ERR_0;
}