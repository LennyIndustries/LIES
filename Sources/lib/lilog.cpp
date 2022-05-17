/*
 * Created by Leander @ Lenny Industries on 22/03/2022.
 * Project: NA.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/lilog.hpp"

// Constructor (Private)
lilog::lilog(const std::string &logFile, bool createBackup, bool clear)
{
	this->logFile = logFile;
	
	if (createBackup)
	{
		struct stat buffer{};
		if (stat(logFile.c_str(), &buffer) == 0) // Create a backup of the old log file
		{
			std::cout << "Creating backup of old log file.\n";
			std::string tmpLogName = logFile.substr(0, logFile.find('.'));
			tmpLogName += "_old.log";
			std::cout << tmpLogName << std::endl;
			if (remove(tmpLogName.c_str()) != 0)
			{
				std::cout << "Failed to delete.\n";
			}
			if (rename(logFile.c_str(), tmpLogName.c_str()) != 0)
			{
				std::cout << "Failed to rename.\n";
			}
		}
	}
	
	clear ? this->clearLogFile() : this->open();
}

// Public
// "Constructor"
lilog *lilog::create(const std::string &logFile, bool createBackup, bool clear)
{
	return new lilog(logFile, createBackup, clear);
}

// "Destructor"
void lilog::kill()
{
	log(1, __FILE__, __LINE__, "Killing log file.");
	delete this;
}

bool lilog::log(char logLevel, std::string file, unsigned int line, const char *message, ...)
{
	// A big ball of wibbly wobbly, timey wimey stuff.
	char dateTime[22] = {'\0'};
	time_t myTime = time(nullptr);
	// Log file.
	std::string logLevelString;
	// Arguments
	va_list arg;
	
	switch (logLevel) // Sets the log level from given number
	{
		case 1:
			logLevelString = "INFO";
			break;
		case 2:
			logLevelString = "WARN";
			break;
		case 3:
			logLevelString = "CRIT";
			break;
		default:
			lilog::log(2, std::move(file), line, "Could not resolve log level: %i.", logLevel);
			return false;
	}
	
	strftime(dateTime, 22, "%H:%M:%S - %d/%m/%Y", localtime(&myTime)); // Creates date and time string for log file
	
	va_start(arg, message); // Argument handling
	
	int n = vsnprintf(nullptr, 0, message, arg);
	char messageFull[n];
	
	::vsprintf(messageFull, message, arg);
	
	va_end(arg);
	
	this->myStream << logLevelString << " :: " << dateTime << " :: File: " << file << " (Line: " << line << ") :: " << messageFull << std::endl; // Prints all needed info to the log file
	
	return true;
}

void lilog::clearLogFile()
{
	if (this->myStream.is_open())
		this->close();
	this->myStream.open(this->logFile, std::ofstream::out | std::ofstream::trunc);
	this->myStream.close();
	this->open();
	log(1, __FILE__, __LINE__, "Log file cleared.");
}

void lilog::open()
{
	this->myStream.open(this->logFile, std::ofstream::out | std::ofstream::app);
	if (!this->myStream.is_open())
	{
		std::cerr << "-!!!- CRITICAL ERROR -!!!-\nCan not open log file!\n";
		this->kill();
		return;
	}
	log(1, __FILE__, __LINE__, "Opening log file.");
}

void lilog::close()
{
	log(1, __FILE__, __LINE__, "Closing log file.");
	this->myStream.close();
}

// Destructor (Private)
lilog::~lilog()
{
	if (this->myStream.is_open())
		this->close();
}
