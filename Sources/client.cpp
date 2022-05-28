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
bool checkFile(lilog *myLog, const std::string &checkThis, const std::string &extension, bool exist);

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
	std::string smartExtension = ".txt";
	
	// Getting inputs
	std::string imagePath = myInputHandler.getCmdOption("-image");
	std::string textPath = myInputHandler.getCmdOption("-text");
	std::string passwd = myInputHandler.getCmdOption("-passwd");
	int encrypt = myInputHandler.cmdOptionExists("-encrypt") ? 1 : (myInputHandler.cmdOptionExists("-decrypt") ? 0 : -1);
	
	// Checking inputs
	if (encrypt == -1)
	{
		LOG(myLog, 3, "Failed to get encrypt/decrypt command");
		cryptLib::colorPrint("What do you want to do\nYou did not specify whether to encrypt or decrypt\nAborting", ERRORCLR);
		myLog->kill();
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, 0x7);
		return ERR_3;
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
		// Setting up key
		cryptLib::colorPrint("Setting up server key", WAITMSG);
		Botan::AutoSeeded_RNG rng;
		Botan::DataSource_Memory DSMPublicServer(serverPublicKey);
		Botan::X509_PublicKey *X509Key_publicServer;
		try
		{
			X509Key_publicServer = Botan::X509::load_key(DSMPublicServer);
			
			if (!X509Key_publicServer->check_key(rng, true))
			{
				cryptLib::colorPrint("Server key failed check", ERRORCLR);
				delete X509Key_publicServer;
				throw std::invalid_argument("Server key is invalid");
			}
		}
		catch (const std::exception &e)
		{
			LOG(myLog, 2, e.what());
			cryptLib::colorPrint(e.what(), ERRORCLR);
			return ERR_2;
		}
		std::unique_ptr <Botan::Public_Key> publicKeyServer(X509Key_publicServer);
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
		// Loading & encrypting image & text & password
		const Botan::BigInt n = 1000000000000000;
		std::vector <uint8_t> tweak; // No tweak (salt)
		tweak.clear();
		std::unique_ptr <Botan::PBKDF> pbkdf(Botan::PBKDF::create("PBKDF2(SHA-256)"));
		std::unique_ptr <Botan::Cipher_Mode> encryption = Botan::Cipher_Mode::create("AES-256/SIV", Botan::ENCRYPTION);
		Botan::secure_vector <uint8_t> key = pbkdf->pbkdf_iterations(encryption->maximum_keylength(), passwd, tweak.data(), tweak.size(), 100000);
		// Image
		std::vector <char> imageVector;
		if (checkFile(myLog, imagePath, "bmp", false))
		{
			std::ifstream imageStream(imagePath, std::ios::binary);
			imageVector = std::vector <char>(std::istreambuf_iterator <char>(imageStream), std::istreambuf_iterator <char>());
			imageStream.close();
			
//			encryption->set_key(key);
//			Botan::secure_vector <uint8_t> dataVectorImage(imageVector.data(), imageVector.data() + imageVector.size());
//			encryption->finish(dataVectorImage);
//			imageVector.clear();
//			std::copy(dataVectorImage.begin(), dataVectorImage.end(), std::back_inserter(imageVector));
		}
		// Text
		std::vector <char> textVector;
		if (checkFile(myLog, textPath, "txt", false))
		{
			std::ifstream textStream(textPath, std::ifstream::binary);
			textVector = std::vector <char>(std::istreambuf_iterator <char>(textStream), std::istreambuf_iterator <char>());
			textStream.close();
			
			encryption->set_key(key);
			Botan::secure_vector <uint8_t> dataVectorText(textVector.data(), textVector.data() + textVector.size());
			encryption->finish(dataVectorText);
			textVector.clear();
			std::copy(dataVectorText.begin(), dataVectorText.end(), std::back_inserter(textVector));
		}
		// Password
		if (!passwd.empty())
		{
			encryption->set_key(key);
			Botan::secure_vector <uint8_t> dataVectorPasswd(passwd.data(), passwd.data() + passwd.size());
			encryption->finish(dataVectorPasswd);
			passwd.clear();
			std::copy(dataVectorPasswd.begin(), dataVectorPasswd.end(), std::back_inserter(passwd));
		}
		// RSA encryption on key for sending
		Botan::PK_Encryptor_EME encKey(*publicKeyServer, rng, "EME-PKCS1-v1_5");
		std::vector <uint8_t> encKey_t = encKey.encrypt(key, rng);
		// Sending prep
		std::vector <char> messageToSend;
		messageToSend.clear();
		// Message prefix
		std::string tempString = "LennyIndustries|LIES_Server|";
		tempString += encrypt ? "Encrypt" : "Decrypt";
		tempString += "|UUID=";
		std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
		std::copy(uuid.begin(), uuid.end(), std::back_inserter(messageToSend));
		tempString = ":KeyLength=" + std::to_string(encKey_t.size());
		std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
		tempString = ":Key=";
		std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
		std::copy(encKey_t.begin(), encKey_t.end(), std::back_inserter(messageToSend));
		if (!textVector.empty())
		{
			tempString = ":TextLength=" + std::to_string(textVector.size());
			std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
			tempString = ":Text=";
			std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
			std::copy(textVector.begin(), textVector.end(), std::back_inserter(messageToSend));
		}
		if (!imageVector.empty())
		{
			tempString = ":ImageLength=" + std::to_string(imageVector.size());
			std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
			tempString = ":Image=";
			std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
			std::copy(imageVector.begin(), imageVector.end(), std::back_inserter(messageToSend));
		}
		if (!passwd.empty())
		{
			tempString = ":PasswordLength=" + std::to_string(passwd.size());
			std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
			tempString = ":Password=";
			std::copy(tempString.begin(), tempString.end(), std::back_inserter(messageToSend));
			std::copy(passwd.begin(), passwd.end(), std::back_inserter(messageToSend));
		}
		// Sending
		std::string subscribeTo = "LennyIndustries|LIES_Client_" + uuid + "|";
		subscriber.set(zmq::sockopt::subscribe, subscribeTo);
		ventilator.send(cryptLib::printableVector(messageToSend).c_str(), messageToSend.size());
		subscriber.recv(msg);
		msgStr = std::string(static_cast<char *>(msg->data()), msg->size());
		std::string data = msgStr.substr(subscribeTo.length());
		std::cout << "Received data:\n" << data << std::endl;
		if (data[0] == 'B' && data[1] == 'M')
		{
			std::ofstream output("LIES_output.bmp", std::ios::out | std::ofstream::trunc | std::ofstream::binary);
			std::copy(data.begin(), data.end(), std::ostreambuf_iterator <char>(output));
			output.close();
		}
		else
		{
			std::ofstream output("LIES_output.txt", std::ios::out | std::ofstream::trunc);
			if (passwd.empty() && ((imageVector.empty() && !textVector.empty()) || (textVector.empty() && !imageVector.empty() && encrypt)) && (data != "ERROR_OCCURRED"))
			{
				std::cout << "CRC32 to string\n";
				std::vector <unsigned char> tmpVector;
				std::copy(data.begin(), data.end(), std::back_inserter(tmpVector));
				std::string tmpOutString = Botan::hex_encode(tmpVector);
				std::cout << tmpOutString << std::endl;
				data.clear();
				std::copy(tmpOutString.begin(), tmpOutString.end(), std::back_inserter(data));
			}
			std::copy(data.begin(), data.end(), std::ostreambuf_iterator <char>(output));
			output.close();
		}
		cryptLib::colorPrint("Done", ALTMSGCLR);
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

bool checkFile(lilog *myLog, const std::string &checkThis, const std::string &extension, bool exist)
{
	if (!checkThis.empty())
	{
		if (exist)
		{
			struct stat buffer{};
			if (stat(checkThis.c_str(), &buffer) != 0)
			{
				LOG(myLog, 3, "File does not exist: %s", checkThis.c_str());
				cryptLib::colorPrint("File does not exist: " + checkThis, ERRORCLR);
				return false;
			}
		}
		if (checkThis.substr(checkThis.find_last_of('.') + 1) != extension)
		{
			LOG(myLog, 3, "File has incorrect extension: %s Expected:%s", checkThis.c_str(), extension.c_str());
			cryptLib::colorPrint("File has incorrect extension: " + checkThis + " Expected: " + extension, ERRORCLR);
			return false;
		}
		return true;
	}
	else
	{
		LOG(myLog, 3, "File to check empty");
		cryptLib::colorPrint("File to check empty", ERRORCLR);
		return false;
	}
}
