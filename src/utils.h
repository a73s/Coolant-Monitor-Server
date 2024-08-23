/*
Author: Adam Seals
*/

#pragma once

#include <cstdint>
#include <fstream>
#include <map>
#include <string>

const bool DEBUG = true;

//description: generates a random number in the range (inclusive)
//pre:input 2 integers, otherwise will set to default, srand is assumed to be pre-seeded in main
//post: returns the random number within the parameters
unsigned int generateRand(const unsigned long & min, const unsigned long & max);

void mapToFile(std::ofstream & file, const std::map<uint32_t, std::string>);
