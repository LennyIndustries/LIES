/*
 * Created by Leander @ Lenny Industries on 22/03/2022.
 * Project: NA.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/lilog.hpp"

lilog::lilog(const std::string &logFile)
{
	this->logFile = logFile;
	myStream.open(logFile);
	if (!myStream.is_open())
	{
		std::cerr << "-!!!- CRITICAL ERROR -!!!-\nCan not open log file!\n";
		// Failure = true
	}
}

bool lilog::log(char logLevel, std::string file, unsigned int line, const char *message, ...)
{
	// LILog V2
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
	
	int n = ::_vscprintf(message, arg);
	char *messageFull = new char[n + 1];
	
	::vsprintf(messageFull, message, arg);
	
	va_end(arg);
	
	myStream << logLevelString << " :: " << dateTime << " :: File: " << file << "(Line: " << line << ") :: " << messageFull << std::endl; // Prints all needed info to the log file
	
	return true;
}

void lilog::clearLogFile()
{
	if (!myStream.is_open())
	{
		myStream.open(logFile);
		if (!myStream.is_open())
		{
			std::cerr << "-!!!- CRITICAL ERROR -!!!-\nCan not open log file!\n";
			return;
		}
	}
	std::ofstream ofs;
	ofs.open(logFile, std::ofstream::out | std::ofstream::trunc);
	ofs.close();
}

void lilog::kill()
{
	delete this;
}

lilog::~lilog()
{
	log(1, __FILE__, __LINE__, "Closing log file (kill).");
	myStream.close();
}
