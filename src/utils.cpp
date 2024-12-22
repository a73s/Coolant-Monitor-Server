/*
Author: Adam Seals
*/

#include <cstdlib>
#include <curl/curl.h>

#include "utils.h"

unsigned int generateRand(const unsigned long & min, const unsigned long & max){

	return (rand()%(max-min+1)+min);
}

void mapToFile(std::ofstream & file, const std::map<uint32_t, std::string> map){

	for(auto i = map.begin(); i != map.end(); i++){

		file << i->first << ':' << i->second << '\n';
	}
}

bool sendtext(CURL * curl_handle, std::string phone_num, std::string message, std::string api_key, std::string sender){

	const std::string json = "{\"phone\":\"" + phone_num + "\",\"message\":\"" + message + "\",\"key\":\"" + api_key + "\",\"sender\":\"" + sender + "\"}";

	struct curl_slist *slist1 = NULL;

	slist1 = curl_slist_append(slist1, "Content-Type: application/json");
	slist1 = curl_slist_append(slist1, "Accept: application/json");
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(curl_handle, CURLOPT_URL, "https://textbelt.com/text");
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, json.c_str());
	CURLcode ret = curl_easy_perform(curl_handle);

	return true;
}
