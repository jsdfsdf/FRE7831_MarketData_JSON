#include "Stock.h"
#include "MarketData.h"
#include "Database.h"
#include "Util.h"
#include <map>
#include <string>
#include <stdio.h>
#include "json/json.h"
#include "curl/curl.h"

// to do 
// 1 get api
// 2 do it in a map way

hash_t hash_(char const* str)
{
	hash_t ret{ basis };

	while (*str) {
		ret ^= *str;
		ret *= prime;
		str++;
	}

	return ret;
}

constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)
{
	return *str ? hash_compile_time(str + 1, (*str ^ last_value) * prime) : last_value;
}

constexpr unsigned long long operator "" _hash(char const* p, size_t)
{
	return hash_compile_time(p);
}


int optionA(const char* sql_CreateTable, string tableName) {
	string database_name = "MarketData.db";
	cout << "Opening MarketData.db ..." << endl;
	sqlite3* db = NULL;
	if (OpenDatabase(database_name.c_str(), db) == -1)
		return -1;

	// Drop the table if exists
	cout << "Drop " << tableName << "if exists" << endl;
	string sql_DropaTable = "DROP TABLE IF EXISTS " + tableName;
	if (DropTable(db, sql_DropaTable.c_str()) == -1)
		return -1;

	// Create the table
	cout << "Creating " << tableName << "table ..." << endl;
	if (ExecuteSQL(db, sql_CreateTable) == -1)
		return -1;

	return 1;
}


int main(void)
{
	sqlite3* db = NULL;
	const char* dataBaseName = "MarketData.db";
	if (OpenDatabase(dataBaseName,db) != 0) return -1;
	bool bCompleted = false;
	char selection;
	string sConfigFile = "config.csv";
	map<string, string> config_map = ProcessConfigData(sConfigFile);
	string symbol = config_map["symbol"];
	//cout << symbol;
	vector<string> symbols = split(symbol, ',');
	for (auto i = symbols.begin(); i != symbols.end(); ++i) {
		cout << *i << endl;
	}
	//vector<string> symbols;
	//symbols.push_back("MSFT");
	//symbols.push_back("TWTR");
	//symbols.push_back("IBM");
	map<string, Stock> stockMap;
	string daily_symbol = "MCD";
	string intraday_symbol = "AAPL";
	string fakeSymbolTest = "IBM";
	while (!bCompleted)
	{   std::cout << endl;
	    std::cout << "Menu" << endl;
		std::cout << "========" << endl;
		std::cout << "A - Create DailyTrades and Intraday Trades Tables" << endl;
		std::cout << "B - Retrieve Daily Trade and Intraday Trade Data" << endl; // stock
		std::cout << "C - Populate DailyTrades and IntradayTrades Table" << endl;
		std::cout << "D - Retrieve Data from DailyTrades and IntradayTrades Tables" << endl;
		std::cout << "X - Exit" << endl << endl;
		char* selection;
		selection = new char;
		std::cin >> selection;
		switch (hash_(selection))
		{
		case "a"_hash:
		case "A"_hash:
		{
			const char* sql_CreateTable = "CREATE TABLE IF NOT EXISTS DailyTrades(" \
				"symbol CHAR(20) NOT NULL," \
				"Date CHAR(20) NOT NULL," \
				"Open REAL NOT NULL," \
				"High REAL NOT NULL," \
				"Low REAL NOT NULL," \
				"Close REAL NOT NULL," \
				"Adjusted_close REAL NOT NULL," \
				"Volume INT NOT NULL," \
				"PRIMARY KEY(symbol, date),"
				"Foreign Key(Symbol) references Stocks(Symbol)\n"
				"ON DELETE CASCADE\n"
				"ON UPDATE CASCADE"
				");";

			int resultNum = optionA(sql_CreateTable, "DailyTrades");

			if (resultNum != 1) {
				return resultNum;
			}

			sql_CreateTable = "CREATE TABLE IF NOT EXISTS IntradayTrades "
							 "(Symbol CHAR(20) NOT NULL,"
							 "Date CHAR(20) NOT NULL,"
							 "Timestamp CHAR(20) NOT NULL,"
							 "Open REAL NOT NULL,"
							 "High REAL NOT NULL,"
							 "Low REAL NOT NULL,"
							 "Close REAL NOT NULL,"
							 "Volume INT NOT NULL,"
							 "PRIMARY KEY(Symbol, Date, Timestamp),"
							 "Foreign Key(Symbol, Date) references DailyTrades(Symbol, Date)\n"
							 "ON DELETE CASCADE\n"
							 "ON UPDATE CASCADE\n"
							 ");";
			resultNum = optionA(sql_CreateTable, "IntradayTrades");

			if (resultNum != 1) {
				return resultNum;
			}

			break;
		}
		case "b"_hash:
		case "B"_hash:
		{
			//build a stock map

			// potential need more

			string readBuffer, readBuffer_Intra;
			string daily_url_common = config_map["daily_url_common"];
			string intraday_url_common = config_map["intraday_url_common"];
			string start_date = config_map["start_date"];
			string end_date = config_map["end_date"];
			string api_token = config_map["api_token"];
			//string from = config_map["from"];
			//string to = config_map["to"];
			// we have not yet use api so manually set symbol
			//string symbol = config_map["symbol"];


			//string url_request = daily_url_common + daily_symbol + ".US?" + "from=" + start_date + "&to=" + end_date + "&api_token=" + api_token + "&period=d&fmt=json";
			//string url_request_intra = intraday_url_common + intraday_symbol + ".US?" + "&api_token=" + api_token + "&interval=5m&fmt=json";
			//if (PullMarketData(url_request.c_str(), readBuffer) != 0)
			//	return -1;
			//else
			//{
			//	Stock myStock(daily_symbol);
			//	if (PopulateDailyTrades(readBuffer, myStock) != 0)
			//		return -1;
			//	stockMap[daily_symbol] = myStock;
			//}

			//if (PullMarketData(url_request_intra.c_str(), readBuffer_Intra) != 0)
			//	return -1;
			//else
			//{
			//	Stock myStock(intraday_symbol);
			//	long Jan_1_2022 = 1641013200;
			//	if (PopulateIntradayTrades(readBuffer_Intra, myStock, Jan_1_2022) != 0)
			//		return -1;
			//	stockMap[intraday_symbol] = myStock;
			//}

			//vector<string> symbols;
			// the stock map version  reassig readBuffer, not sure it will work reassgin mystock not sure too
			long Jan_1_2022 = 1641013200;
			for (auto i = symbols.begin(); i != symbols.end(); ++i) {
				cout << *i;
				Stock myStock(*i);
				// do daily
				string url_request = daily_url_common + *i + ".US?" + "from=" + start_date + "&to=" + end_date + "&api_token=" + api_token + "&period=d&fmt=json";
				if (PullMarketData(url_request.c_str(), readBuffer) != 0)
					return -1;
				else
				{
					if (PopulateDailyTrades(readBuffer, myStock) != 0)
						return -1;
					//stockMap[*i] = myStock; later on do it
				}
				cout << "to do intra";
				// do intra
				readBuffer.clear();
				string url_request_intra = intraday_url_common + *i + ".US?" + "&api_token=" + api_token + "&interval=5m&fmt=json";
				if (PullMarketData(url_request_intra.c_str(), readBuffer) != 0)
					return -1;
				else
				{
					if (PopulateIntradayTrades(readBuffer, myStock, Jan_1_2022) != 0)
						return -1;
					stockMap[*i] = myStock;
				}
				readBuffer.clear();
				// destruct that maybe?
			}
			break;
		}
		case "c"_hash:
		case "C"_hash:
		// get Trade from stock, iterate through stock map
		// GetDailyTrade(void)  GetIntradayTrade(void) 
		{
			//const vector<DailyTrade> dailyTrades = stockMap[daily_symbol].GetDailyTrade();
			//for (auto i = dailyTrades.begin(); i != dailyTrades.end(); ++i) {

			//	cout << *i << endl;
			//	cout << "Inserting daily data for a stock into table DailyTrades ..." << endl << endl;
			//	char sql_Insert[512];
			//	sprintf_s(sql_Insert, "INSERT INTO DailyTrades(symbol, date, open, high, low, close, adjusted_close, volume) VALUES(\"%s\", \"%s\", %f, %f, %f, %f, %f, %d)", fakeSymbolTest.c_str(), i->GetDate().c_str(), i->GetOpen(), i->GetHigh(), i->GetLow(), i->GetClose(), i->GetAdjustedClose(), i->GetVolume());
			//	if (ExecuteSQL(db, sql_Insert) == -1)
			//		return -1;
			//}
			// doing intraday  data
			//const vector<IntradayTrade> intradTrades = stockMap[intraday_symbol].GetIntradayTrade();
			//for (auto i = intradTrades.begin(); i != intradTrades.end(); ++i) {

			//	cout << *i << endl;
			//	cout << "Inserting intra data for a stock into table IntradayTrades ..." << endl << endl;
			//	char sql_Insert[512];
			//	sprintf_s(sql_Insert, "INSERT INTO IntradayTrades(symbol, date, Timestamp, open, high, low, close, volume) VALUES(\"%s\", \"%s\", \"%s\", %f, %f, %f, %f, %d)", fakeSymbolTest.c_str(), i->GetDate().c_str(), i->GetTimestamp().c_str(), i->GetOpen(), i->GetHigh(), i->GetLow(), i->GetClose(), i->GetVolume());
			//	if (ExecuteSQL(db, sql_Insert) == -1)
			//		return -1;
			//}
			
			//vector<string> symbols;
			// the stock map version 
			for (auto j = symbols.begin(); j != symbols.end(); ++j) {
				// do daily
				const vector<DailyTrade> dailyTrades = stockMap[*j].GetDailyTrade();
				for (auto i = dailyTrades.begin(); i != dailyTrades.end(); ++i) {

					cout << *i << endl;
					cout << "Inserting daily data for a stock into table DailyTrades ..." << endl << endl;
					char sql_Insert[512];
					sprintf_s(sql_Insert, "INSERT INTO DailyTrades(symbol, date, open, high, low, close, adjusted_close, volume) VALUES(\"%s\", \"%s\", %f, %f, %f, %f, %f, %d)", (*j).c_str(), i->GetDate().c_str(), i->GetOpen(), i->GetHigh(), i->GetLow(), i->GetClose(), i->GetAdjustedClose(), i->GetVolume());
					if (ExecuteSQL(db, sql_Insert) == -1)
						return -1;
				}
				// do intraday
				const vector<IntradayTrade> intradTrades = stockMap[*j].GetIntradayTrade();
				for (auto i = intradTrades.begin(); i != intradTrades.end(); ++i) {

					cout << *i << endl;
					cout << "Inserting intra data for a stock into table IntradayTrades ..." << endl << endl;
					char sql_Insert[512];
					sprintf_s(sql_Insert, "INSERT INTO IntradayTrades(symbol, date, Timestamp, open, high, low, close, volume) VALUES(\"%s\", \"%s\", \"%s\", %f, %f, %f, %f, %d)", (*j).c_str(), i->GetDate().c_str(), i->GetTimestamp().c_str(), i->GetOpen(), i->GetHigh(), i->GetLow(), i->GetClose(), i->GetVolume());
					if (ExecuteSQL(db, sql_Insert) == -1)
						return -1;
				}
			}

			break;
		}
		case "d"_hash:
		case "D"_hash: 
		{
			// Display DailyTrades Table
			cout << "Retrieving values in table DailyTrades ..." << endl;
			const char* sql_Select = "SELECT * FROM DailyTrades;";
			if (ShowTable(db, sql_Select) == -1)
				return -1;

			// Display intradayTrades Table
			cout << "Retrieving values in table IntradayTrades ..." << endl;
			const char* sql_SelectIntra = "SELECT * FROM IntradayTrades;";
			if (ShowTable(db, sql_SelectIntra) == -1)
				return -1;
	
			break;
		}


		case "x"_hash:
		case "X"_hash:
			bCompleted = true;
			break;
		default:
			cout << "unknown";
//			break;
		}


	}
}


