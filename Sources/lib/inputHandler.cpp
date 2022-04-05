/*
 * Created by Leander @ Lenny Industries on 05/04/2022.
 * Project: client.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

#include "include/inputHandler.hpp"

inputHandler::inputHandler(int &argc, char **argv)
{
	for (int i = 1; i < argc; i++)
	{
		this->tokens.emplace_back(argv[i]);
	}
}

const std::string &inputHandler::getCmdOption(const std::string &option)
const
{
	std::vector <std::string>::const_iterator itr;
	itr = std::find(this->tokens.begin(), this->tokens.end(), option);
	if (itr != this->tokens.end() && ++itr != this->tokens.end())
	{
		return *itr;
	}
	static const std::string empty_string;
	return empty_string;
}

bool inputHandler::cmdOptionExists(const std::string &option) const
{
	return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
}
