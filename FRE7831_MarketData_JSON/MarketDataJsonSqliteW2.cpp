#include <stdio.h>

#include "json/json.h"
#include "curl/curl.h"

#include "Stock.h"
#include "Util.h"
#include "Database.h"




int main1(void)
{
    string database_name = "Stocks.db";
    cout << "Opening Stocks.db ..." << endl;
    sqlite3* db = NULL;
    if (OpenDatabase(database_name.c_str(), db) == -1)
        return -1;

    // Drop the table if exists
    cout << "Drop DailyTrades table if exists" << endl;
    string sql_DropaTable = "DROP TABLE IF EXISTS DailyTrades";
    if (DropTable(db, sql_DropaTable.c_str()) == -1)
        return -1;

    // Create the table
    cout << "Creating DailyTrades table ..." << endl;
    const char* sql_CreateTable = "CREATE TABLE DailyTrades (" \
        "symbol CHAR(20) NOT NULL," \
        "date CHAR(20) NOT NULL," \
        "open REAL NOT NULL," \
        "high REAL NOT NULL," \
        "low REAL NOT NULL," \
        "close REAL NOT NULL," \
        "adjusted_close REAL NOT NULL," \
        "volume INT NOT NULL," \
        "PRIMARY KEY(symbol, date));";

    if (ExecuteSQL(db, sql_CreateTable) == -1)
        return -1;

	string sConfigFile = "config.csv";
	map<string, string> config_map = ProcessConfigData(sConfigFile);

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
	string api_token = config_map["api_token"];
	string symbol = config_map["symbol"];
	string url_request = url_common + symbol + ".US?" + "from=" + start_date + "&to=" + end_date + "&api_token=" + api_token + "&period=d&fmt=json";
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
			long volume;
			Stock myStock(symbol);
			for (Json::Value::const_iterator itr = root.begin(); itr != root.end(); itr++) 
			{
				date = (*itr)["date"].asString();
				open = (*itr)["open"].asFloat();
				high = (*itr)["high"].asFloat();
				low = (*itr)["low"].asFloat();
				close = (*itr)["close"].asFloat();
				adjusted_close = (*itr)["adjusted_close"].asFloat();
				volume = (*itr)["volume"].asInt64();
				DailyTrade aTrade(date, open, high, low, close, adjusted_close, volume);
				myStock.addTrade(aTrade);

				cout << aTrade << endl;
				cout << "Inserting daily data for a stock into table DailyTrades ..." << endl << endl;
				char sql_Insert[512];
				sprintf_s(sql_Insert, "INSERT INTO DailyTrades(symbol, date, open, high, low, close, adjusted_close, volume) VALUES(\"%s\", \"%s\", %f, %f, %f, %f, %f, %d)", symbol.c_str(), date.c_str(), open, high, low, close, adjusted_close, volume);
				if (ExecuteSQL(db, sql_Insert) == -1)
					return -1;
			} 
		}
	}
	curl_easy_cleanup(handle);
	// Display DailyTrades Table
	cout << "Retrieving values in table DailyTrades ..." << endl;
	const char* sql_Select = "SELECT * FROM DailyTrades;";
	if (ShowTable(db, sql_Select) == -1)
		return -1;
	return 0;
}

/*
Opening Stocks.db ...
Opened Stocks.db
Drop DailyTrades table if exists
Creating DailyTrades table ...

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
        },
        {
                "adjusted_close" : 296.37,
                "close" : 296.37,
                "date" : "2022-01-24",
                "high" : 297.11000000000001,
                "low" : 276.05000000000001,
                "open" : 292.19999999999999,
                "volume" : 86035400
        },
        {
                "adjusted_close" : 288.49000000000001,
                "close" : 288.49000000000001,
                "date" : "2022-01-25",
                "high" : 294.99000000000001,
                "low" : 285.17000000000002,
                "open" : 291.51999999999998,
                "volume" : 72848600
        },
        {
                "adjusted_close" : 296.70999999999998,
                "close" : 296.70999999999998,
                "date" : "2022-01-26",
                "high" : 308.5,
                "low" : 293.02999999999997,
                "open" : 307.99000000000001,
                "volume" : 90428900
        },
        {
                "adjusted_close" : 299.83999999999997,
                "close" : 299.83999999999997,
                "date" : "2022-01-27",
                "high" : 307.30000000000001,
                "low" : 297.93000000000001,
                "open" : 302.66000000000003,
                "volume" : 53376900
        },
        {
                "adjusted_close" : 308.25999999999999,
                "close" : 308.25999999999999,
                "date" : "2022-01-28",
                "high" : 308.48000000000002,
                "low" : 294.44999999999999,
                "open" : 300.23000000000002,
                "volume" : 49717000
        },
        {
                "adjusted_close" : 310.98000000000002,
                "close" : 310.98000000000002,
                "date" : "2022-01-31",
                "high" : 312.37,
                "low" : 306.37,
                "open" : 308.94999999999999,
                "volume" : 45566415
        }
]
Date: 2022-01-03 Open: 335.35 High: 338 Low: 329.78 Close: 334.75 Adjusted_Close: 334.75 Volume: 28865100

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-04 Open: 334.83 High: 335.2 Low: 326.12 Close: 329.01 Adjusted_Close: 329.01 Volume: 32674300

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-05 Open: 325.86 High: 326.07 Low: 315.98 Close: 316.38 Adjusted_Close: 316.38 Volume: 40054300

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-06 Open: 313.15 High: 318.7 Low: 311.49 Close: 313.88 Adjusted_Close: 313.88 Volume: 39646100

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-07 Open: 314.15 High: 316.5 Low: 310.09 Close: 314.04 Adjusted_Close: 314.04 Volume: 32674000

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-10 Open: 309.49 High: 314.72 Low: 304.69 Close: 314.27 Adjusted_Close: 314.27 Volume: 44289500

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-11 Open: 313.38 High: 316.61 Low: 309.89 Close: 314.98 Adjusted_Close: 314.98 Volume: 29386800

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-12 Open: 319.67 High: 323.41 Low: 317.08 Close: 318.27 Adjusted_Close: 318.27 Volume: 34372200

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-13 Open: 320.47 High: 320.88 Low: 304 Close: 304.8 Adjusted_Close: 304.8 Volume: 45366000

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-14 Open: 304.25 High: 310.82 Low: 303.75 Close: 310.2 Adjusted_Close: 310.2 Volume: 39823500

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-18 Open: 304.07 High: 309.8 Low: 301.74 Close: 302.65 Adjusted_Close: 302.65 Volume: 42333200

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-19 Open: 306.29 High: 313.91 Low: 302.7 Close: 303.33 Adjusted_Close: 303.33 Volume: 45933900

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-20 Open: 309.07 High: 311.65 Low: 301.14 Close: 301.6 Adjusted_Close: 301.6 Volume: 35380700

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-21 Open: 302.69 High: 304.11 Low: 295.61 Close: 296.03 Adjusted_Close: 296.03 Volume: 57118300

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-24 Open: 292.2 High: 297.11 Low: 276.05 Close: 296.37 Adjusted_Close: 296.37 Volume: 86035400

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-25 Open: 291.52 High: 294.99 Low: 285.17 Close: 288.49 Adjusted_Close: 288.49 Volume: 72848600

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-26 Open: 307.99 High: 308.5 Low: 293.03 Close: 296.71 Adjusted_Close: 296.71 Volume: 90428900

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-27 Open: 302.66 High: 307.3 Low: 297.93 Close: 299.84 Adjusted_Close: 299.84 Volume: 53376900

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-28 Open: 300.23 High: 308.48 Low: 294.45 Close: 308.26 Adjusted_Close: 308.26 Volume: 49717000

Inserting daily data for a stock into table DailyTrades ...

Date: 2022-01-31 Open: 308.95 High: 312.37 Low: 306.37 Close: 310.98 Adjusted_Close: 310.98 Volume: 45566415

Inserting daily data for a stock into table DailyTrades ...

Retrieving values in table DailyTrades ...
symbol       date         open         high         low          close        adjusted_close volume
~~~~~~~~~~~~ ~~~~~~~~~~~~ ~~~~~~~~~~~~ ~~~~~~~~~~~~ ~~~~~~~~~~~~ ~~~~~~~~~~~~ ~~~~~~~~~~~~ ~~~~~~~~~~~~
MSFT         2022-01-03   335.350006   338.0        329.779999   334.75       334.75       28865100
MSFT         2022-01-04   334.829987   335.200012   326.119995   329.01001    329.01001    32674300
MSFT         2022-01-05   325.859985   326.070007   315.980011   316.380005   316.380005   40054300
MSFT         2022-01-06   313.149994   318.700012   311.48999    313.880005   313.880005   39646100
MSFT         2022-01-07   314.149994   316.5        310.089996   314.040009   314.040009   32674000
MSFT         2022-01-10   309.48999    314.720001   304.690002   314.269989   314.269989   44289500
MSFT         2022-01-11   313.380005   316.609985   309.890015   314.980011   314.980011   29386800
MSFT         2022-01-12   319.670013   323.410004   317.079987   318.269989   318.269989   34372200
MSFT         2022-01-13   320.470001   320.880005   304.0        304.799988   304.799988   45366000
MSFT         2022-01-14   304.25       310.820007   303.75       310.200012   310.200012   39823500
MSFT         2022-01-18   304.070007   309.799988   301.73999    302.649994   302.649994   42333200
MSFT         2022-01-19   306.290009   313.910004   302.700012   303.329987   303.329987   45933900
MSFT         2022-01-20   309.070007   311.649994   301.140015   301.600006   301.600006   35380700
MSFT         2022-01-21   302.690002   304.109985   295.609985   296.029999   296.029999   57118300
MSFT         2022-01-24   292.200012   297.109985   276.049988   296.369995   296.369995   86035400
MSFT         2022-01-25   291.519989   294.98999    285.170013   288.48999    288.48999    72848600
MSFT         2022-01-26   307.98999    308.5        293.029999   296.709991   296.709991   90428900
MSFT         2022-01-27   302.660004   307.299988   297.929993   299.839996   299.839996   53376900
MSFT         2022-01-28   300.230011   308.480011   294.450012   308.26001    308.26001    49717000
MSFT         2022-01-31   308.950012   312.369995   306.369995   310.980011   310.980011   45566415

C:\Users\Song\Dropbox\PolyNYU\PolyNYU2022\FRE7831\FRE7831_MarketData\x64\Debug\FRE7831_MarketData_JSON_Sqlite.exe (process 129860) exited with code 0.
Press any key to close this window . . .
*/