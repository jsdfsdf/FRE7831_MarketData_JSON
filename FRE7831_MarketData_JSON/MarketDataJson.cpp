#include <stdio.h>

#include "json/json.h"
#include "curl/curl.h"
#include <sqlite3.h>

#include "Stock.h"
#include "Util.h"

int main(void)
{
	string sConfigFile = "config.csv";
	map<string, string> config_map = ProcessConfigData(sConfigFile);

    // get out put
    ofstream outfile("Results.txt", ios::trunc);

	string readBuffer;

	//global initiliation of curl before calling a function
	curl_global_init(CURL_GLOBAL_ALL);

	//creating session handle
	CURL * handle;

	// Store the result of CURL’s webpage retrieval, for simple error checking.
	CURLcode result;

	handle = curl_easy_init();
    
    if (!handle)
    {
        cout << "curl_easy_init failed" << endl;
        return -1;
    }

	string url_common = config_map["url_common"];
	string start_date = config_map["start_date"];
	string end_date = config_map["end_date"];
    string interval = config_map["interval"];
	string api_token = config_map["api_token"];
	string symbol = config_map["symbol"];
	//string url_request = url_common + symbol + ".US?" + "from=" + start_date + "&to=" + end_date + "&api_token=" + api_token + "&period=d&fmt=json";
    string url_request = url_common + symbol + ".US?" + "api_token=" + api_token + "&interval=" + interval + "&fmt=json";
    cout << url_request;
	curl_easy_setopt(handle, CURLOPT_URL, url_request.c_str());
	
	//adding a user agent
	curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:74.0) Gecko/20100101 Firefox/74.0");
	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);

	// send all data to this function 
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);

	// we pass our 'chunk' struct to the callback function 
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &readBuffer);

	//perform a blocking file transfer
	result = curl_easy_perform(handle);

	// check for errors 
	if (result != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
	}
	else
	{
		//json parsing
		Json::CharReaderBuilder builder;
		Json::CharReader* reader = builder.newCharReader();
		Json::Value root;   // will contains the root value after parsing.
		string errors;

		bool parsingSuccessful = reader->parse(readBuffer.c_str(), readBuffer.c_str() + readBuffer.size(), &root, &errors);
		if (not parsingSuccessful)
		{
			// Report failures and their locations in the document.
			cout << "Failed to parse JSON" << endl << readBuffer << errors << endl;
			return -1;
		}
		else
		{
			cout << "\nSucess parsing json\n" << root << endl;
			string date;
			float open, high, low, close, adjusted_close;
			long volume, timestamp;
            char* datetime;
			Stock myStock(symbol);
			for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++) 
			{
				date = (*itr)["datetime"].asString();
				open = (*itr)["open"].asFloat();
				high = (*itr)["high"].asFloat();
				low = (*itr)["low"].asFloat();
				close = (*itr)["close"].asFloat();
				//adjusted_close = (*itr)["adjusted_close"].asFloat();
				volume = (*itr)["volume"].asInt64();
                timestamp = (*itr)["timestamp"].asInt64();
                time_t t = timestamp;
                struct tm* tm = localtime(&t);
                datetime = new char;
                sprintf(datetime, "%s", asctime(tm));
                //cout << timestamp;
                //cout << datetime;
                IntradayTrade aTrade(timestamp, datetime, open, high, low, close, volume);
				myStock.addTrade(aTrade);
			}
			//cout << myStock;
            outfile << myStock;
            outfile.close();
		}
	}
	curl_easy_cleanup(handle);
	return 0;
}

//int main(void)
//{
//	string sConfigFile = "config.csv";
//	map<string, string> config_map = ProcessConfigData(sConfigFile);
//
//	string readBuffer;
//
//	//global initiliation of curl before calling a function
//	curl_global_init(CURL_GLOBAL_ALL);
//
//	//creating session handle
//	CURL* handle;
//
//	// Store the result of CURL’s webpage retrieval, for simple error checking.
//	CURLcode result;
//
//	handle = curl_easy_init();
//
//	if (!handle)
//	{
//		cout << "curl_easy_init failed" << endl;
//		return -1;
//	}
//
//	string url_common = config_map["url_common"];
//	string start_date = config_map["start_date"];
//	string end_date = config_map["end_date"];
//	string api_token = config_map["api_token"];
//	string symbol = config_map["symbol"];
//	string url_request = url_common + symbol + ".US?" + "from=" + start_date + "&to=" + end_date + "&api_token=" + api_token + "&period=d&fmt=json";
//	curl_easy_setopt(handle, CURLOPT_URL, url_request.c_str());
//
//	//adding a user agent
//	curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:74.0) Gecko/20100101 Firefox/74.0");
//	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
//	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
//
//	// send all data to this function 
//	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteCallback);
//
//	// we pass our 'chunk' struct to the callback function 
//	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &readBuffer);
//
//	//perform a blocking file transfer
//	result = curl_easy_perform(handle);
//
//	// check for errors 
//	if (result != CURLE_OK) {
//		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(result));
//	}
//	else
//	{
//		//json parsing
//		Json::CharReaderBuilder builder;
//		Json::CharReader* reader = builder.newCharReader();
//		Json::Value root;   // will contains the root value after parsing.
//		string errors;
//
//		bool parsingSuccessful = reader->parse(readBuffer.c_str(), readBuffer.c_str() + readBuffer.size(), &root, &errors);
//		if (not parsingSuccessful)
//		{
//			// Report failures and their locations in the document.
//			cout << "Failed to parse JSON" << endl << readBuffer << errors << endl;
//			return -1;
//		}
//		else
//		{
//			cout << "\nSucess parsing json\n" << root << endl;
//			string date;
//			float open, high, low, close, adjusted_close;
//			long volume;
//			Stock myStock(symbol);
//			for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++)
//			{
//				date = (*itr)["date"].asString();
//				open = (*itr)["open"].asFloat();
//				high = (*itr)["high"].asFloat();
//				low = (*itr)["low"].asFloat();
//				close = (*itr)["close"].asFloat();
//				adjusted_close = (*itr)["adjusted_close"].asFloat();
//				volume = (*itr)["volume"].asInt64();
//				DailyTrade aTrade(date, open, high, low, close, adjusted_close, volume);
//				myStock.addTrade(aTrade);
//			}
//			cout << myStock;
//		}
//	}
//	curl_easy_cleanup(handle);
//	return 0;
//}

/*
Sucess parsing json
[
{
    "adjusted_close" : 334.75,
        "close" : 334.75,
        "date" : "2022-01-03",
        "high" : 338,
        "low" : 329.77999999999997,
        "open" : 335.35000000000002,
        "volume" : 28865100
},
        {
                "adjusted_close" : 329.00999999999999,
                "close" : 329.00999999999999,
                "date" : "2022-01-04",
                "high" : 335.19999999999999,
                "low" : 326.12,
                "open" : 334.82999999999998,
                "volume" : 32674300
        },
        {
                "adjusted_close" : 316.38,
                "close" : 316.38,
                "date" : "2022-01-05",
                "high" : 326.06999999999999,
                "low" : 315.98000000000002,
                "open" : 325.86000000000001,
                "volume" : 40054300
        },
        {
                "adjusted_close" : 313.88,
                "close" : 313.88,
                "date" : "2022-01-06",
                "high" : 318.69999999999999,
                "low" : 311.49000000000001,
                "open" : 313.14999999999998,
                "volume" : 39646100
        },
        {
                "adjusted_close" : 314.04000000000002,
                "close" : 314.04000000000002,
                "date" : "2022-01-07",
                "high" : 316.5,
                "low" : 310.08999999999997,
                "open" : 314.14999999999998,
                "volume" : 32674000
        },
        {
                "adjusted_close" : 314.26999999999998,
                "close" : 314.26999999999998,
                "date" : "2022-01-10",
                "high" : 314.72000000000003,
                "low" : 304.69,
                "open" : 309.49000000000001,
                "volume" : 44289500
        },
        {
                "adjusted_close" : 314.98000000000002,
                "close" : 314.98000000000002,
                "date" : "2022-01-11",
                "high" : 316.61000000000001,
                "low" : 309.88999999999999,
                "open" : 313.38,
                "volume" : 29386800
        },
        {
                "adjusted_close" : 318.26999999999998,
                "close" : 318.26999999999998,
                "date" : "2022-01-12",
                "high" : 323.41000000000003,
                "low" : 317.07999999999998,
                "open" : 319.67000000000002,
                "volume" : 34372200
        },
        {
                "adjusted_close" : 304.80000000000001,
                "close" : 304.80000000000001,
                "date" : "2022-01-13",
                "high" : 320.88,
                "low" : 304,
                "open" : 320.47000000000003,
                "volume" : 45366000
        },
        {
                "adjusted_close" : 310.19999999999999,
                "close" : 310.19999999999999,
                "date" : "2022-01-14",
                "high" : 310.81999999999999,
                "low" : 303.75,
                "open" : 304.25,
                "volume" : 39823500
        },
        {
                "adjusted_close" : 302.64999999999998,
                "close" : 302.64999999999998,
                "date" : "2022-01-18",
                "high" : 309.80000000000001,
                "low" : 301.74000000000001,
                "open" : 304.06999999999999,
                "volume" : 42333200
        },
        {
                "adjusted_close" : 303.32999999999998,
                "close" : 303.32999999999998,
                "date" : "2022-01-19",
                "high" : 313.91000000000003,
                "low" : 302.69999999999999,
                "open" : 306.29000000000002,
                "volume" : 45933900
        },
        {
                "adjusted_close" : 301.60000000000002,
                "close" : 301.60000000000002,
                "date" : "2022-01-20",
                "high" : 311.64999999999998,
                "low" : 301.13999999999999,
                "open" : 309.06999999999999,
                "volume" : 35380700
        },
        {
                "adjusted_close" : 296.02999999999997,
                "close" : 296.02999999999997,
                "date" : "2022-01-21",
                "high" : 304.11000000000001,
                "low" : 295.61000000000001,
                "open" : 302.69,
                "volume" : 57118300
        }
]
Symbol: MSFT
Date : 2022 - 01 - 03 Open : 335.35 High : 338 Low : 329.78 Close : 334.75 Adjusted_Close : 334.75 Volume : 28865100
Date : 2022 - 01 - 04 Open : 334.83 High : 335.2 Low : 326.12 Close : 329.01 Adjusted_Close : 329.01 Volume : 32674300
Date : 2022 - 01 - 05 Open : 325.86 High : 326.07 Low : 315.98 Close : 316.38 Adjusted_Close : 316.38 Volume : 40054300
Date : 2022 - 01 - 06 Open : 313.15 High : 318.7 Low : 311.49 Close : 313.88 Adjusted_Close : 313.88 Volume : 39646100
Date : 2022 - 01 - 07 Open : 314.15 High : 316.5 Low : 310.09 Close : 314.04 Adjusted_Close : 314.04 Volume : 32674000
Date : 2022 - 01 - 10 Open : 309.49 High : 314.72 Low : 304.69 Close : 314.27 Adjusted_Close : 314.27 Volume : 44289500
Date : 2022 - 01 - 11 Open : 313.38 High : 316.61 Low : 309.89 Close : 314.98 Adjusted_Close : 314.98 Volume : 29386800
Date : 2022 - 01 - 12 Open : 319.67 High : 323.41 Low : 317.08 Close : 318.27 Adjusted_Close : 318.27 Volume : 34372200
Date : 2022 - 01 - 13 Open : 320.47 High : 320.88 Low : 304 Close : 304.8 Adjusted_Close : 304.8 Volume : 45366000
Date : 2022 - 01 - 14 Open : 304.25 High : 310.82 Low : 303.75 Close : 310.2 Adjusted_Close : 310.2 Volume : 39823500
Date : 2022 - 01 - 18 Open : 304.07 High : 309.8 Low : 301.74 Close : 302.65 Adjusted_Close : 302.65 Volume : 42333200
Date : 2022 - 01 - 19 Open : 306.29 High : 313.91 Low : 302.7 Close : 303.33 Adjusted_Close : 303.33 Volume : 45933900
Date : 2022 - 01 - 20 Open : 309.07 High : 311.65 Low : 301.14 Close : 301.6 Adjusted_Close : 301.6 Volume : 35380700
Date : 2022 - 01 - 21 Open : 302.69 High : 304.11 Low : 295.61 Close : 296.03 Adjusted_Close : 296.03 Volume : 57118300

*/