#pragma once
#include <string>
 class Stock;
 int PullMarketData(const std::string& url_request, std::string&
	read_buffer);
 int PopulateDailyTrades(const std::string& read_buffer,
	Stock& stock);
 int PopulateIntradayTrades(const std::string& read_buffer,
	Stock& stock, long start_date);
