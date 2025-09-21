#include "alpaca_client.hpp"
#include <iostream>
#include <cpr/cpr.h>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace quantlab::data {

// ENHANCED: MULTI-MINUTE RATE-LIMITED AGGREGATION SYSTEM
std::vector<quantlab::core::Bar> AlpacaClient::get_aggregated_historical_bars(
    const std::string& symbol,
    const std::string& timeframe,
    int total_days,
    int days_per_call) {
    
    // Calculate rate limiting for API calls
    const int MAX_CALLS_PER_MINUTE = 200; // Free plan limit
    int total_minutes = (total_days + MAX_CALLS_PER_MINUTE - 1) / MAX_CALLS_PER_MINUTE;
    
    std::vector<quantlab::core::Bar> all_bars;
    int calls_in_current_minute = 0;
    auto minute_start = std::chrono::steady_clock::now();
    
    // Start from 15 days ago to avoid recent SIP data restrictions
    int start_offset = 15;
    for (int day_offset = start_offset; day_offset < total_days + start_offset; ++day_offset) {
        // Check if we need to wait for next minute
        if (calls_in_current_minute >= MAX_CALLS_PER_MINUTE) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - minute_start).count();
            
            if (elapsed < 60) {
                int wait_time = 60 - elapsed;
                std::this_thread::sleep_for(std::chrono::seconds(wait_time));
            }
            
            // Reset for new minute
            calls_in_current_minute = 0;
            minute_start = std::chrono::steady_clock::now();
        }
        
        // Calculate the specific date for this offset
        std::string date_str = get_date_string(day_offset);
        
        // Make single-day call with specific date range
        auto daily_bars = get_historical_bars(symbol, timeframe, date_str, date_str);
        
        if (!daily_bars.empty()) {
            all_bars.insert(all_bars.end(), daily_bars.begin(), daily_bars.end());
        }
        
        calls_in_current_minute++;
    }
    
    // Debug: Show data collection summary
    std::cout << "Collected " << all_bars.size() << " bars from " << total_days << " day period" << std::endl;
    
    // CRITICAL FIX: Reverse the data to be chronological (oldest first, newest last)
    // Current order: newest -> oldest (wrong for backtesting)
    // Required order: oldest -> newest (correct for backtesting)
    std::reverse(all_bars.begin(), all_bars.end());
    std::cout << "🔄 Data reordered chronologically (oldest to newest) for proper backtesting" << std::endl;
    
    return all_bars;
}


// Helper function to get date string for historical lookups
std::string AlpacaClient::get_date_string(int days_ago) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    // Subtract days
    time_t -= days_ago * 24 * 60 * 60;
    
    auto tm = *std::gmtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string AlpacaClient::make_request(const std::string& endpoint, bool use_market_data_api) {
    // Choose the correct base URL
    std::string base_url = use_market_data_api ? market_data_base_url_ : trading_base_url_;
    std::string url = base_url + endpoint;

    int retry_count = 0;
    const int max_retries = 5;
    const int base_delay_ms = 1000; // 1 second

    while (retry_count <= max_retries) {
        auto response = cpr::Get(
            cpr::Url{url}, 
            cpr::Header{
                {"APCA-API-KEY-ID", api_key_}, 
                {"APCA-API-SECRET-KEY", api_secret_}
            }
        );

        if (response.status_code == 200) {
            return response.text;
        } else if (response.status_code == 429) {
            // Handle rate limit with exponential backoff and jitter
            int backoff_time = base_delay_ms * (1 << retry_count); // Exponential backoff
            backoff_time += rand() % 500; // Add jitter (0-500ms)
            std::cerr << "Rate limit hit. Retrying in " << backoff_time << " ms..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(backoff_time));
            retry_count++;
        } else {
            std::cerr << "HTTP Error " << response.status_code << " for URL: " << url << std::endl;
            std::cerr << "Response: " << response.text << std::endl;
            return "";
        }
    }

    std::cerr << "Max retries reached for URL: " << url << std::endl;
    return "";
}

// METHOD 1: Test Connection
bool AlpacaClient::test_connection() {
    std::cout << "🔍 Testing Alpaca API connection..." << std::endl;
    
    std::string response = make_request("/account", false);
    
    if (response.empty()) {
        return false;
    }
    return true;
}

// METHOD 2: Get Historical Bars with Date Range Support
std::vector<quantlab::core::Bar> AlpacaClient::get_historical_bars(
    const std::string& symbol,
    const std::string& timeframe,
    const std::string& start_date,
    const std::string& end_date) {
    
    // Request historical bars for symbol
    
    // Build endpoint with proper date parameters (base URL already has /v2)
    std::string endpoint = "/stocks/" + symbol + "/bars?timeframe=" + timeframe;
    
    // Add date parameters for historical data
    if (!start_date.empty()) {
        endpoint += "&start=" + start_date;
    }
    if (!end_date.empty()) {
        endpoint += "&end=" + end_date;
    }
    
    std::string response = make_request(endpoint, true); // Use market data API
    
    if (response.empty()) {
        return {};
    }
    
    // Parse the JSON response
    try {
        auto json_data = nlohmann::json::parse(response);
        
        if (!json_data.contains("bars") || json_data["bars"].empty()) {
            return {};
        }
        
        std::vector<quantlab::core::Bar> bars;
        
        for (const auto& bar_json : json_data["bars"]) {
            quantlab::core::Bar bar;
            bar.timestamp = bar_json["t"];
            bar.open = bar_json["o"];
            bar.high = bar_json["h"];
            bar.low = bar_json["l"];
            bar.close = bar_json["c"];
            bar.volume = bar_json["v"];
            
            bars.push_back(bar);
        }
        
        // Return the bars
        
        return bars;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ JSON parsing error: " << e.what() << std::endl;
        std::cerr << "Raw response: " << response.substr(0, 200) << "..." << std::endl;
        return {};
    }
}

// METHOD 3: Get Latest Quote
std::optional<quantlab::core::Quote> AlpacaClient::get_latest_quote(const std::string& symbol) {
    // Get latest quote for symbol
    
    std::string endpoint = "/stocks/" + symbol + "/quotes/latest";
    std::string response = make_request(endpoint, true);
    
    if (response.empty()) {
        return std::nullopt;
    }
    
    try {
        auto json_data = nlohmann::json::parse(response);
        
        if (!json_data.contains("quote")) {
            return std::nullopt;
        }
        
        auto quote_data = json_data["quote"];
        
        quantlab::core::Quote quote;
        quote.symbol = symbol;
        quote.bid_price = quote_data["bp"];
        quote.ask_price = quote_data["ap"];
        quote.bid_size = quote_data["bs"];
        quote.ask_size = quote_data["as"];
        quote.timestamp = quote_data["t"];
        
        // Return the quote data
        
        return quote;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Quote parsing error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

} // namespace quantlab::data
