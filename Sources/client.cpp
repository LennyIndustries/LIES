/*
 * Created by Leander @ Lenny Industries on 25/03/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * LIES: Lenny Industries Encryption Service
 * Can encrypt and decrypt text into bmp images over benternet.
 * The client checks the input given and sends it to the server to process.
 */

// Libraries
#include "include/lilog.hpp"
#include "include/inputHandler.hpp"
#include "include/connectionHandler.hpp"

#include <cstdio>

// Definitions
#define LOCALHOST(port) "tcp://localhost:" port
#define LOCAL(port) "tcp://192.168.1.8:" port
#define INTERNET(port) "tcp://benternet.pxl-ea-ict.be:" port
#define MSG_PREFIX "LennyIndustries|LIES_Client|"
#define FILTER_CHAR '|'
#define TIMEOUT 10

// Error codes
#define ERR_0 0 // No error
#define ERR_1 1 // Invalid network option
#define ERR_2 2 // Key error
#define ERR_3 3 // Input error

// Structs
//struct fileCheck
//{
//	unsigned int b : 3;
//};

// Functions
bool checkFiles(lilog *myLog, char filesToCheck, const std::string &imagePath, const std::string &textPath, const std::string &passwd);

int main(int argc, char **argv)
{
	SetConsoleTitleA("LIES Client");
	cryptLib::colorPrint("Starting LIES client", DEFAULTCLR);
	
	// Starting input Arguments
	inputHandler myInputHandler(argc, argv);
	
	// Setting up log
	std::string logName = myInputHandler.getCmdOption("-log");
	// Log naming
	if (logName.empty()) // Default log name
	{
		time_t now = time(nullptr); // https://www.tutorialspoint.com/cplusplus/cpp_date_time.htm
		tm *ltm = localtime(&now);
		std::string dateTime = "_" + std::to_string(1900 + ltm->tm_year) + "-" + std::to_string(ltm->tm_mon + 1) + "-" + std::to_string(ltm->tm_mday);
		dateTime += "_" + std::to_string(ltm->tm_hour) + "-" + std::to_string(ltm->tm_min) + "-" + std::to_string(ltm->tm_sec);
		logName = "LIES_client" + dateTime + ".log";
	}
	std::cout << "Log name: " << logName << std::endl;
	// Starting log
	auto *myLog = lilog::create(logName, true, true);
	LOG(myLog, 1, "Starting LIES client");
	
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
	
	// Variables
	size_t bits = 0;
	
	// Getting inputs
	std::string imagePath = myInputHandler.getCmdOption("-image");
	std::string textPath = myInputHandler.getCmdOption("-text");
	std::string passwd = myInputHandler.getCmdOption("-passwd");
	int encrypt = myInputHandler.cmdOptionExists("-encrypt") ? 1 : (myInputHandler.cmdOptionExists("-decrypt") ? 0 : -1);
	
	try
	{
		bits = std::stoi(myInputHandler.getCmdOption("-keyBits"));
	}
	catch (const std::exception &e)
	{
		LOG(myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
		cryptLib::colorPrint("Key bits probably not set", ERRORCLR);
	}
	// Checking inputs
	if (encrypt == -1)
	{
		LOG(myLog, 3, "Failed to get encrypt/decrypt command");
		cryptLib::colorPrint("What do you want to do\nYou did not specify whether to encrypt or decrypt\nAborting", ERRORCLR);
		myLog->kill();
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, 0x7);
		
	}
	
	if (encrypt)
	{
		// Encrypting
		LOG(myLog, 1, "Encrypting");
		cryptLib::colorPrint("Encrypting", ALTMSGCLR);
		if (!checkFiles(myLog, 7, imagePath, textPath, passwd))
		{
			myLog->kill();
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, 0x7);
			return ERR_3;
		}
	}
	else
	{
		// Decrypting
		LOG(myLog, 1, "Decrypting");
		cryptLib::colorPrint("Decrypting", ALTMSGCLR);
		if (!checkFiles(myLog, 5, imagePath, textPath, passwd))
		{
			myLog->kill();
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, 0x7);
			return ERR_3;
		}
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
	
	// Generating keys
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
		myLog->kill();
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, 0x7);
		return ERR_2;
	}
	
	// Connecting benternet
	try
	{
		zmq::context_t context(1);
		
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
			throw std::invalid_argument("Invalid network option");
		}
		
		// Now the program starts
		auto *msg = new zmq::message_t();
		std::string msgStr;
		// Pinging server
		cryptLib::colorPrint("Pinging server, if not responsive sever might be offline", WAITMSG);
		subscriber.set(zmq::sockopt::subscribe, "LennyIndustries|LIES_Pong");
		ventilator.send("LennyIndustries|LIES_Server|Ping", 32);
		subscriber.recv(msg);
		msgStr = std::string(static_cast<char *>(msg->data()), msg->size());
		std::cout << "Response" << std::endl;
		// Requesting key
		cryptLib::colorPrint("Requesting public key", WAITMSG);
		subscriber.set(zmq::sockopt::subscribe, "LennyIndustries|LIES_Key|");
		ventilator.send("LennyIndustries|LIES_Server|Key", 31);
		subscriber.recv(msg);
		msgStr = std::string(static_cast<char *>(msg->data()), msg->size());
		std::string serverPublicKey = msgStr.substr(25);
		std::cout << "Server key:\n" << serverPublicKey;
		// Requesting UUID
		cryptLib::colorPrint("Requesting UUID", WAITMSG);
		subscriber.set(zmq::sockopt::subscribe, "LennyIndustries|LIES_UUID|");
		ventilator.send("LennyIndustries|LIES_Server|UUID", 32);
		subscriber.recv(msg);
		msgStr = std::string(static_cast<char *>(msg->data()), msg->size());
		std::string uuid = msgStr.substr(26);
		std::cout << "Assigned UUID:\n" << uuid << std::endl;
		// Requesting encrypt/decrypt
		cryptLib::colorPrint("Requesting encrypt / decrypt", WAITMSG);
		// Loading image
		std::ifstream imageStream(imagePath, std::ios::binary);
		std::vector <char> imageVector = std::vector <char> (std::istreambuf_iterator <char> (imageStream), std::istreambuf_iterator <char> ());
		imageStream.close();
		// Loading text
		std::ifstream textStream(textPath, std::ifstream::binary);
		std::vector <char> textVector = std::vector <char> (std::istreambuf_iterator <char> (textStream), std::istreambuf_iterator <char> ());
		textStream.close();
		// Sending prep
		std::vector <char> messageToSend;
		messageToSend.clear();
		std::string tempString = "LennyIndustries|LIES_Server|Decrypt|UUID=";
		std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
		std::copy(uuid.begin(), uuid.end(), std::back_inserter(messageToSend));
		tempString = ":Password=";
		std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
		std::copy(passwd.begin(), passwd.end(), std::back_inserter(messageToSend));
//		tempString = ":TextLength=" + std::to_string(textVector.size());
//		std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
		tempString = ":ImageLength=" + std::to_string(imageVector.size());
		std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
		tempString = ":Image=";
		std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
		std::copy(imageVector.begin(), imageVector.end(), std::back_inserter(messageToSend));
//		tempString = ":Text=";
//		std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
//		std::copy(textVector.begin(), textVector.end(), std::back_inserter(messageToSend));
		// Sending
		std::string subscribeTo = "LennyIndustries|LIES_Client_" + uuid + "|";
		subscriber.set(zmq::sockopt::subscribe, subscribeTo);
		ventilator.send(cryptLib::printableVector(messageToSend).c_str(), messageToSend.size());
		subscriber.recv(msg);
		
	}
	catch (const std::exception &e)
	{
		LOG(myLog, 3, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
	}
	
	// Halting client
	LOG(myLog, 3, "Halting client");
	cryptLib::colorPrint("Halting client", ALTMSGCLR);
	// Resetting color & killing log
	myLog->kill();
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 0x7);
	// Stop
	return ERR_0;
}

bool checkFiles(lilog *myLog, char filesToCheck, const std::string &imagePath, const std::string &textPath, const std::string &passwd)
{
	struct stat buffer{};
	char mask = 0x1;
	bool returnValue = true;
	if (filesToCheck & mask) // XX1
	{
		if (imagePath.empty())
		{
			LOG(myLog, 3, "Image not set");
			cryptLib::colorPrint("Image not set\nAborting", ERRORCLR);
			returnValue = false;
		}
		else if (stat(imagePath.c_str(), &buffer) != 0)
		{
			LOG(myLog, 3, "Image does not exist");
			cryptLib::colorPrint("Image does not exist\nAborting", ERRORCLR);
			returnValue = false;
		}
		else if (imagePath.substr(imagePath.find('.')) != ".bmp")
		{
			LOG(myLog, 3, "Image not in BMP format");
			cryptLib::colorPrint("Image not in BMP format\nAborting", ERRORCLR);
			returnValue = false;
		}
	}
	filesToCheck >>= 1;
	if (filesToCheck & mask) // X1X
	{
		if (textPath.empty())
		{
			LOG(myLog, 3, "Text not set");
			cryptLib::colorPrint("Text not set\nAborting", ERRORCLR);
			returnValue = false;
		}
		else if (stat(textPath.c_str(), &buffer) != 0)
		{
			LOG(myLog, 3, "Test file does not exist");
			cryptLib::colorPrint("Test file does not exist\nAborting", ERRORCLR);
			returnValue = false;
		}
		else if (textPath.substr(textPath.find('.')) != ".txt")
		{
			LOG(myLog, 3, "Text not in TXT format");
			cryptLib::colorPrint("Text not in TXT format\nAborting", ERRORCLR);
			returnValue = false;
		}
	}
	filesToCheck >>= 1;
	if (filesToCheck & mask) // 1XX
	{
		if (passwd.empty())
		{
			LOG(myLog, 3, "Passwd not set");
			cryptLib::colorPrint("Password not set\nAborting", ERRORCLR);
			returnValue = false;
		}
	}
	return returnValue;
}
