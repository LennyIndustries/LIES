/*
 * Created by Leander @ Lenny Industries on 19/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * Find out what has been requested and solve the message
 * Keep thread alive until everything has been handled, then close connection and thread
 */

#include "include/connectionHandler.hpp"

// Constructor (Private)
connectionHandler::connectionHandler(std::vector <uint8_t> &function, const std::vector <uint8_t> &message, lilog *log, zmq::socket_t *vent, std::string key, zmq::socket_t *rec)
{
	// Passed values
	this->function = function;
	this->message = message;
	this->myLog = log;
	this->myVent = vent; // New
	this->myRec = rec; // New
	this->myKeyString = std::move(key);
	// Default values
	this->error = false;
	this->options = 0;
	
	LOG(this->myLog, 1, "Calling handle");
	handle(); // To threat
}

// Public
// "Constructor"
connectionHandler *connectionHandler::create(std::vector <uint8_t> &function, std::vector <uint8_t> &message, lilog *log, zmq::socket_t *vent, std::string key, zmq::socket_t *rec)
{
	return new connectionHandler(function, message, log, vent, std::move(key), rec);
}

// "Destructor"
void connectionHandler::kill()
{
	delete this;
}

Botan::UUID connectionHandler::getUUID() const
{
	return this->uuid;
}

// Protected
// Private
void connectionHandler::handle()
{
	LOG(this->myLog, 1, "Calling messageHandler");
	// If this fails there is no hope
	if (!messageHandler())
	{
		LOG(this->myLog, 2, "Message solver failed");
		cryptLib::colorPrint("Message solver failed", ERRORCLR);
		this->error = true;
	}
	// Data decryption
	if (!this->key.empty())
	{
		if (!decryptKey())
		{
			LOG(this->myLog, 2, "Failed to decrypt key");
			cryptLib::colorPrint("Failed to decrypt key", ERRORCLR);
			this->error = true;
		}
		else
		{
			decryptData();
		}
	}
	else
	{
		LOG(this->myLog, 2, "No key provided");
		cryptLib::colorPrint("No key provided", ERRORCLR);
		this->error = true;
	}
	// Option based checks
	if (this->text.empty() && (this->options & 0x1))
	{
		LOG(this->myLog, 2, "Failed to set text");
		cryptLib::colorPrint("Failed to set text", ERRORCLR);
		this->error = true;
	}
	if (this->image.empty() && (this->options & 0x2))
	{
		LOG(this->myLog, 2, "Failed to set image");
		cryptLib::colorPrint("Failed to set image", ERRORCLR);
		this->error = true;
	}
	if (this->passwd.empty() && (this->options & 0x4))
	{
		LOG(this->myLog, 2, "Failed to set password");
		cryptLib::colorPrint("Failed to set password", ERRORCLR);
		this->error = true;
	}
	// General checks
	if (!this->image.empty()) // If there is an image it needs to be BMP
	{
		if ((this->image[0] != 'B') || (this->image[1] != 'M'))
		{
			LOG(this->myLog, 2, "BMP file error");
			cryptLib::colorPrint("BMP file error", ERRORCLR);
			this->error = true;
		}
	}
	if (!this->uuid.is_valid()) // Always need a UUID
	{
		LOG(this->myLog, 2, "UUID not valid");
		cryptLib::colorPrint("UUID not valid", ERRORCLR);
		this->error = true;
	}
	else
	{
		if (this->uuid.to_string().empty())
		{
			LOG(this->myLog, 2, "UUID not set");
			cryptLib::colorPrint("UUID not set", ERRORCLR);
			this->error = true;
		}
	}
	// Error check
	if (this->error)
	{
		LOG(this->myLog, 3, "A critical error occurred, aborting");
		cryptLib::colorPrint("A critical error occurred, aborting", ERRORCLR);
		std::string message = "LennyIndustries|LIES_Client_" + this->uuid.to_string() + "|ERROR_OCCURRED";
		LOG(this->myLog, 2, "Sending error message back");
		cryptLib::colorPrint("Sending error message back", ERRORCLR);
		this->myVent->send(message.c_str(), message.length());
		this->kill();
	}
	else if (cryptLib::vectorCompare(this->function, "Encrypt"))
	{
		// Encrypt
		encryptCall();
		this->kill();
	}
	else if (cryptLib::vectorCompare(this->function, "Decrypt"))
	{
		// Decrypt
		decryptCall();
		this->kill();
	}
	else // Redundant function check
	{
		LOG(this->myLog, 3, "ERROR: Unknown function: %s", cryptLib::printableVector(function).c_str());
		cryptLib::colorPrint("ERROR: Unknown function: " + cryptLib::printableVector(function), ERRORCLR);
		this->kill();
	}
}

bool connectionHandler::messageHandler()
{
	// UUID, this is attached to the start string
	cryptLib::colorPrint("Setting UUID", MSGCLR);
	try
	{
		this->uuid = Botan::UUID(cryptLib::printableVector(this->message));
		if (!this->uuid.is_valid())
			throw std::invalid_argument("Invalid UUID");
	}
	catch (const std::exception &e)
	{
		LOG(this->myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
		return false;
	}
	std::cout << "UUID: " << this->uuid.to_string() << std::endl;
	{
		std::string prefix;
		prefix = "LennyIndustries|LIES_Client_" + this->uuid.to_string() + "|";
		std::copy(prefix.begin(), prefix.end(), std::back_inserter(this->clientPrefix));
		prefix = "LennyIndustries|LIES_Server_" + this->uuid.to_string() + "|";
		std::copy(prefix.begin(), prefix.end(), std::back_inserter(this->myPrefix));
	}
	this->myRec->set(zmq::sockopt::subscribe, cryptLib::printableVector(this->myPrefix));
	// Requesting key
	cryptLib::colorPrint("Requesting key", WAITMSG);
	this->key.clear();
	sendRequest("Key?", this->key);
	if (this->key.empty())
		return false;
	// Requesting text
	cryptLib::colorPrint("Requesting text", WAITMSG);
	this->text.clear();
	sendRequest("Text?", this->text);
	if (!this->text.empty())
		this->options |= 0x1;
	// Requesting image
	cryptLib::colorPrint("Requesting image", WAITMSG);
	this->image.clear();
	sendRequest("Image?", this->image);
	if (!this->image.empty())
		this->options |= 0x2;
	// Requesting passwd
	cryptLib::colorPrint("Requesting password", WAITMSG);
	this->passwd.clear();
	sendRequest("Passwd?", this->passwd);
	if (!this->passwd.empty())
		this->options |= 0x4;
	
	return true;
}

void connectionHandler::sendRequest(const std::string& request, std::vector <uint8_t> & putItHere)
{
	std::string sendString = cryptLib::printableVector(this->clientPrefix) + request;
	auto *msg = new zmq::message_t();
	this->myVent->send(sendString.c_str(), sendString.size());
	this->myRec->recv(msg);
	std::vector <uint8_t> returnVector;
	std::string msgStr = std::string(static_cast<char *>(msg->data()), msg->size());
	std::string msgStrSub = msgStr.substr(this->myPrefix.size());
	std::copy(msgStrSub.begin(), msgStrSub.end(), std::back_inserter(putItHere));
}

bool connectionHandler::decryptKey()
{
	cryptLib::colorPrint("Decrypting key", MSGCLR);
	// Decrypt prep
	Botan::AutoSeeded_RNG rngTest;
	Botan::DataSource_Memory DSMPrivate(this->myKeyString);
	Botan::PKCS8_PrivateKey *PKCS8Key_Private;
	try
	{
		PKCS8Key_Private = Botan::PKCS8::load_key(DSMPrivate, rngTest);
		
		if (!PKCS8Key_Private->check_key(rngTest, true))
		{
			cryptLib::colorPrint("Key failed check", ERRORCLR);
			delete PKCS8Key_Private;
			throw std::invalid_argument("Loaded key is invalid");
		}
		
		std::unique_ptr <Botan::Private_Key> privateKey(PKCS8Key_Private);
		Botan::PK_Decryptor_EME dec(*privateKey, rngTest, "EME-PKCS1-v1_5");
		// Decrypting
		std::vector <uint8_t> dec_t = Botan::unlock(dec.decrypt(this->key)); // This throws errors
		this->key.clear();
		std::copy(dec_t.begin(), dec_t.end(), std::back_inserter(this->key));
	}
	catch (const std::exception &e)
	{
		LOG(myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
		return false;
	}
	return true;
}

void connectionHandler::decryptData()
{
	cryptLib::colorPrint("Decrypting data", MSGCLR);
	
	const Botan::BigInt n = 1000000000000000;
	std::vector <uint8_t> tweak; // No tweak (salt)
	tweak.clear();
	std::unique_ptr <Botan::PBKDF> pbkdf(Botan::PBKDF::create("PBKDF2(SHA-256)"));
	std::unique_ptr <Botan::Cipher_Mode> decFPE = Botan::Cipher_Mode::create("AES-256/SIV", Botan::DECRYPTION);
	decFPE->set_key(this->key);
	
	if (!this->text.empty())
	{
		try
		{
			Botan::secure_vector <uint8_t> ptFPE(this->text.data(), this->text.data() + this->text.size());
			decFPE->finish(ptFPE);
			this->text.clear();
			std::copy(ptFPE.begin(), ptFPE.end(), std::back_inserter(this->text));
		}
		catch (const std::exception &e)
		{
			LOG(myLog, 2, e.what());
			cryptLib::colorPrint(e.what(), ERRORCLR);
			return;
		}
	}
	if (!this->image.empty())
	{
		try
		{
			Botan::secure_vector <uint8_t> ptFPE(this->image.data(), this->image.data() + this->image.size());
			decFPE->finish(ptFPE);
			this->image.clear();
			std::copy(ptFPE.begin(), ptFPE.end(), std::back_inserter(this->image));
		}
		catch (const std::exception &e)
		{
			LOG(myLog, 2, e.what());
			cryptLib::colorPrint(e.what(), ERRORCLR);
			return;
		}
	}
}

void connectionHandler::encryptCall()
{
	cryptLib::colorPrint("Calling encrypt", MSGCLR);
	encrypt myEncrypt = encrypt(this->myLog);
	char mask = 0x1;
	int check;
	check = mask & this->options;
	if (check)
		myEncrypt.setText(this->text);
	mask <<= 1;
	check = mask & this->options;
	if (check)
		myEncrypt.setImage(this->image);
	mask <<= 1;
	check = mask & this->options;
	if (check)
		myEncrypt.setPasswd(this->passwd);
	
	myEncrypt.run();
	std::vector <uint8_t> returnVector;
	returnVector = myEncrypt.getData(); // Get encrypted image
	// Prep for sending
	std::vector <uint8_t> sendVector;
	std::string msgPrefix = "LennyIndustries|LIES_Client_" + this->uuid.to_string() + '|';
	// Encrypting data
	std::unique_ptr <Botan::Cipher_Mode> encryption = Botan::Cipher_Mode::create("AES-256/SIV", Botan::ENCRYPTION);
	encryption->set_key(this->key);
	Botan::secure_vector <uint8_t> dataVectorText(returnVector.data(), returnVector.data() + returnVector.size());
	encryption->finish(dataVectorText);
	returnVector.clear();
	std::copy(dataVectorText.begin(), dataVectorText.end(), std::back_inserter(returnVector));
	// Coping data
	std::copy(msgPrefix.begin(), msgPrefix.end(), std::back_inserter(sendVector)); // Copy prefix
	std::copy(returnVector.begin(), returnVector.end(), std::back_inserter(sendVector)); // Copy data
	// Sending
	LOG(this->myLog, 1, "Sending data back");
	cryptLib::colorPrint("Sending data back", ALTMSGCLR);
	this->myVent->send(cryptLib::printableVector(sendVector).c_str(), sendVector.size());
}

void connectionHandler::decryptCall()
{
	cryptLib::colorPrint("Calling decrypt", MSGCLR);
	decrypt myDecrypt = decrypt(this->myLog);
	char mask = 0x1;
	int check;
	
	check = mask & this->options;
	if (check)
		myDecrypt.setText(this->text);
	mask <<= 1;
	check = mask & this->options;
	if (check)
		myDecrypt.setImage(this->image);
	mask <<= 1;
	check = mask & this->options;
	if (check)
		myDecrypt.setPasswd(this->passwd);
	
	myDecrypt.run();
	std::vector <uint8_t> returnVector;
	returnVector = myDecrypt.getData(); // Get decrypted text
	// Prep for sending
	std::vector <uint8_t> sendVector;
	std::string msgPrefix = "LennyIndustries|LIES_Client_" + this->uuid.to_string() + '|';
	// Encrypting data
	std::unique_ptr <Botan::Cipher_Mode> encryption = Botan::Cipher_Mode::create("AES-256/SIV", Botan::ENCRYPTION);
	encryption->set_key(this->key);
	Botan::secure_vector <uint8_t> dataVectorText(returnVector.data(), returnVector.data() + returnVector.size());
	encryption->finish(dataVectorText);
	returnVector.clear();
	std::copy(dataVectorText.begin(), dataVectorText.end(), std::back_inserter(returnVector));
	// Coping data
	std::copy(msgPrefix.begin(), msgPrefix.end(), std::back_inserter(sendVector)); // Copy prefix
	std::copy(returnVector.begin(), returnVector.end(), std::back_inserter(sendVector)); // Copy data
	LOG(this->myLog, 1, "Sending text back");
	cryptLib::colorPrint("Sending text back", ALTMSGCLR);
	this->myVent->send(cryptLib::printableVector(sendVector).c_str(), sendVector.size());
}

// Destructor (Private)
connectionHandler::~connectionHandler() = default;
