#include "Calculation.h"
#include <cmath>

int CalculateBackTest(sqlite3 * db, vector<StockPairPrices> & AllPairs)
{
    string BackTestStartDate = "2022-01-04";
    int rc = 0;
    char* error = nullptr;
    
    char** results = NULL;
    int rows, columns;
    
    string Stock1, Stock2;
    double VolPair;
    double kPair;
    
    double Open1d1, Close1d1, Open2d1, Close2d1;
    double Open1d2, Close1d2, Open2d2, Close2d2, ProfitLoss;
    
    int LongShort; // 1 is Long, -1 is Short
    double N1 = 10000;
    double N2;
    
    double Kvalue = 1;

    string date;
    for (auto itr = AllPairs.begin(); itr != AllPairs.end(); itr++)
    {
        Stock1 = (itr->GetStockPair()).first;
        Stock2 = (itr->GetStockPair()).second;
        
        itr->SetK(Kvalue);
        
        VolPair = itr->GetVolatility();
        kPair = itr->GetK();
        
        string sqlSelect = string("SELECT symbol1, symbol2, date, open1, close1, open2, close2 FROM PairPrices ")
            + "WHERE symbol1 = \'" + Stock1 + "\' AND symbol2 = \'" + Stock2 + "\';";
        
        rc = sqlite3_get_table(db, sqlSelect.c_str(), &results, &rows, &columns, &error);
        if (rc)
        {
            cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
            sqlite3_free(error);
        }
        else
        {
            Open1d1 = atof(results[(1 * columns) + 3]);
            Close1d1 = atof(results[(1 * columns) + 4]);
            Open2d1 = atof(results[(1 * columns) + 5]);
            Close2d1 = atof(results[(1 * columns) + 6]);
            ProfitLoss = 0;
            
            PairPrice aPair(Open1d1, Close1d1, Open2d1, Close2d1, ProfitLoss);
            
            itr->SetDailyPairPrice(results[(1 * columns) + 2], aPair);
            
//            map <string, PairPrice> dailyPairPrices = (itr->second).GetDailyPrices();
//            for (map <string, PairPrice>::iterator itr2 = dailyPairPrices.begin(); itr2 != dailyPairPrices.end(); itr2++)
//            {
//                cout << "+++++++++++++" << endl;
//                cout << "Date: " << itr2->first << endl;
//                if(itr2->first < BackTestStartDate)
//                    continue;
            char calculate_PnL[512];
            
            for (int rowCtr = 2; rowCtr <= rows; ++rowCtr)
            {
                Open1d2 = atof(results[(rowCtr * columns) + 3]);
                Close1d2 = atof(results[(rowCtr * columns) + 4]);
                Open2d2 = atof(results[(rowCtr * columns) + 5]);
                Close2d2 = atof(results[(rowCtr * columns) + 6]);
                
                if (abs(Close1d1/Close2d1 - Open1d2/Open2d2) > (VolPair * kPair))
                    LongShort = -1;
                else
                    LongShort = 1;
                
                N2 = N1 * (Open1d2 / Open2d2);
                ProfitLoss = (-LongShort * N1 * (Open1d2 - Close1d2)) + (LongShort * N2 * (Open2d2 - Close2d2));
                
                PairPrice aPair(Open1d2, Close1d2, Open2d2, Close2d2, ProfitLoss);
                
                itr->SetDailyPairPrice(results[(rowCtr * columns) + 2], aPair);
                
//                map <string, PairPrice> dailyPairPrices = (itr->second).GetDailyPrices();
//                for (map <string, PairPrice>::iterator itr2 = dailyPairPrices.begin(); itr2 != dailyPairPrices.end(); itr2++)
//                {
//                    cout << "+++++++++++++" << endl;
//                    cout << "Date: " << itr2->first << endl;
                    if(results[(rowCtr * columns) + 2] < BackTestStartDate)
                        continue;
                Close1d1 = atof(results[(rowCtr * columns) + 4]);
                Close2d1 = atof(results[(rowCtr * columns) + 6]);
                
                sprintf(calculate_PnL, "Update PairPrices SET profit_loss = %f WHERE date = \"%s\" AND symbol1 = \"%s\" AND symbol2 = \"%s\";", ProfitLoss, results[(rowCtr * columns) + 2], results[(rowCtr * columns) + 0], results[(rowCtr * columns) + 1]);
                
                rc = sqlite3_exec(db, calculate_PnL, NULL, NULL, &error);
                if (rc)
                {
                    cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
                    sqlite3_free(error);
                    return -1;
                }
                
            }
        }
        sqlite3_free_table(results);
//        }// end inner for
    }// end outter for
    
    return 0;
}
