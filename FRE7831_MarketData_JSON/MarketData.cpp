#include <string>
#include "MarketData.h"
#include "curl/curl.h"
#include <iostream>
#include "Util.h"
#include "json/json.h"
#include "Stock.h"
#include <map>
#include <string>
#include <vector>
#include<fstream>
class Stock;

// every thing before decode json, target get buffer
int PullMarketData(const std::string& url_request, std::string&
	read_buffer) {

	//global initiliation of curl before calling a function
	curl_global_init(CURL_GLOBAL_ALL);

	//creating session handle
	CURL* handle;

	// Store the result of CURL’s webpage retrieval, for simple error checking.
	CURLcode result;

	handle = curl_easy_init();

	if (!handle)
	{
		std::cout << "curl_easy_init failed" << std::endl;
		return -1;
	}

	curl_easy_setopt(handle, CURLOPT_URL, url_request.c_str());

	//adding a user agent
	curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:74.0) Gecko/20100101 Firefox/74.0");
	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);

	// send all data to this function 
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);

	// we pass our 'chunk' struct to the callback function 
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &read_buffer);

	//perform a blocking file transfer
	result = curl_easy_perform(handle);

	// check for errors 
	if (result != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
		return -1;
	}
	curl_easy_cleanup(handle);// test see 
	return 0;
}

// feed data to stock but not insert sql yet
int PopulateDailyTrades(const std::string& read_buffer,
	Stock& stock) {

	//json parsing
	Json::CharReaderBuilder builder;
	Json::CharReader* reader = builder.newCharReader();
	Json::Value root;   // will contains the root value after parsing.
	string errors;

	bool parsingSuccessful = reader->parse(read_buffer.c_str(), read_buffer.c_str() + read_buffer.size(), &root, &errors);
	if (not parsingSuccessful)
	{
		// Report failures and their locations in the document.
		cout << "Failed to parse JSON" << endl << read_buffer << errors << endl;
		return -1;
	}
	else
	{
		cout << "\nSucess parsing json\n" << root << endl;		}
	
		string date;
		float open, high, low, close, adjusted_close;
		long volume;
		for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++)
			{
				date = (*itr)["date"].asString();
				//open = round((*itr)["open"].asFloat() * 100) / 100;
				open = (*itr)["open"].asFloat();
				high = (*itr)["high"].asFloat();
				low = (*itr)["low"].asFloat();
				close = (*itr)["close"].asFloat();
				adjusted_close = (*itr)["adjusted_close"].asFloat();
				volume = (*itr)["volume"].asInt64();
				//DailyTrade aTrade(date, open, high, low, close, adjusted_close, volume);
				TradeData aTrade(date, open, high, low, close, adjusted_close, volume);
				stock.addTrade(aTrade);
				cout << aTrade << endl;

			}
		return 0;
}



