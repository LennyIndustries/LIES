/*
 * Created by Leander @ Lenny Industries on 22/03/2022.
 * Project: NA.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/lilog.hpp"

lilog::lilog(char *logFile)
{
	this->logFile = logFile;
	myStream.open(logFile);
}

void lilog::sayHello()
{
	printf("Hello World - LILog\n");
}

bool lilog::log(char logLevel, std::string file, unsigned int line, const char *message, ...)
{
	// LILog V2
	// A big ball of wibbly wobbly, timey wimey stuff.
	char dateTime[22] = {'\0'};
	time_t myTime = time(nullptr);
	// Log file stream
	std::ofstream myStream;
	
	if (!myStream.is_open())
	{
		myStream.open(logFile);
	}
	
	return true;
	
//	// A big ball of wibbly wobbly, timey wimey stuff.
//	char dateTime[22] = {'\0'};
//	time_t myTime = time(nullptr);
//	// Log file.
//	FILE *myLogFile = nullptr;
//	char *logLevelString = nullptr;
//	// Arguments
//	va_list arg;
//
//	switch (logLevel) // Sets the log level from given number
//	{
//		case 1:
//			strcpy(logLevelString, "INFO");
//			break;
//		case 2:
//			strcpy(logLevelString, "WARN");
//			break;
//		case 3:
//			strcpy(logLevelString, "CRIT");
//			break;
//		default:
////			lilog::log(2, file, line, "Could not resolve log level: %i.", logLevel);
//			return false;
//	}
//
//	myLogFile = fopen(logFile, "a"); // Opens the log file
//
//	if (myLogFile == nullptr) // Check if the file is opened
//	{
//		std::cerr << "-!!!- CRITICAL ERROR -!!!-\nCan not open log file!\n";
//		return false;
//	}
//
//	strftime(dateTime, 22, "%H:%M:%S - %d/%m/%Y", localtime(&myTime)); // Creates date and time string for log file
//
//	fprintf(myLogFile, "%s :: %s :: File: %s (line: %u) :: ", logLevelString, dateTime, file, line); // Prints all needed info to the log file
//
//	va_start(arg, message); // Prints the message to the log file
//	vfprintf(myLogFile, message, arg);
//	va_end(arg);
//	fprintf(myLogFile, "\n"); // Prints a new line at the end
//
//	fclose(myLogFile); // Closes the log file
//
//	return true;
}

void lilog::clearLogFile()
{
	if (!myStream.is_open())
	{
		myStream.open(logFile);
	}
	std::ofstream ofs;
	ofs.open(logFile, std::ofstream::out | std::ofstream::trunc);
	ofs.close();
}

void lilog::closeLogFile()
{
	myStream.close();
}
