#pragma once
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include<fstream>
#include <utility>
using namespace std;

class TradeData
{
private:
    string sDate;
    double dOpen;
    double dHigh;
    double dLow;
    double dClose;
    double dAdjClose;
    long lVolume;
public:
    TradeData() : sDate(""), dOpen(0), dClose(0), dHigh(0), dLow(0), dAdjClose(0), lVolume(0) {}
    TradeData(string sDate_, double dOpen_, double dHigh_, double dLow_, double dClose_, double dAdjClose_, long lVolume_) :
        sDate(sDate_), dOpen(round(dOpen_ * 100) / 100), dHigh(round(dHigh_ * 100) / 100), dLow(round(dLow_ * 100) / 100), dClose(round(dClose_ * 100) / 100), dAdjClose(round(dAdjClose_ * 100) / 100), lVolume(lVolume_) {}

    TradeData(const TradeData& TradeData) : sDate(TradeData.sDate), dOpen(TradeData.dOpen), dHigh(TradeData.dHigh), dLow(TradeData.dLow), dClose(TradeData.dClose), dAdjClose(TradeData.dAdjClose), lVolume(TradeData.lVolume) {}
    TradeData operator=(const TradeData& TradeData)
    {
        sDate = TradeData.sDate;
        dOpen = TradeData.dOpen;
        dHigh = TradeData.dHigh;
        dLow = TradeData.dLow;
        dClose = TradeData.dClose;
        dAdjClose = TradeData.dAdjClose;
        lVolume = TradeData.lVolume;
        return *this;
    }
    // Getter
    string GetDate() const { return sDate; }
    double GetOpen() const { return dOpen; }
    double GetHigh() const { return dHigh; }
    double GetLow() const { return dLow; }
    double GetClose() const { return dClose; }
    double GetAdjClose() const { return dAdjClose; }
    long GetVolume() const { return lVolume; }
    // Friend
    friend ostream& operator << (ostream& out, const TradeData& t)
    {
        out << " Date: " << t.GetDate() << " Open: " << t.GetOpen()
            << " High: " << t.GetHigh() << " Low: " << t.GetLow() << " Close: " << t.GetClose() << " AdjVolume: " << t.GetAdjClose()
            << " Volume: " << t.GetVolume() << endl;
        return out;

    }
};


class Stock
{
private:
    string sSymbol;
    vector<TradeData> trades;

public:
    // Default constructor
    Stock() : sSymbol("") {}

    // Parameterized constructor
    Stock(string sSymbol_, const vector<TradeData> trades_)
    {
        this->sSymbol = sSymbol_;
        this->trades = trades_;
    }

    // Copy constructor
    Stock(const Stock& other)
    {
        this->sSymbol = other.sSymbol;
        this->trades = other.trades;
    }

    // Overload assignment operator
    Stock operator=(const Stock& other)
    {
        this->sSymbol = other.sSymbol;
        this->trades = other.trades;
        return *this;
    }

    // Getter
    string GetSymbol() const { return sSymbol; }

    const vector<TradeData>& GetTrades() const { return trades; }

    // Function
    void addTrade(const TradeData& tradedata)
    {
        trades.push_back(tradedata);
    }

    // Friend
    friend ostream& operator<<(ostream& out, const Stock& stock);
};

struct PairPrice
{
    double dOpen1;
    double dClose1;
    double dOpen2;
    double dClose2;
    double dProfitLoss;

    // Default constructor
    PairPrice() : dOpen1(0), dClose1(0), dOpen2(0), dClose2(0), dProfitLoss(0) {}

    // Parameterized constructor
    PairPrice(double dOpen1_, double dClose1_, double dOpen2_, double dClose2_, double dProfitLoss_)
    {
        this->dOpen1 = dOpen1_;
        this->dClose1 = dClose1_;
        this->dOpen2 = dOpen2_;
        this->dClose2 = dClose2_;
        this->dProfitLoss = dProfitLoss_;
    }

    friend ostream& operator<<(ostream& out, const PairPrice& pp);
};

class StockPairPrices
{
private:
    pair<string, string> stockPair;
    double volatility;
    double k;
    double FinalPnL;
    map<string, PairPrice> dailyPairPrices;

public:
    // Default constructor
    StockPairPrices() : volatility(0), k(0) {}

    // Parameterized constructor
    StockPairPrices(pair<string, string> stockPair_)
    {
        this->stockPair = stockPair_;
        this->volatility = 0;
        this->k = 0;
    }

    // Getter
    const pair<string, string>& GetStockPair() const { return stockPair; }
    const map<string, PairPrice>& GetDailyPrices() const { return dailyPairPrices; }
    map<string, PairPrice>& GetDailyPrices() { return dailyPairPrices; }
    double GetVolatility() const { return volatility; }
    double GetK() const { return k; }
    double GetFinalPnL() const { return FinalPnL; }

    // Calculate the total profit and loss
    double GetTotalPnL() const;

    // Setter
    void SetVolatility(double volatility_) { this->volatility = volatility_; }
    void SetK(double k_) { this->k = k_; }
    void SetStockPair(string symbol1, string symbol2) { stockPair.first = symbol1; stockPair.second = symbol2; }
    void SetFinalPnL(double FinalPnL_) { FinalPnL = FinalPnL_; }

    void SetDailyPairPrice(string sDate_, PairPrice pairPrice_)
    {
        dailyPairPrices.insert(pair<string, PairPrice>(sDate_, pairPrice_));
    }

    void UpdateProfitLoss(string sDate_, double dProfitLoss_)
    {
        dailyPairPrices[sDate_].dProfitLoss = dProfitLoss_;
    }

    friend ostream& operator<<(ostream& out, const StockPairPrices& spp);
};
