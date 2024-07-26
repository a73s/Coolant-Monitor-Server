/*
Author: Adam Seals
*/

#include <cstdlib>

#include "utils.h"

unsigned int generateRand(const unsigned long & min, const unsigned long & max){

	return (rand()%(max-min+1)+min);
}

