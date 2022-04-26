/*
 * Created by Leander @ Lenny Industries on 16/04/2022.
 * Project: LIES.
 * Copyright (c) 2022 Lenny Industries. All rights reserved.
 */

/*
 * Helps out with encryption and decryption
 */

#ifndef LIES_CRYPTLIB_HPP
#define LIES_CRYPTLIB_HPP

// Libraries
#include <cstdio>
#include <iostream>
#include <cstring>
// Definitions

class cryptLib
{
public:
protected:
	static char getImageData(char *imageP, char **headerReturn, char **dataReturn);
private:
};


#endif //LIES_CRYPTLIB_HPP
