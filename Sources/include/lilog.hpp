/*
 * Created by Leander @ Lenny Industries on 22/03/2022.
 * Project: NA.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * lilog V2
 * Lenny Industries Log
 * Used for logging purposes
 */

#ifndef LENNYINDUSTRIES_LILOG_HPP
#define LENNYINDUSTRIES_LILOG_HPP

// Libraries
#include <cstdarg>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sys/stat.h>

// Definitions
// Macros
#define LOG(lilogName, logLevel, message, ...) lilogName->log(logLevel, __FILE__, __LINE__, message, ##__VA_ARGS__)

class lilog
{
public:
	// Con- Destructor
	static lilog *create(const std::string &logFile, bool createBackup, bool clear = false);
	void kill();
	// Functions
	bool log(char logLevel, std::string file, unsigned int line, const char *message, ...);
	void clearLogFile();
	void open();
	void close();
protected:
private:
	// Con- Destructor
	explicit lilog(const std::string &logFile, bool createBackup, bool clear);
	~lilog();
	// Variables
	std::string logFile;
	std::ofstream myStream;
};


#endif //LENNYINDUSTRIES_LILOG_HPP
