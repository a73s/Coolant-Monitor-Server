/*
Author: Adam Seals
*/

#include <cstdlib>

#include "utils.h"

unsigned int generateRand(const unsigned long & min, const unsigned long & max){

	return (rand()%(max-min+1)+min);
}

void mapToFile(std::ofstream & file, const std::map<uint32_t, std::string> map){

	for(auto i = map.begin(); i != map.end(); i++){

		file << i->first << ':' << i->second << '\n';
	}
}
