#pragma once
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include<fstream>
#include <utility>
using namespace std;

//class Trade
//	 {
// protected:
//	 float open;
//	 float high;
//	 float low;
//	 float close;
//	 int volume;
// public:
//	 Trade(float open_, float high_, float low_, float close_, int volume_) :
//		 open(open_), high(high_), low(low_), close(close_), volume(volume_) {}
//	 ~Trade() {}
//	 float GetOpen() const { return open; }
//	 float GetHigh() const { return high; }
//	 float GetLow() const { return low; }
//	 float GetClose() const { return close; }
//	 int GetVolume() const { return volume; }
//};
//
//
//class DailyTrade : public Trade
//	 {
// private:
//	 string date;
//	 float adjusted_close;
// public:
//	 DailyTrade(string date_, float open_, float high_, float low_, float close_, float adjusted_close_, int volume_) :
//		 Trade(open_, high_, low_, close_, volume_), date(date_), adjusted_close(adjusted_close_) {}
//	 ~DailyTrade() {}
//	 string GetDate() const { return date; }
//	 float GetAdjustedClose() const { return close; }
//	 friend ostream& operator << (ostream& out, const DailyTrade& t)
//		 {
//		 out << "Date: " << t.date << " Open: " << t.open << " High: " << t.high << " Low: " << t.low
//			 << " Close: " << t.close << " Adjusted_Close: " << t.adjusted_close << " Volume: " << t.volume << endl;
//		 return out;
//		
//	}
//};
//
//
//class IntradayTrade : public Trade
//	 {
// private:
//	 string date;
//	 string timestamp;
// public:
//	 IntradayTrade(string date_, string timestamp_, float open_, float high_, float low_, float close_, int volume_) :
//		 Trade(open_, high_, low_, close_, volume_), date(date_), timestamp(timestamp_) {}
//	 ~IntradayTrade() {}
//	 string GetDate() const { return date; }
//	 string GetTimestamp() const { return timestamp; }
//	 friend ostream& operator << (ostream& out, const IntradayTrade& t)
//		 {
//		 out << " Date: " << t.date << " Timestamp: " << t.timestamp << " Open: " << t.open
//			 << " High: " << t.high << " Low: " << t.low << " Close: " << t.close
//			 << " Volume: " << t.volume << endl;
//		 return out;
//		
//	}
//	
//};

//class Stock
//	 {
//private:
//	 string symbol;
//	 vector<DailyTrade> dailyTrades;
//	 vector<IntradayTrade> intradayTrades;
// public:
//	 Stock() :symbol("") { dailyTrades.clear(); intradayTrades.clear(); }
//	 Stock(string symbol_) : symbol(symbol_) { dailyTrades.clear(); intradayTrades.clear(); }
//	 Stock(const Stock& stock) { memcpy(this, &stock, sizeof(stock)); }
//	 ~Stock() {}
//	 void addDailyTrade(const DailyTrade& aTrade) { dailyTrades.push_back(aTrade); }
//	 void addIntradayTrade(const IntradayTrade& aTrade) { intradayTrades.push_back(aTrade); }
//	 string GetSymbol() const { return symbol; }
//	 const vector<DailyTrade>& GetDailyTrade(void) const { return dailyTrades; }
//	 const vector<IntradayTrade>& GetIntradayTrade(void) const { return intradayTrades; }
//	 friend ostream& operator << (ostream& out, const Stock& s)
//		 {
//		 out << "Symbol: " << s.symbol << endl;
//		 for (vector<DailyTrade>::const_iterator itr = s.dailyTrades.begin(); itr != s.dailyTrades.end(); itr++)
//			 out << *itr;
//		 for (vector<IntradayTrade>::const_iterator itr = s.intradayTrades.begin(); itr != s.intradayTrades.end(); itr ++)
//			 out << *itr;
//		 return out;
//		
//	}
//	
//};

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
	TradeData() : sDate(""), dOpen(0), dClose(0), dHigh(0), dLow(0), dAdjClose(0), lVolume(0){}
	TradeData(string sDate_, double dOpen_, double dHigh_, double dLow_, double dClose_, double dAdjClose_, long lVolume_):
		sDate(sDate_), dOpen(round(dOpen_ * 100) / 100), dHigh(round(dHigh_ * 100) / 100), dLow(round(dLow_ * 100) / 100), dClose(round(dClose_ * 100) / 100), dAdjClose(round(dAdjClose_ * 100) / 100), lVolume(lVolume_){}
    
	TradeData(const TradeData & TradeData): sDate(TradeData.sDate), dOpen(TradeData.dOpen), dHigh(TradeData.dHigh), dLow(TradeData.dLow), dClose(TradeData.dClose), dAdjClose(TradeData.dAdjClose), lVolume(TradeData.lVolume)  {}
	TradeData operator=(const TradeData & TradeData)
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
    PairPrice(double dOpen1_, double dClose1_, double dOpen2_, double dClose2_)
    {
        this->dOpen1 = dOpen1_;
        this->dClose1 = dClose1_;
        this->dOpen2 = dOpen2_;
        this->dClose2 = dClose2_;
        this->dProfitLoss = 0.;
    }

    friend ostream& operator<<(ostream& out, const PairPrice& pp);
};

class StockPairPrices
{
private:
    pair<string, string> stockPair;
    double volatility;
    double k;
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

    // Calculate the total profit and loss
    double GetTotalPnL() const;

    // Setter
    void SetVolatility(double volatility_) { this->volatility = volatility_; }
    void SetK(double k_) { this->k = k_; }

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