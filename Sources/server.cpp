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
#define ERR_2 2 // Key error

int main(int argc, char **argv)
{
	cryptLib::colorPrint("Starting LIES server", DEFAULTCLR);
	// Input Arguments
	inputHandler myInputHandler(argc, argv);
	// Log naming
	std::string logName = myInputHandler.getCmdOption("-log");
	if (logName.empty()) // Default log name
	{
		time_t now = time(nullptr); // https://www.tutorialspoint.com/cplusplus/cpp_date_time.htm
		tm *ltm = localtime(&now);
		std::string dateTime = "_" + std::to_string(1900 + ltm->tm_year) + "-" + std::to_string(ltm->tm_mon) + "-" + std::to_string(ltm->tm_wday);
		dateTime += "_" + std::to_string(ltm->tm_hour) + "-" + std::to_string(ltm->tm_min) + "-" + std::to_string(ltm->tm_sec);
		logName = "LIES" + dateTime + ".log";
	}
	std::cout << "Log name: " << logName << std::endl;
	
	// Starting log
	auto *myLog = lilog::create(logName, true, true);
	LOG(myLog, 1, "Starting LIES server");
	
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
	
	Botan::DataSource_Memory DSMPrivate(keyPrivate_unsecure);
	Botan::DataSource_Memory DSMPublic(keyPublic_unsecure);
	
	Botan::PKCS8_PrivateKey *PKCS8Key_Private;
	Botan::X509_PublicKey *X509Key_public;
	
	try
	{
		PKCS8Key_Private = Botan::PKCS8::load_key(DSMPrivate, rng);
		X509Key_public = Botan::X509::load_key(DSMPublic);
		
		if ((!PKCS8Key_Private->check_key(rng, true)) || (!X509Key_public->check_key(rng, true)))
		{
			cryptLib::colorPrint("Keys failed check", ERRORCLR);
			delete PKCS8Key_Private;
			delete X509Key_public;
			throw std::invalid_argument("Loaded keys are invalid");
		}
	}
	catch (const std::exception &e)
	{
		LOG(myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
		return ERR_2;
	}
	
	std::unique_ptr <Botan::Private_Key> privateKey(PKCS8Key_Private);
	std::unique_ptr <Botan::Public_Key> publicKey(X509Key_public);
	
	// Encryption testing
	{ // Don't need any of this later
		cryptLib::colorPrint("Testing keys:", WAITMSG);
		std::cout << "Private: " << privateKey.get() << "\nPublic: " << publicKey.get() << std::endl;
		// Base text
		std::string plainText = "He had accidentally hacked into his company's server.";
		std::vector<uint8_t> pt(plainText.data(), plainText.data() + plainText.length());
		// Encryption
		Botan::PK_Encryptor_EME enc(*publicKey, rng, "EME1(SHA-256)");
		std::vector<uint8_t> enc_t = enc.encrypt(pt, rng);
		// Decrypt
		Botan::PK_Decryptor_EME dec(*privateKey, rng, "EME1(SHA-256)");
		std::vector<uint8_t> dec_t = Botan::unlock(dec.decrypt(enc_t));
		
		std::cout << "Start String:\n" << plainText << "\nStart vector:\n" << pt.data() << "\nEncrypted:\n" << "Not shown, random data" << "\nDecrypted:\n" << dec_t.data() << std::endl;
		cryptLib::colorPrint("Maximum encrypt size: " + std::to_string(enc.maximum_input_size()), WAITMSG); // Large files: https://stackoverflow.com/questions/13777902/how-to-get-encryption-decryption-progress-when-encrypt-big-files-with-botan-in-q
	}
	
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
			cryptLib::colorPrint(std::string() + "Connecting to: " + LOCALHOST("0"), MSGCLR);
			subscriber.connect(LOCALHOST("24042"));
			ventilator.connect(LOCALHOST("24041"));
		}
		else if (myInputHandler.cmdOptionExists("-local"))
		{
			LOG(myLog, 1, LOCAL("0"));
			cryptLib::colorPrint(std::string() + "Connecting to: " + LOCAL("0"), MSGCLR);
			subscriber.connect(LOCAL("24042"));
			ventilator.connect(LOCAL("24041"));
		}
		else if (myInputHandler.cmdOptionExists("-internet"))
		{
			LOG(myLog, 1, INTERNET("0"));
			cryptLib::colorPrint(std::string() + "Connecting to: " + INTERNET("0"), MSGCLR);
			subscriber.connect(INTERNET("24042"));
			ventilator.connect(INTERNET("24041"));
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
			return ERR_1;
		}
		
		subscriber.set(zmq::sockopt::subscribe, MSG_PREFIX); // LennyIndustries|ProjectName|Function|Message
//		subscriber.setsockopt(ZMQ_SUBSCRIBE, MSG_PREFIX, strlen(MSG_PREFIX));
		
		auto *msg = new zmq::message_t();
		std::string msgStr, function, message, subMsgStr;
		std::size_t pos;
//		unsigned int uuid;
		while (subscriber.handle() != nullptr)
		{
			subscriber.recv(msg);
			LOG(myLog, 1, "Incoming message");
			cryptLib::colorPrint("Incoming message", MSGCLR);
			
			msgStr = std::string(static_cast<char *>(msg->data()), msg->size());
			
			subMsgStr = msgStr.substr(strlen(MSG_PREFIX));
			pos = subMsgStr.find(FILTER_CHAR);
			
			function = subMsgStr.substr(0, pos);
			message = subMsgStr.substr(pos + 1);
			
			std::vector <char> functionVector(function.begin(), function.end());
			std::vector <char> messageVector(message.begin(), message.end());
			
			LOG(myLog, 1, "Function: %s", function.c_str());
			cryptLib::colorPrint(std::string() + "Function: " + function, ALTMSGCLR);
			LOG(myLog, 1, "Size of received message: %i", msg->size());
			cryptLib::colorPrint("Size of received message: " + std::to_string(msg->size()), ALTMSGCLR);
			
			if ((function == "encrypt") || (function == "decrypt"))
			{
				auto *myConnectionHandler = connectionHandler::create(functionVector, messageVector, myLog, &ventilator);
			}
			else if (function == "key")
			{
				// Send the public key LennyIndustries|LIES_Key|
			}
			else if (function == "uuid")
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
			else if (function == "ping")
			{
				// Send pong
				ventilator.send("LennyIndustries|LIES_Pong", 25);
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
				cryptLib::colorPrint("Function unknown, ignoring message", ERRORCLR);
			}
			
			cryptLib::colorPrint("Done", MSGCLR);
		}
	}
	catch (zmq::error_t &ex)
	{
		std::cerr << "Caught an exception : " << ex.what();
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