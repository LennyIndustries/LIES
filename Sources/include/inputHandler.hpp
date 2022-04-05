/*
 * Created by Leander @ Lenny Industries on 05/04/2022.
 * Project: client.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * CMD input handling courtesy of https://stackoverflow.com/a/868894/4620857
 */

#ifndef CLIENT_INPUTHANDLER_HPP
#define CLIENT_INPUTHANDLER_HPP

// Libraries
#include <string>
#include <vector>
#include <algorithm>

// Definition
// Functions

class inputHandler
{
public:
	inputHandler(int &argc, char **argv);
	[[nodiscard]] const std::string &getCmdOption(const std::string &option) const;
	[[nodiscard]] bool cmdOptionExists(const std::string &option) const;
protected:
private:
	std::vector <std::string> tokens;
};


#endif //CLIENT_INPUTHANDLER_HPP
