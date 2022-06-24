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
#include <zmq.hpp>
#include <list>

// Definitions
#define LOCALHOST(port) "tcp://localhost:" port
#define LOCAL(port) "tcp://192.168.1.8:" port
#define INTERNET(port) "tcp://benternet.pxl-ea-ict.be:" port
#define MSG_PREFIX "LennyIndustries|LIES_Server|"
#define FILTER_CHAR '|'

// Error codes
#define ERR_R_EXIT -1 // Remote exit request
#define ERR_0 0 // No error
#define ERR_1 1 // Invalid network option
#define ERR_2 2 // Key error

int main(int argc, char **argv)
{
	SetConsoleTitleA("LIES Server");
	cryptLib::colorPrint("Starting LIES server", DEFAULTCLR);
	// Input Arguments
	inputHandler myInputHandler(argc, argv);
	// Log naming
	std::string logName = myInputHandler.getCmdOption("-log");
	if (logName.empty()) // Default log name
	{
		time_t now = time(nullptr); // https://www.tutorialspoint.com/cplusplus/cpp_date_time.htm
		tm *ltm = localtime(&now);
		std::string dateTime = "_" + std::to_string(1900 + ltm->tm_year) + "-" + std::to_string(ltm->tm_mon + 1) + "-" + std::to_string(ltm->tm_mday);
		dateTime += "_" + std::to_string(ltm->tm_hour) + "-" + std::to_string(ltm->tm_min) + "-" + std::to_string(ltm->tm_sec);
		logName = "LIES_server" + dateTime + ".log";
	}
	std::cout << "Log name: " << logName << std::endl;
	
	// Starting log
	auto *myLog = lilog::create(logName, true, true);
	LOG(myLog, 1, "Starting LIES server");
	
	// Checking connection option
	if (!myInputHandler.cmdOptionExists("-localhost") && !myInputHandler.cmdOptionExists("-local") && !myInputHandler.cmdOptionExists("-internet"))
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, ERRORCLR);
		LOG(myLog, 3, "Invalid network option");
		std::cout << "No valid network option given\n";
		std::cout << "Valid options:\n'-localhost': uses localhost address, you need to host the broker yourself\n";
		std::cout << "'-local': uses the local ip of the broker, you need to be on the same network\n'-internet': connects over the internet";
		SetConsoleTextAttribute(hConsole, 0x7);
		return ERR_1;
	}
	
	// Generating keys
	size_t bits = 0;
	try
	{
		bits = std::stoi(myInputHandler.getCmdOption("-keyBits"));
	}
	catch (const std::exception &e)
	{
		LOG(myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
	}
	
	if (bits < 1024)
	{
		cryptLib::colorPrint("Key bits given too low, using lowest", ALTMSGCLR);
		bits = 1024;
	}
	else if (bits > 2048)
	{
		cryptLib::colorPrint("Key bits given too high, using highest", ALTMSGCLR);
		bits = 2048;
	}
	else if ((bits % 256) != 0)
	{
		cryptLib::colorPrint("Key bits given not multiple of 256, using average", ALTMSGCLR);
		bits = 1536;
	}
	// Key prep
	Botan::AutoSeeded_RNG rng;
	LOG(myLog, 1, "Generating key: %i", bits);
	cryptLib::colorPrint("Generating key, this might take a moment\nUsing: " + std::to_string(bits) + " bits", WAITMSG);
	// Key gen
	Botan::RSA_PrivateKey key(rng, bits);
	// The following is unsafe, but for this I don't care
	// WARNING unencrypted key storage is unsafe
	std::string keyPrivate_unsecure = Botan::PKCS8::PEM_encode(key);
	std::string keyPublic_unsecure = Botan::X509::PEM_encode(key);
	// WARNING printing key is unsafe
	std::cout << "Keys:\n" << keyPrivate_unsecure << keyPublic_unsecure;
	
	// Connecting benternet
	std::string ip;
	try
	{
		zmq::context_t context(1);
		
		//Incoming messages come in here
		zmq::socket_t subscriber(context, ZMQ_SUB);
		zmq::socket_t ventilator(context, ZMQ_PUSH);
		if (myInputHandler.cmdOptionExists("-localhost"))
		{
			LOG(myLog, 1, LOCALHOST("0"));
			cryptLib::colorPrint(std::string() + "Connecting to: " + LOCALHOST("0"), MSGCLR);
			subscriber.connect(LOCALHOST("24042"));
			ventilator.connect(LOCALHOST("24041"));
			ip = LOCALHOST("");
		}
		else if (myInputHandler.cmdOptionExists("-local"))
		{
			LOG(myLog, 1, LOCAL("0"));
			cryptLib::colorPrint(std::string() + "Connecting to: " + LOCAL("0"), MSGCLR);
			subscriber.connect(LOCAL("24042"));
			ventilator.connect(LOCAL("24041"));
			ip = LOCAL("");
		}
		else if (myInputHandler.cmdOptionExists("-internet"))
		{
			LOG(myLog, 1, INTERNET("0"));
			cryptLib::colorPrint(std::string() + "Connecting to: " + INTERNET("0"), MSGCLR);
			subscriber.connect(INTERNET("24042"));
			ventilator.connect(INTERNET("24041"));
			ip = INTERNET("");
		}
		else
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, ERRORCLR);
			LOG(myLog, 3, "Invalid network option");
			std::cout << "No valid network option given\n";
			std::cout << "Valid options:\n'-localhost': uses localhost address, you need to host the broker yourself\n";
			std::cout << "'-local': uses the local ip of the broker, you need to be on the same network\n'-internet': connects over the internet";
			SetConsoleTextAttribute(hConsole, 0x7);
			throw std::invalid_argument("Invalid network option");
		}
		
		subscriber.set(zmq::sockopt::subscribe, MSG_PREFIX); // LennyIndustries|ProjectName|Function|Message
//		subscriber.setsockopt(ZMQ_SUBSCRIBE, MSG_PREFIX, strlen(MSG_PREFIX));
		
		auto *msg = new zmq::message_t();
		std::string msgStr, subMsgStr;
		std::size_t pos;
		std::vector <uint8_t> messageVector, function, message;
//		unsigned int uuid;
		std::list<Botan::UUID> usedUUIDs;
		while (subscriber.handle() != nullptr)
		{
			subscriber.recv(msg);
			LOG(myLog, 1, "Incoming message");
			cryptLib::colorPrint("Incoming message", MSGCLR);
			
			messageVector.clear();
			messageVector.resize(msg->size());
			std::memcpy(messageVector.data(), msg->data(), msg->size());
			
			std::vector <uint8_t> tmpVector(cryptLib::subVector(messageVector, strlen(MSG_PREFIX)));
			pos = cryptLib::vectorFind(tmpVector, FILTER_CHAR);
			
			function.clear();
			message.clear();
			
			if (pos == std::string::npos)
			{
				function = tmpVector;
			}
			else
			{
				function = cryptLib::subVector(tmpVector, 0, pos);
				message = cryptLib::subVector(tmpVector, pos + 1);
			}
			
			LOG(myLog, 1, "Function: %s", cryptLib::printableVector(function).c_str());
			cryptLib::colorPrint(std::string() + "Function: " + cryptLib::printableVector(function), ALTMSGCLR);
			LOG(myLog, 1, "Size of received message: %i", message.size());
			cryptLib::colorPrint("Size of received message: " + std::to_string(message.size()), ALTMSGCLR);
			
			if ((cryptLib::vectorCompare(function, "Encrypt")) || (cryptLib::vectorCompare(function, "Decrypt")))
			{
//				subscriber.set(zmq::sockopt::unsubscribe, MSG_PREFIX);
				cryptLib::colorPrint("UUID: " + cryptLib::printableVector(message), ALTMSGCLR);
				usedUUIDs.emplace_back(Botan::UUID(cryptLib::printableVector(message)));
				auto *myConnectionHandler = connectionHandler::create(function, message, myLog, keyPrivate_unsecure, ip);
//				subscriber.set(zmq::sockopt::subscribe, MSG_PREFIX);
			}
			else if (cryptLib::vectorCompare(function, "Key"))
			{
				// Send the public key LennyIndustries|LIES_Key|
				std::string reply = "LennyIndustries|LIES_Key|";
				reply += keyPublic_unsecure;
				LOG(myLog, 1, "Sending key back");
				cryptLib::colorPrint("Sending key back", ALTMSGCLR);
				ventilator.send(reply.c_str(), reply.length());
			}
			else if (cryptLib::vectorCompare(function, "UUID"))
			{
				// Send UUID
				Botan::UUID uuid;
				do
				{
					Botan::AutoSeeded_RNG uuidRng;
					uuid = Botan::UUID(uuidRng);
					std::cout << "New UUID: " << uuid.to_string() << std::endl;
				} while (!uuid.is_valid());
				std::string returnString = "LennyIndustries|LIES_UUID|" + uuid.to_string();
				LOG(myLog, 1, "Sending UUID back: %s", uuid.to_string().c_str());
				cryptLib::colorPrint("Sending UUID back", ALTMSGCLR);
				ventilator.send(returnString.c_str(), returnString.length());
			}
			else if (cryptLib::vectorCompare(function, "UUIDs"))
			{
				// Send used UUIDs back
				cryptLib::colorPrint("Sending used UUIDs back", ALTMSGCLR);
				std::string sendString = "LennyIndustries|LIES_UUIDs|";
				for (auto & usedUUID : usedUUIDs)
				{
					std::cout << usedUUID.to_string() << std::endl;
					sendString += usedUUID.to_string();
					sendString += '\n';
				}
				if (sendString == "LennyIndustries|LIES_UUIDs|")
				{
					sendString += "No used UUIDs yet";
				}
				else
				{
					sendString.erase(sendString.length() - 1); // Remove last new line
				}
				//std::cout << sendString << std::endl; // Debug
				ventilator.send(sendString.c_str(), sendString.length());
			}
			else if (cryptLib::vectorCompare(function, "Ping"))
			{
				// Send pong
				LOG(myLog, 1, "Sending pong");
				cryptLib::colorPrint("Sending pong", ALTMSGCLR);
				ventilator.send("LennyIndustries|LIES_Pong", 25);
			}
			else
			{
				cryptLib::colorPrint("Function unknown, ignoring message", ERRORCLR);
			}
			
			cryptLib::colorPrint("Done", MSGCLR);
		}
	}
	catch (const std::exception &e)
	{
		LOG(myLog, 3, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
	}
	
	// Halting server
	LOG(myLog, 3, "Halting server");
	cryptLib::colorPrint(std::string() + "Halting server", ALTMSGCLR);
	// Resetting color & killing log
	myLog->kill();
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 0x7);
	// Stop
	return ERR_0;
}