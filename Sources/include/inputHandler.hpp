/*
 * Created by Leander @ Lenny Industries on 05/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * CMD input handling courtesy of https://stackoverflow.com/a/868894/4620857
 */

#ifndef LIES_INPUTHANDLER_HPP
#define LIES_INPUTHANDLER_HPP

// Libraries
#include <string>
#include <vector>
#include <algorithm>

// Definitions

class inputHandler
{
public:
	inputHandler(int &argc, char **argv);
	[[nodiscard]] const std::string &getCmdOption(const std::string &option) const;
	[[nodiscard]] bool cmdOptionExists(const std::string &option) const;
protected:
private:
	std::vector<std::string> tokens;
};


#endif //LIES_INPUTHANDLER_HPP
