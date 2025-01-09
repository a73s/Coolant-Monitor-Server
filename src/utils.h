/*
Author: Adam Seals
*/

#pragma once

#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <curl/curl.h>
#include <vector>

const bool DEBUG = true;

//generates a random number in the range (inclusive)
unsigned int generateRand(const unsigned long & min, const unsigned long & max);

void mapToFile(std::ofstream & file, const std::map<uint32_t, std::string>);

bool sendText(CURL * const curl_handle, const std::string phone_num, const std::string message, const std::string api_key, const std::string sender);

bool multiSendText(CURL * const curl_handle, const std::vector<std::string> phone_nums, const std::string message, const std::string api_key, const std::string sender);
