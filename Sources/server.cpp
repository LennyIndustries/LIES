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
		std::vector <uint8_t> pt(plainText.data(), plainText.data() + plainText.length());
		// Encryption
		Botan::PK_Encryptor_EME enc(*publicKey, rng, "EME-PKCS1-v1_5");
		std::vector <uint8_t> enc_t = enc.encrypt(pt, rng);
		// Decrypt
		Botan::PK_Decryptor_EME dec(*privateKey, rng, "EME-PKCS1-v1_5");
		std::vector <uint8_t> dec_t = Botan::unlock(dec.decrypt(enc_t));

		std::cout << "Start String:\n" << plainText << "\nStart vector:\n" << pt.data() << "\nEncrypted:\n" << "Not shown, random data" << "\nDecrypted:\n" << dec_t.data() << std::endl;
		cryptLib::colorPrint("Maximum encrypt size: " +
							 std::to_string(enc.maximum_input_size()), ALTMSGCLR); // Large files: https://stackoverflow.com/questions/13777902/how-to-get-encryption-decryption-progress-when-encrypt-big-files-with-botan-in-q

		cryptLib::colorPrint("Testing hash:", WAITMSG);
		std::unique_ptr <Botan::HashFunction> crc32(Botan::HashFunction::create("CRC32"));
		crc32->update(plainText);
		Botan::secure_vector <uint8_t> hashOut = crc32->final();
		std::cout << "CRC32 hash: " << Botan::hex_encode(hashOut) << "\n should be equal to 'B62C54AB'" << std::endl;

		cryptLib::colorPrint("Testing text encryption:", WAITMSG);
		const Botan::BigInt n = 1000000000000000;
		std::string pass = "Testing"; // A password, given by the user
		const std::vector <uint8_t> tweak = Botan::unlock(hashOut); // tweak based on text hash
		std::unique_ptr <Botan::PBKDF> pbkdf(Botan::PBKDF::create("PBKDF2(SHA-256)"));
		// Encryption
		std::unique_ptr <Botan::Cipher_Mode> encFPE = Botan::Cipher_Mode::create("AES-256/SIV", Botan::ENCRYPTION);
		Botan::secure_vector <uint8_t> pbkdfKey = pbkdf->pbkdf_iterations(encFPE->maximum_keylength(), pass, tweak.data(), tweak.size(), 100000);
		encFPE->set_key(pbkdfKey);
//		Botan::secure_vector<uint8_t> ivFPE = rng.random_vec(encFPE->default_nonce_length());
		plainText += " Some more and longer text. This text needs to be longer so it can be used to test the encryption, if it is not long enough text send to the server might fail as well. Also We'll use this method of encryption to encrypt the text and image data send to and from the server having only RSA encryption on the key to encrypt and dercypt the data send.";// + "He had accidentally hacked into his company's server." + "He had accidentally hacked into his company's server." + "He had accidentally hacked into his company's server.";
		Botan::secure_vector <uint8_t> ptFPE(plainText.data(), plainText.data() + plainText.length());
//		encFPE->start(ivFPE);
		encFPE->finish(ptFPE);
		std::cout << "Encrypted:\n" << "Not shown, random data" << std::endl;

		// Encryption of the key with RSA
//		Botan::PK_Encryptor_EME encKey(*publicKey, rng, "EME-PKCS1-v1_5");
//		std::vector <uint8_t> encKey_t = enc.encrypt(pbkdfKey, rng);
//
//		Botan::PK_Decryptor_EME decKey(*privateKey, rng, "EME-PKCS1-v1_5");
//		std::vector <uint8_t> decKey_t = Botan::unlock(dec.decrypt(encKey_t));

		// Decryption
		Botan::secure_vector <uint8_t> pbkdfKey2 = pbkdf->pbkdf_iterations(encFPE->maximum_keylength(), pass, tweak.data(), tweak.size(), 100000);
		std::unique_ptr <Botan::Cipher_Mode> decFPE = Botan::Cipher_Mode::create("AES-256/SIV", Botan::DECRYPTION);
		decFPE->set_key(pbkdfKey2);
//		decFPE->start(ivFPE);
		decFPE->finish(ptFPE);
		Botan::secure_vector <uint8_t> out(ptFPE.begin(), ptFPE.end());
		std::cout << "Decrypted:\n" << out.data() << std::endl;
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
			throw std::invalid_argument("Invalid network option");
		}
		
		subscriber.set(zmq::sockopt::subscribe, MSG_PREFIX); // LennyIndustries|ProjectName|Function|Message
//		subscriber.setsockopt(ZMQ_SUBSCRIBE, MSG_PREFIX, strlen(MSG_PREFIX));
		
		auto *msg = new zmq::message_t();
		std::string msgStr, subMsgStr;
		std::size_t pos;
		std::vector <char> messageVector, function, message;
//		unsigned int uuid;
		while (subscriber.handle() != nullptr)
		{
			subscriber.recv(msg);
			LOG(myLog, 1, "Incoming message");
			cryptLib::colorPrint("Incoming message", MSGCLR);
			
			messageVector.clear();
			messageVector.resize(msg->size());
			std::memcpy(messageVector.data(), msg->data(), msg->size());
			
			std::vector <char> tmpVector(cryptLib::subVector(messageVector, strlen(MSG_PREFIX)));
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
				auto *myConnectionHandler = connectionHandler::create(function, message, myLog, &ventilator);
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