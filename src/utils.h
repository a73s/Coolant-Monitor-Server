/*
Author: Adam Seals
*/

#pragma once

const bool DEBUG = true;

//description: generates a random number in the range (inclusive)
//pre:input 2 integers, otherwise will set to default, srand is assumed to be pre-seeded in main
//post: returns the random number within the parameters
unsigned int generateRand(const unsigned long & min, const unsigned long & max);
