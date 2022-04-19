/*
 * Created by Leander @ Lenny Industries on 19/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * Handles new incoming connections
 */

#ifndef LIES_CONNECTIONHANDLER_HPP
#define LIES_CONNECTIONHANDLER_HPP

// Libraries
#include <string>

// Definitions

class connectionHandler
{
public:
	connectionHandler(const std::string& function, const std::string& message, unsigned int uuid);
	[[nodiscard]] unsigned int getUUID() const;
protected:
private:
	unsigned int uuid;
};


#endif //LIES_CONNECTIONHANDLER_HPP
