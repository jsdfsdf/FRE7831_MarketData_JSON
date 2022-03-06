#pragma once
//
//  Calculation.hpp
//  FRE7831_MarketData_JSON_MAC
//
//  Created by Ruolian Duan on 3/3/22.
//  Copyright © 2022 NYU. All rights reserved.
//

#include <stdio.h>
#include "Database.h"
#include "Stock.h"

int CalculateBackTest(sqlite3* db, vector <StockPairPrices>& AllPairs,double Kvalue);
