/*
 * Created by Leander @ Lenny Industries on 22/03/2022.
 * Project: NA.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/lilog.hpp"

// Constructor (Private)
lilog::lilog(const std::string &logFile, bool clear)
{
	this->logFile = logFile;
	clear ? this->clearLogFile() : this->open();

//	else
//	{
//		auto lam = [](int i)
//		{
//			std::cout << "aborting" << std::endl;
//			exit(0);
//		};
//
//		//^C
//		signal(SIGINT, lam);
//		//abort()
//		signal(SIGABRT, lam);
//		//sent by "kill" command
//		signal(SIGTERM, lam);
//	}
}

// Public
// "Constructor"
lilog *lilog::create(const std::string &logFile, bool clear)
{
	return new lilog(logFile, clear);
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
	
	int n = ::_vscprintf(message, arg);
	char *messageFull = new char[n + 1];
	
	::vsprintf(messageFull, message, arg);
	
	va_end(arg);
	
	myStream << logLevelString << " :: " << dateTime << " :: File: " << file << " (Line: " << line << ") :: " << messageFull << std::endl; // Prints all needed info to the log file
	
	return true;
}

void lilog::clearLogFile()
{
	if (myStream.is_open()) this->close();
	myStream.open(logFile, std::ofstream::out | std::ofstream::trunc);
	myStream.close();
	this->open();
	log(1, __FILE__, __LINE__, "Log file cleared.");
}

void lilog::open()
{
	myStream.open(logFile, std::ofstream::out | std::ofstream::app);
	if (!myStream.is_open())
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
	myStream.close();
}

// Destructor (Private)
lilog::~lilog()
{
	if (myStream.is_open()) this->close();
}
