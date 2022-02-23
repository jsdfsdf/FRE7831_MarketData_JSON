#pragma once
#include <string>
#include "Stock.h"
 class Stock;
 int PullMarketData(const std::string& url_request, std::string&
	read_buffer);
 int PopulateDailyTrades(const std::string& read_buffer,
	Stock& stock);
