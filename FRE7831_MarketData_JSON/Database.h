#pragma once
#include "sqlite3.h"
#include <vector>
#include "Stock.h"

using namespace std;
int OpenDatabase(const char* database_name, sqlite3* & db);

int DropTable(sqlite3 * db, const char* sql_stmt);

int ExecuteSQL(sqlite3* db, const char* sql_stmt);

int ShowTable(sqlite3* db, const char* sql_stmt);

int GetVolFromDatabase(sqlite3* db, vector<double>& vols);

void CloseDatabase(sqlite3* db);

vector<StockPairPrices> getMyPairs(sqlite3* db);