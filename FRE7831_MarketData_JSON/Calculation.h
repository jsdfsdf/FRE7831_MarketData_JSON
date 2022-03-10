#pragma once

#include <stdio.h>
#include "Database.h"
#include "Stock.h"
#include <vector>

class calculation {
private:
	sqlite3* db;
	vector <StockPairPrices>& AllPairs;
	double Kvalue;
public:
	calculation(sqlite3* db_, vector <StockPairPrices>& AllPairs_, double Kvalue_):
		db(db_),AllPairs(AllPairs_),Kvalue(Kvalue_){}
	int CalculateBackTest();

};
