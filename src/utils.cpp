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

bool sendText(CURL * const curl_handle, const std::string phone_num, const std::string message, const std::string api_key, const std::string sender){

	const std::string json = "{\"phone\":\"" + phone_num + "\",\"message\":\"" + message + "\",\"key\":\"" + api_key + "\",\"sender\":\"" + sender + "\"}";

	struct curl_slist *slist1 = NULL;

	slist1 = curl_slist_append(slist1, "Content-Type: application/json");
	slist1 = curl_slist_append(slist1, "Accept: application/json");
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(curl_handle, CURLOPT_URL, "https://textbelt.com/text");
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, json.c_str());
	curl_easy_perform(curl_handle);

	return true;
}

bool multiSendText(CURL * const curl_handle, const std::vector<std::string> phone_nums, const std::string message, const std::string api_key, const std::string sender){

	bool ret = true;

	for(size_t i = 0; i < phone_nums.size(); i++){
		bool tmp = sendText(curl_handle, phone_nums[i], message, api_key, sender);
		if(!tmp){
			ret = false;
		}
	}

	return ret;
}
