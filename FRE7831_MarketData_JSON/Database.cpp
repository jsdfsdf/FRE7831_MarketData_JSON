#include "Database.h"
#include <iostream>
#include <string>

int OpenDatabase(const char* database_name, sqlite3* & db)
{
	int rc = 0;
	rc = sqlite3_open(database_name, &db);
	if (rc)
	{
		std::cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return -1;
	}
	else
	{
		std::cout << "Opened " << database_name << std::endl;
	}

	std::string foreign_ley_check = "PRAGMA foreign_keys = ON";
	if (sqlite3_exec(db, foreign_ley_check.c_str(), 0, 0, 0) != SQLITE_OK) {
		std::cout << "SQLite fails in setting foreign key check" << std::endl;
		sqlite3_close(db);
		return -1;
	}

	return 0;
}

int DropTable(sqlite3 * db, const char* sql_stmt)
{
	if (sqlite3_exec(db, sql_stmt, 0, 0, 0) != SQLITE_OK) {
		std::cout << "SQLite can not drop table" << std::endl;
		sqlite3_close(db);
		return -1;
	}
	return 0;
}

int ExecuteSQL(sqlite3* db, const char* sql_stmt)
{
	int rc = 0;
	char* error = nullptr;
	rc = sqlite3_exec(db, sql_stmt, NULL, NULL, &error);
	if (rc)
	{
		std::cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_free(error);
		return -1;
	}
	return 0;
}


int ShowTable(sqlite3* db, const char* sql_stmt)
{
	int rc = 0;
	char* error = nullptr;
	char** results = NULL;
	int rows, columns;
	sqlite3_get_table(db, sql_stmt, &results, &rows, &columns, &error);
	if (rc)
	{
		std::cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << std::endl << std::endl;
		sqlite3_free(error);
		return -1;
	}
	else
	{
		// Display Table
		for (int rowCtr = 0; rowCtr <= rows; ++rowCtr)
		{
			for (int colCtr = 0; colCtr < columns; ++colCtr)
			{
				// Determine Cell Position
				int cellPosition = (rowCtr * columns) + colCtr;

				// Display Cell Value
				std::cout.width(12);
				std::cout.setf(std::ios::left);
				std::cout << results[cellPosition] << " ";
			}

			// End Line
			std::cout << std::endl;

			// Display Separator For Header
			if (0 == rowCtr)
			{
				for (int colCtr = 0; colCtr < columns; ++colCtr)
				{
					std::cout.width(12);
					std::cout.setf(std::ios::left);
					std::cout << "~~~~~~~~~~~~ ";
				}
				std::cout << std::endl;
			}
		}
	}
	sqlite3_free_table(results);
	return 0;
}

int GetVolFromDatabase(sqlite3* db, vector<double>& vols)
{
	vector<double> volalitities;
	int rc = 0;
	char* error = nullptr;
	char** results = NULL;
	int rows, columns;
	const char* sql_stmt = "SELECT volatility FROM StockPairs";
	sqlite3_get_table(db, sql_stmt, &results, &rows, &columns, &error);
	if (rc)
	{
		std::cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << std::endl << std::endl;
		sqlite3_free(error);
		return -1;
	}
	else
	{
		for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
		{
			if (isalnum(results[rowCtr][0]))
				volalitities.push_back(atof(results[rowCtr]));
		}
		vols = volalitities;
	}
	sqlite3_free_table(results);
	return 0;
}

void CloseDatabase(sqlite3* db)
{
	sqlite3_close(db);
}

vector<StockPairPrices> getMyPairs(sqlite3* db)
{
	string sql_stmt = "select symbol1, symbol2 from StockPairs"; //PairPrices
	int rc = 0;
	char* error = nullptr;
	char** results = NULL;
	int rows, columns;
	vector<StockPairPrices> pairInfo;
	sqlite3_get_table(db, sql_stmt.c_str(), &results, &rows, &columns, &error);
	// Display Table
	for (int rowCtr = 1; rowCtr <= rows; ++rowCtr)
	{
		vector<string> curPair;
		for (int colCtr = 0; colCtr < columns; ++colCtr)
		{
			// Determine Cell Position
			int cellPosition = (rowCtr * columns) + colCtr;

			// Display Cell Value
			//std::cout.width(12);
			//std::cout.setf(std::ios::left);
			//std::cout << results[cellPosition] << " ";
			curPair.push_back(results[cellPosition]);
		}
		pair<string, string> aPair = { curPair[0], curPair[1] };
		StockPairPrices stockPair = StockPairPrices(aPair);
		pairInfo.push_back(stockPair);

	}
	sqlite3_free_table(results);
	return pairInfo;
}