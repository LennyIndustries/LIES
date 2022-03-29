/*
 * Created by Leander @ Lenny Industries on 22/03/2022.
 * Project: NA.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * lilog
 * Lenny Industries Log
 * Used for logging purposes
 */

#ifndef MAIN_LILOG_HPP
#define MAIN_LILOG_HPP

// Libraries
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstdarg>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <unistd.h>
#include <utility>

// Definitions
// Macros
#define LOG(lilogName, logLevel, message, ...) lilogName.log(logLevel, __FILE__, __LINE__, message, ##__VA_ARGS__)

// Functions

class lilog
{
public:
	explicit lilog(const std::string &logFile);
	bool log(char logLevel, std::string file, unsigned int line, const char *message, ...);
	void clearLogFile();
	void kill();
	inline void close() { kill(); } // Separate kill and close
	~lilog();
protected:
private:
	std::string logFile;
	std::ofstream myStream;
};


#endif //MAIN_LILOG_HPP
