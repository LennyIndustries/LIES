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
connectionHandler::connectionHandler(std::vector <char> &function, const std::vector <char> &message, lilog *log, zmq::socket_t *vent, std::string key)
{
	// Passed values
	this->function = function;
	this->message = message;
	this->myLog = log;
	this->myVent = vent;
	this->myKeyString = std::move(key);
	// Default values
	this->error = false;
	this->textLength = 0;
	this->imageLength = 0;
	this->keyLength = 0;
	this->passwdLength = 0;
	this->options = 0;
	// Checked values
//	this->encryptSetting = cryptLib::vectorCompare(this->function, "Encrypt");
	
	LOG(this->myLog, 1, "Calling handle");
	handle();
}

// Public
// "Constructor"
connectionHandler *connectionHandler::create(std::vector <char> &function, std::vector <char> &message, lilog *log, zmq::socket_t *vent, std::string key)
{
	return new connectionHandler(function, message, log, vent, std::move(key));
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
	LOG(this->myLog, 1, "Calling messageSolver");
	// If this fails there is no hope
	if (!messageSolver())
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

bool connectionHandler::messageSolver()
{
	// Steps through possible functions and returns a number for them
	std::vector <char> storage;
	std::vector <char> rest;
	std::size_t colonPosition = 0;
	std::size_t equalsPosition;
	
	// Find command & argument
	rest = this->message;
	while (colonPosition != std::string::npos)
	{
		colonPosition = cryptLib::vectorFind(rest, ':');
		std::cout << "Found ':' at: " << colonPosition << std::endl;
		
		if (colonPosition != std::string::npos)
		{
			storage = cryptLib::subVector(rest, 0, colonPosition);
			rest = cryptLib::subVector(rest, colonPosition + 1);
		}
		else
		{
			storage = rest;
			rest.clear();
		}
		
		// Set command & argument
		equalsPosition = cryptLib::vectorFind(storage, '=');
		std::cout << "Found '=' at: " << equalsPosition << std::endl;
		if (equalsPosition != std::string::npos)
		{
			this->messageCommand = cryptLib::subVector(storage, 0, equalsPosition);
		}
		else
		{
			this->messageCommand = storage;
		}
		
		// Filter command & argument
		if ((cryptLib::vectorCompare(this->messageCommand, "TextLength")) && (this->textLength == 0))
		{
			cryptLib::colorPrint("Command: Text length", MSGCLR);
			if (!handleTextLength(storage, equalsPosition))
				return false;
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "Text")) && (equalsPosition != std::string::npos) && (this->textLength > 0))
		{
			cryptLib::colorPrint("Command: Text", MSGCLR);
			this->options |= 0x1;
			if (!handleText(storage, rest, equalsPosition))
				return false;
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "ImageLength")) && (this->imageLength == 0))
		{
			cryptLib::colorPrint("Command: Image length", MSGCLR);
			if (!handleImageLength(storage, equalsPosition))
				return false;
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "Image")) && (equalsPosition != std::string::npos) && (this->imageLength > 0))
		{
			cryptLib::colorPrint("Command: Image", MSGCLR);
			this->options |= 0x2;
			if (!handleImage(storage, rest, equalsPosition))
				return false;
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "KeyLength")) && (this->keyLength == 0))
		{
			cryptLib::colorPrint("Command: Key length", MSGCLR);
			if (!handleKeyLength(storage, equalsPosition))
				return false;
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "Key")) && (equalsPosition != std::string::npos) && (this->keyLength > 0))
		{
			cryptLib::colorPrint("Command: Key", MSGCLR);
			if (!handleKey(storage, rest, equalsPosition))
				return false;
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "PasswordLength")) && (this->passwdLength == 0))
		{
			cryptLib::colorPrint("Command: Password length", MSGCLR);
			if (!handlePasswordLength(storage, equalsPosition))
				return false;
		}
		else if ((cryptLib::vectorCompare(this->messageCommand, "Password")) && (equalsPosition != std::string::npos) && (this->passwdLength > 0))
		{
			cryptLib::colorPrint("Command: Password", MSGCLR);
			this->options |= 0x4;
			if (!handlePassword(storage, rest, equalsPosition))
				return false;
		}
		else if (cryptLib::vectorCompare(this->messageCommand, "UUID"))
		{
			cryptLib::colorPrint("Command: UUID", MSGCLR);
			if (!handleUuid(storage, equalsPosition))
				return false;
		}
		else if ((colonPosition == std::string::npos) && (equalsPosition == std::string::npos)) // End
		{
			return true;
		}
		else
		{
			LOG(this->myLog, 2, "ERROR: Error with command: %s", cryptLib::printableVector(this->messageCommand).c_str());
			cryptLib::colorPrint("ERROR: Error with command: " + cryptLib::printableVector(this->messageCommand), ERRORCLR);
			return false;
		}
	}
	return true;
}

bool connectionHandler::handleTextLength(std::vector <char> &storage, size_t &equalsPosition)
{
	std::vector <char> tmpVector;
	
	tmpVector.clear();
	tmpVector = cryptLib::subVector(storage, equalsPosition + 1);
	
	try
	{
		this->textLength = std::stoi(cryptLib::printableVector(tmpVector));
	}
	catch (const std::exception &e)
	{
		LOG(this->myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
		return false;
	}
	std::cout << "Text length: " << this->textLength << std::endl;
	return true;
}

bool connectionHandler::handleText(std::vector <char> &storage, std::vector <char> &rest, size_t &equalsPosition)
{
	std::vector <char> tempRestVector;
	if (!rest.empty()) // If there is something left add it back to storage
	{
		tempRestVector.clear();
		tempRestVector = rest;
		rest.clear();
		rest.push_back('='); // Add the '=' back since it was removed
		std::copy(tempRestVector.begin(), tempRestVector.end(), std::back_inserter(rest));
		std::copy(rest.begin(), rest.end(), std::back_inserter(storage));
	}
	
	if (this->textLength != cryptLib::subVector(storage, equalsPosition + 1, this->textLength).size())
	{
		LOG(this->myLog, 2, "CRITICAL ERROR : text length does not match vector length");
		cryptLib::colorPrint("CRITICAL ERROR : text length does not match vector length", ERRORCLR);
		this->error = true;
		return false; // Do something to stop the program from continuing.
	}
	
	this->text.clear();
	
	std::vector <char> storageSubstr = cryptLib::subVector(storage, equalsPosition + 1, this->textLength);
	std::copy(storageSubstr.begin(), storageSubstr.end(), std::back_inserter(this->text));
	
	tempRestVector.clear();
	tempRestVector = cryptLib::subVector(storage, (equalsPosition + 1 + this->textLength));
	rest.clear();
	if (!tempRestVector.empty())
	{
		std::cout << "Rest not empty\n";
		std::copy(tempRestVector.begin() + 1, tempRestVector.end(), std::back_inserter(rest));
	}
	else
	{
		std::cout << "Rest empty\n";
		return true;
	}
	return true;
}

bool connectionHandler::handleImageLength(std::vector <char> &storage, size_t &equalsPosition)
{
	std::vector <char> tmpVector;
	
	tmpVector.clear();
	tmpVector = cryptLib::subVector(storage, equalsPosition + 1);
	
	try
	{
		this->imageLength = std::stoi(cryptLib::printableVector(tmpVector));
	}
	catch (const std::exception &e)
	{
		LOG(this->myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
		return false;
	}
	std::cout << "Image length: " << this->imageLength << std::endl;
	return true;
}

bool connectionHandler::handleImage(std::vector <char> &storage, std::vector <char> &rest, size_t &equalsPosition)
{
	std::vector <char> tempRestVector;
	if (!rest.empty()) // If there is something left add it back to storage
	{
		tempRestVector.clear();
		tempRestVector = rest;
		rest.clear();
		rest.push_back('='); // Add the '=' back since it was removed
		std::copy(tempRestVector.begin(), tempRestVector.end(), std::back_inserter(rest));
		std::copy(rest.begin(), rest.end(), std::back_inserter(storage));
	}
	
	if (this->imageLength != cryptLib::subVector(storage, equalsPosition + 1, this->imageLength).size())
	{
		LOG(this->myLog, 2, "CRITICAL ERROR : image length does not match vector length");
		cryptLib::colorPrint("CRITICAL ERROR : image length does not match vector length", ERRORCLR);
		this->error = true;
		return false; // Do something to stop the program from continuing.
	}
	
	std::vector <char> storageSubstr = cryptLib::subVector(storage, equalsPosition + 1, this->imageLength);
	std::copy(storageSubstr.begin(), storageSubstr.end(), std::back_inserter(this->image));
	
	tempRestVector.clear();
	tempRestVector = cryptLib::subVector(storage, (equalsPosition + 1 + this->imageLength));
	rest.clear();
	if (!tempRestVector.empty())
	{
		std::cout << "Rest not empty\n";
		std::copy(tempRestVector.begin() + 1, tempRestVector.end(), std::back_inserter(rest));
	}
	else
	{
		std::cout << "Rest empty\n";
		return true;
	}
	return true;
}

bool connectionHandler::handleKeyLength(std::vector <char> &storage, size_t &equalsPosition)
{
	std::vector <char> tmpVector;
	
	tmpVector.clear();
	tmpVector = cryptLib::subVector(storage, equalsPosition + 1);
	
	try
	{
		this->keyLength = std::stoi(cryptLib::printableVector(tmpVector));
	}
	catch (const std::exception &e)
	{
		LOG(this->myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
		return false;
	}
	std::cout << "Key length: " << this->keyLength << std::endl;
	return true;
}

bool connectionHandler::handleKey(std::vector <char> &storage, std::vector <char> &rest, size_t &equalsPosition)
{
	std::vector <char> tempRestVector;
	if (!rest.empty()) // If there is something left add it back to storage
	{
		tempRestVector.clear();
		tempRestVector = rest;
		rest.clear();
		rest.push_back('='); // Add the '=' back since it was removed
		std::copy(tempRestVector.begin(), tempRestVector.end(), std::back_inserter(rest));
		std::copy(rest.begin(), rest.end(), std::back_inserter(storage));
	}
	
	if (this->keyLength != cryptLib::subVector(storage, equalsPosition + 1, this->keyLength).size())
	{
		LOG(this->myLog, 2, "CRITICAL ERROR : key length does not match vector length");
		cryptLib::colorPrint("CRITICAL ERROR : key length does not match vector length", ERRORCLR);
		this->error = true;
		return false; // Do something to stop the program from continuing.
	}
	
	std::vector <char> storageSubstr = cryptLib::subVector(storage, equalsPosition + 1, this->keyLength);
	std::copy(storageSubstr.begin(), storageSubstr.end(), std::back_inserter(this->key));
	
	tempRestVector.clear();
	tempRestVector = cryptLib::subVector(storage, (equalsPosition + 1 + this->keyLength));
	rest.clear();
	if (!tempRestVector.empty())
	{
		std::cout << "Rest not empty\n";
		std::copy(tempRestVector.begin() + 1, tempRestVector.end(), std::back_inserter(rest));
	}
	else
	{
		std::cout << "Rest empty\n";
		return true;
	}
	return true;
}

bool connectionHandler::handlePasswordLength(std::vector <char> &storage, size_t &equalsPosition)
{
	std::vector <char> tmpVector;
	
	tmpVector.clear();
	tmpVector = cryptLib::subVector(storage, equalsPosition + 1);
	
	try
	{
		this->passwdLength = std::stoi(cryptLib::printableVector(tmpVector));
	}
	catch (const std::exception &e)
	{
		LOG(this->myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
		return false;
	}
	std::cout << "Password length: " << this->passwdLength << std::endl;
	return true;
}

bool connectionHandler::handlePassword(std::vector <char> &storage, std::vector <char> &rest, size_t &equalsPosition)
{
	std::vector <char> tempRestVector;
	if (!rest.empty()) // If there is something left add it back to storage
	{
		tempRestVector.clear();
		tempRestVector = rest;
		rest.clear();
		rest.push_back('='); // Add the '=' back since it was removed
		std::copy(tempRestVector.begin(), tempRestVector.end(), std::back_inserter(rest));
		std::copy(rest.begin(), rest.end(), std::back_inserter(storage));
	}
	
	if (this->passwdLength != cryptLib::subVector(storage, equalsPosition + 1, this->passwdLength).size())
	{
		LOG(this->myLog, 2, "CRITICAL ERROR : password length does not match vector length");
		cryptLib::colorPrint("CRITICAL ERROR : password length does not match vector length", ERRORCLR);
		this->error = true;
		return false; // Do something to stop the program from continuing.
	}
	
	std::vector <char> storageSubstr = cryptLib::subVector(storage, equalsPosition + 1, this->passwdLength);
	std::copy(storageSubstr.begin(), storageSubstr.end(), std::back_inserter(this->passwd));
	
	tempRestVector.clear();
	tempRestVector = cryptLib::subVector(storage, (equalsPosition + 1 + this->passwdLength));
	rest.clear();
	if (!tempRestVector.empty())
	{
		std::cout << "Rest not empty\n";
		std::copy(tempRestVector.begin() + 1, tempRestVector.end(), std::back_inserter(rest));
	}
	else
	{
		std::cout << "Rest empty\n";
		return true;
	}
	return true;
}

bool connectionHandler::handleUuid(std::vector <char> &storage, size_t &equalsPosition)
{
	std::vector <char> tmpVector;
	
	tmpVector.clear();
	tmpVector = cryptLib::subVector(storage, equalsPosition + 1);
	
	try
	{
		this->uuid = Botan::UUID(cryptLib::printableVector(tmpVector));
	}
	catch (const std::exception &e)
	{
		LOG(this->myLog, 2, e.what());
		cryptLib::colorPrint(e.what(), ERRORCLR);
		return false;
	}
	std::cout << "UUID: " << this->uuid.to_string() << std::endl;
	return true;
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
	if (!this->text.empty())
	{
		try
		{
			const Botan::BigInt n = 1000000000000000;
			std::vector <uint8_t> tweak; // No tweak (salt)
			tweak.clear();
			std::unique_ptr <Botan::PBKDF> pbkdf(Botan::PBKDF::create("PBKDF2(SHA-256)"));
			std::unique_ptr <Botan::Cipher_Mode> decFPE = Botan::Cipher_Mode::create("AES-256/SIV", Botan::DECRYPTION);
			decFPE->set_key(this->key);
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
//	if (!this->image.empty())
//	{
//		try
//		{
//			const Botan::BigInt n = 1000000000000000;
//			std::vector <uint8_t> tweak; // No tweak (salt)
//			tweak.clear();
//			std::unique_ptr <Botan::PBKDF> pbkdf(Botan::PBKDF::create("PBKDF2(SHA-256)"));
//			std::unique_ptr <Botan::Cipher_Mode> decFPE = Botan::Cipher_Mode::create("AES-256/SIV", Botan::DECRYPTION);
//			decFPE->set_key(this->key);
//			Botan::secure_vector <uint8_t> ptFPE(this->image.data(), this->image.data() + this->image.size());
//			decFPE->finish(ptFPE);
//			this->image.clear();
//			std::copy(ptFPE.begin(), ptFPE.end(), std::back_inserter(this->image));
//		}
//		catch (const std::exception &e)
//		{
//			LOG(myLog, 2, e.what());
//			cryptLib::colorPrint(e.what(), ERRORCLR);
//			return;
//		}
//	}
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
		myEncrypt.setPasswd(cryptLib::printableVector(this->passwd));
	
	myEncrypt.run();
	std::vector <char> returnVector;
	returnVector = myEncrypt.getData(); // Get encrypted image
	// Prep for sending
	std::vector <char> sendVector;
	std::string msgPrefix = "LennyIndustries|LIES_Client_" + this->uuid.to_string() + '|';
	std::copy(msgPrefix.begin(), msgPrefix.end(), std::back_inserter(sendVector)); // Copy prefix
	std::copy(returnVector.begin(), returnVector.end(), std::back_inserter(sendVector)); // Copy data
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
		myDecrypt.setPasswd(cryptLib::printableVector(this->passwd));
	
	myDecrypt.run();
	std::vector <char> returnVector;
	returnVector = myDecrypt.getData(); // Get decrypted text
	// Prep for sending
	std::vector <char> sendVector;
	std::string msgPrefix = "LennyIndustries|LIES_Client_" + this->uuid.to_string() + '|';
	std::copy(msgPrefix.begin(), msgPrefix.end(), std::back_inserter(sendVector)); // Copy prefix
	std::copy(returnVector.begin(), returnVector.end(), std::back_inserter(sendVector)); // Copy data
	LOG(this->myLog, 1, "Sending text back");
	cryptLib::colorPrint("Sending text back", ALTMSGCLR);
	this->myVent->send(cryptLib::printableVector(sendVector).c_str(), sendVector.size());
}

// Destructor (Private)
connectionHandler::~connectionHandler() = default;
