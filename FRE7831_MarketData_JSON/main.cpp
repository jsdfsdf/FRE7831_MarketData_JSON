#include "Stock.h"
#include "MarketData.h"
#include "Database.h"
#include "calculation.h"
#include "Util.h"
#include <map>
#include <string>
#include <stdio.h>
#include "json/json.h"
#include "curl/curl.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <set>


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
	string database_name = "PairTrading.db";
	cout << "Opening PairTrading.db ..." << endl;
	sqlite3* db = NULL;
	if (OpenDatabase(database_name.c_str(), db) == -1)
		return -1;

	// Drop the table if exists
	cout << "Drop " << tableName << " if exists" << endl;
	string sql_DropaTable = "DROP TABLE IF EXISTS " + tableName;
	if (DropTable(db, sql_DropaTable.c_str()) == -1)
		return -1;

	// Create the table
	cout << "Creating " << tableName << "table ..." << endl;
	if (ExecuteSQL(db, sql_CreateTable) == -1)
		return -1;

	return 1;
}

int optionH(string tableName) {
	string sql_DropaTable = "DROP TABLE IF EXISTS " + tableName;
	string database_name = "PairTrading.db";
	sqlite3* db = NULL;
	if (OpenDatabase(database_name.c_str(), db) == -1)
		return -1;
	if (DropTable(db, sql_DropaTable.c_str()) == -1)
		return -1;
	return 0;
}

vector<StockPairPrices> AllPairs;
int main(void)
{
	sqlite3* db = NULL;
	const char* dataBaseName = "PairTrading.db";
	if (OpenDatabase(dataBaseName, db) != 0) return -1;
	bool bCompleted = false;
	//char selection;
	string sConfigFile = "config.csv";
	string back_test_start_date = "2022-01-01";
	map<string, string> config_map = ProcessConfigData(sConfigFile);


	map<string, Stock> stockMap;
	set<string> symbol1, symbol2;
	vector<string> symbolVec1, symbolVec2;
	vector<StockPairPrices> AllPairs;

	while (!bCompleted)
	{
		std::cout << endl;
		std::cout << "Menu" << endl;
		std::cout << "========" << endl;
		std::cout << "A - Create and Populate Pair Tables" << endl;
		std::cout << "B - Retrieve and Populate Historical Data for each stock" << endl; // stock
		std::cout << "C - Create PairPrices Table" << endl;
		std::cout << "D - Calculate Volatility" << endl;
		std::cout << "E - Back Test" << endl;
		std::cout << "F - Calculate Profit and Loss for Each Pair" << endl;
		std::cout << "G - Manual Testing" << endl;
		std::cout << "H - Drop All the Tables" << endl;

		std::cout << "X - Exit" << endl << endl;
		char* selection;
		selection = new char;
		std::cin >> selection;
		switch (hash_(selection))
		{
		case "a"_hash:
		case "A"_hash:
		{
			const char* sql_CreateTable = "CREATE TABLE IF NOT EXISTS StockPairs(" \
				"id INT NOT NULL, "
				"symbol1 CHAR(20) NOT NULL," \
				"symbol2 CHAR(20) NOT NULL," \
				"volatility REAL NOT NULL," \
				"profit_loss REAL NOT NULL," \
				"PRIMARY KEY(symbol1, symbol2)"
				");";

			int resultNum = optionA(sql_CreateTable, "StockPairs");

			if (resultNum != 1) {
				return resultNum;
			}
			// insert data from txt
			ifstream myfile("PairTradingTest.txt");
			string s;
			int pairId = 1;
			cout << "Inserting pair data into table StockPairs ..." << endl << endl;
			while (getline(myfile, s))      //是逐行读取文件信息
			{
				cout << s << endl;
				vector<string> symbols = split(s, ',');
				char sql_Insert[512];
				sprintf_s(sql_Insert, "INSERT INTO StockPairs(id, symbol1, symbol2, volatility, profit_loss) VALUES(%d, \"%s\", \"%s\", %f, %f)", pairId, symbols[0].c_str(), symbols[1].c_str(), 0.0, 0.0);
				if (ExecuteSQL(db, sql_Insert) == -1)
					return -1;
				symbol1.insert(symbols[0]);
				symbol2.insert(symbols[1]);
				symbolVec1.push_back(symbols[0]);
				symbolVec2.push_back(symbols[1]);
				pairId++;
			}

			sql_CreateTable = "CREATE TABLE IF NOT EXISTS PairOnePrices "
				"(symbol CHAR(20) NOT NULL,"
				"date CHAR(20) NOT NULL,"
				"open REAL NOT NULL,"
				"high REAL NOT NULL,"
				"low REAL NOT NULL,"
				"close REAL NOT NULL,"
				"adjusted_close REAL NOT NULL,"
				"Volume INT NOT NULL,"
				"PRIMARY KEY(symbol, date)"
				");";
			resultNum = optionA(sql_CreateTable, "PairOnePrices");

			if (resultNum != 1) {
				return resultNum;
			}

			sql_CreateTable = "CREATE TABLE IF NOT EXISTS PairTwoPrices "
				"(symbol CHAR(20) NOT NULL,"
				"date CHAR(20) NOT NULL,"
				"open REAL NOT NULL,"
				"high REAL NOT NULL,"
				"low REAL NOT NULL,"
				"close REAL NOT NULL,"
				"adjusted_close REAL NOT NULL,"
				"Volume INT NOT NULL,"
				"PRIMARY KEY(symbol, date)"
				");";
			resultNum = optionA(sql_CreateTable, "PairTwoPrices");

			if (resultNum != 1) {
				return resultNum;
			}

			break;
		}
		case "b"_hash:
		case "B"_hash:
		{
			//build a stock map

			string readBuffer, readBuffer_Intra;
			string daily_url_common = config_map["daily_url_common"];
			string start_date = config_map["start_date"];
			string end_date = config_map["end_date"];
			string api_token = config_map["api_token"];
			// symbol 1
			for (auto i = symbol1.begin(); i != symbol1.end(); ++i) {
				cout << *i;
				vector<TradeData> TradeDataVec;
				Stock myStock(*i, TradeDataVec);
				// do daily
				string url_request = daily_url_common + *i + ".US?" + "from=" + start_date + "&to=" + end_date + "&api_token=" + api_token + "&period=d&fmt=json";
				if (PullMarketData(url_request.c_str(), readBuffer) != 0)
					return -1;
				else
				{
					if (PopulateDailyTrades(readBuffer, myStock) != 0)
						return -1;
				}
				readBuffer.clear();
				stockMap[*i] = myStock;
			}
			// symbol 2
			for (auto i = symbol2.begin(); i != symbol2.end(); ++i) {
				cout << *i;
				vector<TradeData> TradeDataVec;
				Stock myStock(*i, TradeDataVec);
				// do daily
				string url_request = daily_url_common + *i + ".US?" + "from=" + start_date + "&to=" + end_date + "&api_token=" + api_token + "&period=d&fmt=json";
				if (PullMarketData(url_request.c_str(), readBuffer) != 0)
					return -1;
				else
				{
					if (PopulateDailyTrades(readBuffer, myStock) != 0)
						return -1;
				}
				readBuffer.clear();
				stockMap[*i] = myStock;
			}
			// data
			// pair one
			for (auto j = symbol1.begin(); j != symbol1.end(); ++j) {
				cout << *j;
				const vector<TradeData> dailyTrades = stockMap[*j].GetTrades();
				for (auto i = dailyTrades.begin(); i != dailyTrades.end(); ++i) {

					cout << *i << endl;
					cout << "Inserting daily data for a stock into table PairOnePrices ..." << endl << endl;
					char sql_Insert[512];
					sprintf_s(sql_Insert, "INSERT INTO PairOnePrices(symbol, date, open, high, low, close, adjusted_close, volume) VALUES(\"%s\", \"%s\", %f, %f, %f, %f, %f, %d)", (*j).c_str(), i->GetDate().c_str(), i->GetOpen(), i->GetHigh(), i->GetLow(), i->GetClose(), i->GetAdjClose(), i->GetVolume());
					if (ExecuteSQL(db, sql_Insert) == -1)
						return -1;
				}
			}
			// pair two
			for (auto j = symbol2.begin(); j != symbol2.end(); ++j) {
				cout << *j;
				const vector<TradeData> dailyTrades = stockMap[*j].GetTrades();
				for (auto i = dailyTrades.begin(); i != dailyTrades.end(); ++i) {

					cout << *i << endl;
					cout << "Inserting daily data for a stock into table PairTwoPrices ..." << endl << endl;
					char sql_Insert[512];
					sprintf_s(sql_Insert, "INSERT INTO PairTwoPrices(symbol, date, open, high, low, close, adjusted_close, volume) VALUES(\"%s\", \"%s\", %f, %f, %f, %f, %f, %d)", (*j).c_str(), i->GetDate().c_str(), i->GetOpen(), i->GetHigh(), i->GetLow(), i->GetClose(), i->GetAdjClose(), i->GetVolume());
					if (ExecuteSQL(db, sql_Insert) == -1)
						return -1;
				}
			}
			// Pop Data into stockPair
			for (int i = 0; i < symbolVec1.size(); i++)
			{
				pair<string, string> aPair = { symbolVec1[i], symbolVec2[i] };
				StockPairPrices stockPair = StockPairPrices(aPair);
				
				AllPairs.push_back(stockPair);
			}
			break;
		}
		case "c"_hash:
		case "C"_hash:
		{
			const char* sql_CreateTable = "CREATE TABLE IF NOT EXISTS PairPrices(" \
				"symbol1 CHAR(20) NOT NULL," \
				"symbol2 CHAR(20) NOT NULL," \
				"date CHAR(20) NOT NULL," \
				"open1 REAL NOT NULL,"
				"close1 REAL NOT NULL,"
				"adjusted_close1 REAL NOT NULL,"
				"open2 REAL NOT NULL,"
				"close2 REAL NOT NULL,"
				"adjusted_close2 REAL NOT NULL,"
				"profit_loss REAL NOT NULL," \
				"PRIMARY KEY(symbol1, symbol2, date),"
				"FOREIGN KEY(symbol1, date) REFERENCES PairOnePrices(symbol, date) ON DELETE CASCADE ON UPDATE CASCADE,"
				"FOREIGN KEY(symbol2, date) REFERENCES PairTwoPrices(symbol, date) ON DELETE CASCADE ON UPDATE CASCADE,"
				"FOREIGN KEY(symbol1, symbol2) REFERENCES StockPairs(symbol1, symbol2) ON DELETE CASCADE ON UPDATE CASCADE);";

			int resultNum = optionA(sql_CreateTable, "PairPrices");

			if (resultNum != 1) {
				return resultNum;
			}

			// insert
			char sql_Insert[1024];
			sprintf_s(sql_Insert, \
				"INSERT INTO PairPrices "
				"SELECT StockPairs.symbol1 AS symbol1, "
				"StockPairs.symbol2 AS symbol2, "
				"PairOnePrices.date AS date, "
				"PairOnePrices.open AS open1, "
				"PairOnePrices.close AS close1, "
				"PairOnePrices.adjusted_close AS adjusted_close1, " // Use adjusted_close for historical data
				"PairTwoPrices.open AS open2, "
				"PairTwoPrices.close AS close2, "
				"PairTwoPrices.adjusted_close AS adjusted_close2, " // Use adjusted_close for historical data
				"0 AS profit_loss "
				"FROM StockPairs, PairOnePrices, PairTwoPrices "
				"WHERE (((StockPairs.symbol1 = PairOnePrices.symbol) "
				"AND (StockPairs.symbol2 = PairTwoPrices.symbol)) "
				"AND (PairOnePrices.date = PairTwoPrices.date)) "
				"ORDER BY symbol1, symbol2;");
			if (ExecuteSQL(db, sql_Insert) == -1)
				return -1;
			for (auto& p : AllPairs)
			{
				string s1 = p.GetStockPair().first, s2 = p.GetStockPair().second;
				string sql_stmt = "SELECT date, open1, close1, open2, close2 FROM PairPrices WHERE symbol1 =  " + s1 + " AND symbol2 = " + s2 + " AND date >=" + back_test_start_date + ";";
				int rc = 0;
				char* error = nullptr;
				char** results = NULL;
				int rows, columns;
				string date_;
				double open1_, close1_, open2_, close2_;

				sqlite3_get_table(db, sql_stmt.c_str(), &results, &rows, &columns, &error);
				if (rc)
				{
					std::cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << std::endl << std::endl;
					sqlite3_free(error);
					return -1;
				}
				else
				{
					// Display Table
					for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
					{
						int count = rowCtr * columns;
						date_ = results[count];
						open1_ = atof(results[count + 1]);
						close1_ = atof(results[count + 2]);
						open2_ = atof(results[count + 3]);
						close2_ = atof(results[count + 4]);
						PairPrice price = PairPrice(open1_, close1_, open2_, close2_, 0);
						p.SetDailyPairPrice(date_, price);
					}
				}
				sqlite3_free_table(results);
			}

			break;
		}
		case "d"_hash:
		case "D"_hash:
		{
			//string back_test_start_date = "2022-01-05";
			string calculate_volatility_for_pair = string("Update StockPairs SET volatility =")
				+ "(SELECT(AVG((adjusted_close1/adjusted_close2)*(adjusted_close1/ adjusted_close2)) - AVG(adjusted_close1/adjusted_close2)*AVG(adjusted_close1/adjusted_close2)) as variance "
				+ "FROM PairPrices "
				+ "WHERE StockPairs.symbol1 = PairPrices.symbol1 AND StockPairs.symbol2 = PairPrices.symbol2 AND PairPrices.date <= \'"
				+ back_test_start_date + "\');";
			if (ExecuteSQL(db, calculate_volatility_for_pair.c_str()) == -1)
				return -1;
			vector<double> vols;
			AllPairs = getMyPairs(db);
			if (GetVolFromDatabase(db, vols) == -1)
				return -1;
			for (int i = 0; i < vols.size(); i++)
			{
				AllPairs[i].SetVolatility(vols[i]);
			}
			break;
		}

		case "e"_hash:
		case "E"_hash:
		{
			double kvalue=1.0;
			cout << "Please input k: ";
			cin >> kvalue;
			cout << endl;
			//vector<StockPairPrices> AllPairs = getMyPairs(db);
			if (CalculateBackTest(db, AllPairs, kvalue) != 0)
				return -1;

			cout << "Retrieving Top values of table PairPrices ..." << endl;
			cout << endl;
			string sqlSelect = "SELECT * FROM PairPrices where date >= \'" + back_test_start_date + "\';";


			if (ShowTable(db, sqlSelect.c_str()) != 0)
				return -1;
			break;
		}







		case "f"_hash:
		case "F"_hash:
		{
			string symbol_1;
			string symbol_2;
			//string back_test_start_date = "2022-01-05";
			int rc = 0;
			char* error = nullptr;
			//vector<StockPairPrices> AllPairs = getMyPairs(db);
			for (vector<StockPairPrices>::iterator itr = AllPairs.begin(); itr != AllPairs.end(); itr++) {
				symbol_1 = itr->GetStockPair().first;
				symbol_2 = itr->GetStockPair().second;
				cout << itr->GetVolatility();
				double PNL = 0.0;
				map<string, PairPrice> dailyPairPrices = itr->GetDailyPrices();

				for (map< string, PairPrice>::iterator d_itr = dailyPairPrices.begin(); d_itr != dailyPairPrices.end(); d_itr++) {
					if (d_itr->first < back_test_start_date)
						continue;
					PNL += d_itr->second.dProfitLoss;
					
					cout << "dProfitLoss: " << (d_itr->second.dProfitLoss) << endl;
				};
				cout << "pnl: " << PNL << endl;

				cout << "Update Total PNl for Stock Pair" << symbol_1 << "," << symbol_2 << endl;
				
				char calculate_total_pnl[512];
				sprintf(calculate_total_pnl, "Update StockPairs SET Profit_Loss = %f WHERE Symbol1 = \"%s\" AND Symbol2 = \"%s\";", PNL, symbol_1.c_str(), symbol_2.c_str());

				rc = sqlite3_exec(db, calculate_total_pnl, NULL, NULL, &error);
				if (rc)
				{
					cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
					sqlite3_free(error);
					return -1;
				}
				
				
			}






			break;
		}



		case "G"_hash:
		case "g"_hash:
		{
			// Show avaliable pairs
			std::cout << "Show avaliable pairs" << endl;
			const char* sql_select = "SELECT id, symbol1, symbol2 FROM StockPairs";
			if (ShowTable(db, sql_select) == -1)
				return -1;
			std::cout << "Enter the id of stock pairs: " << endl;
			unsigned short pair_id;
			std::cin >> pair_id;
			pair_id -= 1;
			if (pair_id > symbolVec1.size())
			{
				std::cout << "Pair id out of range, please reenter the pair id: " << endl;
				return -1;
			}
			std::cout << symbolVec1[pair_id] << ' ' << symbolVec2[pair_id] << endl;
			double vol_pair = AllPairs[pair_id].GetVolatility();
			std::cout << "Volatility: " << vol_pair << endl;
			double open1d2, open2d2, close1d1, close2d1, close1d2, close2d2, N2, ProfitLoss, k_pair;
			int LongShort;
			const double N1 = 10000.0;

			std::cout << endl;
			std::cout << "Choose k: ";
			std::cin >> k_pair;
			std::cout << endl;
			std::cout << endl;

			std::cout << "Enter Day 1 Close Prices: " << endl;
			std::cout << "Enter Close Price for First Stock: ";
			std::cin >> close1d1;
			std::cout << "Enter Close Price for Second Stock: ";
			std::cin >> close2d1;
			std::cout << endl;
			std::cout << endl;

			std::cout << "Enter Day 2 Open Prices" << endl;
			std::cout << "Enter Open Price for First Stock: ";
			std::cin >> open1d2;
			std::cout << "Enter Open Price for Second Stock: ";
			std::cin >> open2d2;
			std::cout << endl;
			std::cout << endl;

			std::cout << "Enter Day 2 Close Prices" << endl;
			std::cout << "Enter Close Price for First Stock: ";
			std::cin >> close1d2;
			std::cout << "Enter Close Price for Second Stock: ";
			std::cin >> close2d2;
			std::cout << endl;

			if (abs(close1d1 / close2d1 - open1d2 / open2d2) > (vol_pair * k_pair))
				LongShort = -1;
			else
				LongShort = 1;
			N2 = N1 * (open1d2 / open2d2);
			ProfitLoss = (-LongShort * N1 * (open1d2 - close1d2)) + (LongShort * N2 * (open2d2 - close2d2));

			std::cout << endl << "Simmulated Results" << endl;
			if (LongShort == -1) {
				std::cout << "Short Position Taken in First Stock: 10000 Shares" << endl;
				std::cout << "Long Postion Taken in Second Stock: " << N2 << endl;
			}
			else {
				std::cout << "Long Position Taken in First Stock: 10,000 Shares" << endl;
				std::cout << "Short Postion Taken in Second Stock: " << N2 << " Shares" << endl;
			}
			std::cout << "The Profit and Loss is: " << ProfitLoss << endl;
			break;
		}
		case "h"_hash:
		case "H"_hash:
		{
			optionH("StockPairs");
			optionH("PairPrices");
			optionH("PairOnePrices");
			optionH("PairTwoPrices");
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



