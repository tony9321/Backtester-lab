#include "alpaca_client.hpp"
#include <iostream>
#include <cpr/cpr.h>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>

namespace quantlab::data {

// ENHANCED: MULTI-MINUTE RATE-LIMITED AGGREGATION SYSTEM
std::vector<quantlab::core::Bar> AlpacaClient::get_aggregated_historical_bars(
    const std::string& symbol,
    const std::string& timeframe,
    int total_days,
    int days_per_call) {
    
    std::cout << "🚀 FREE PLAN MULTI-MINUTE AGGREGATION SYSTEM ACTIVATED!" << std::endl;
    std::cout << "🎯 Target: " << total_days << " days of " << symbol << " historical data" << std::endl;
    std::cout << "💡 Strategy: Rate-limited calls for free plan (200/min limit)" << std::endl;
    
    // Calculate optimal batching strategy for FREE PLAN
    const int MAX_CALLS_PER_MINUTE = 200; // Free plan limit is much lower (typically 200/min)
    int total_minutes = (total_days + MAX_CALLS_PER_MINUTE - 1) / MAX_CALLS_PER_MINUTE;
    
    std::cout << "📊 Optimization Analysis:" << std::endl;
    std::cout << "   • Total calls needed: " << total_days << std::endl;
    std::cout << "   • Calls per minute limit: " << MAX_CALLS_PER_MINUTE << std::endl;
    std::cout << "   • Estimated duration: " << total_minutes << " minute(s)" << std::endl;
    std::cout << "   • Each call = 1 daily bar = Professional institutional data" << std::endl;
    
    std::vector<quantlab::core::Bar> all_bars;
    int calls_in_current_minute = 0;
    auto minute_start = std::chrono::steady_clock::now();
    
    for (int day_offset = 0; day_offset < total_days; ++day_offset) {
        // Check if we need to wait for next minute
        if (calls_in_current_minute >= MAX_CALLS_PER_MINUTE) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - minute_start).count();
            
            if (elapsed < 60) {
                int wait_time = 60 - elapsed;
                std::cout << "⏰ Rate limit reached. Waiting " << wait_time << " seconds for next minute..." << std::endl;
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
            std::cout << "📈 Day " << (day_offset + 1) << "/" << total_days 
                      << " collected (" << daily_bars.size() << " bars)" << std::endl;
        } else {
            std::cout << "⚠️  Day " << (day_offset + 1) << "/" << total_days 
                      << " - No data (weekend/holiday)" << std::endl;
        }
        
        calls_in_current_minute++;
        
        // Show progress every 50 calls
        if ((day_offset + 1) % 50 == 0) {
            std::cout << "�� Progress: " << (day_offset + 1) << "/" << total_days 
                      << " days (" << std::fixed << std::setprecision(1) 
                      << (100.0 * (day_offset + 1) / total_days) << "%)" << std::endl;
        }
    }
    
    std::cout << "✅ AGGREGATION COMPLETE!" << std::endl;
    std::cout << "📊 Final Dataset: " << all_bars.size() << " bars across " << total_days << " day period" << std::endl;
    
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
    
    auto response = cpr::Get(
        cpr::Url{url}, 
        cpr::Header{
            {"APCA-API-KEY-ID", api_key_}, 
            {"APCA-API-SECRET-KEY", api_secret_}
        }
    );
    
    if (response.status_code != 200) {
        std::cerr << "HTTP Error " << response.status_code << " for URL: " << url << std::endl;
        std::cerr << "Response: " << response.text << std::endl;
        return "";
    }
    return response.text;
}

// METHOD 1: Test Connection
bool AlpacaClient::test_connection() {
    std::cout << "🔍 Testing Alpaca API connection..." << std::endl;
    
    std::string response = make_request("/v2/account");
    
    if (response.empty()) {
        std::cout << "❌ Connection test failed!" << std::endl;
        return false;
    }
    
    std::cout << "✅ Connection successful!" << std::endl;
    return true;
}

// METHOD 2: Get Historical Bars with Date Range Support
std::vector<quantlab::core::Bar> AlpacaClient::get_historical_bars(
    const std::string& symbol,
    const std::string& timeframe,
    const std::string& start_date,
    const std::string& end_date) {
    
    std::cout << "📊 Fetching " << symbol << " bars from " << start_date 
              << " to " << end_date << " (" << timeframe << ")" << std::endl;
    
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
        std::cout << "❌ Failed to get historical data" << std::endl;
        return {};
    }
    
    // Parse the JSON response
    try {
        auto json_data = nlohmann::json::parse(response);
        
        if (!json_data.contains("bars") || json_data["bars"].empty()) {
            std::cout << "⚠️  No bars found for " << symbol << " on " << start_date << std::endl;
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
        
        std::cout << "✅ Retrieved " << bars.size() << " bars for " << symbol << std::endl;
        
        // Show price range for verification
        if (!bars.empty()) {
            double min_price = bars[0].low;
            double max_price = bars[0].high;
            
            for (const auto& bar : bars) {
                min_price = std::min(min_price, bar.low);
                max_price = std::max(max_price, bar.high);
            }
            
            std::cout << "💰 Price range: $" << std::fixed << std::setprecision(2) 
                      << min_price << " - $" << max_price 
                      << " (Range: " << std::setprecision(1) 
                      << ((max_price - min_price) / min_price * 100) << "%)" << std::endl;
        }
        
        return bars;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ JSON parsing error: " << e.what() << std::endl;
        std::cerr << "Raw response: " << response.substr(0, 200) << "..." << std::endl;
        return {};
    }
}

// METHOD 3: Get Latest Quote
std::optional<quantlab::core::Quote> AlpacaClient::get_latest_quote(const std::string& symbol) {
    std::cout << "💰 Getting latest quote for " << symbol << std::endl;
    
    std::string endpoint = "/stocks/" + symbol + "/quotes/latest";
    std::string response = make_request(endpoint, true);
    
    if (response.empty()) {
        return std::nullopt;
    }
    
    try {
        auto json_data = nlohmann::json::parse(response);
        
        if (!json_data.contains("quote")) {
            std::cout << "❌ No quote data found" << std::endl;
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
        
        std::cout << "✅ Quote: Bid $" << quote.bid_price 
                  << " Ask $" << quote.ask_price << std::endl;
        
        return quote;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Quote parsing error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

} // namespace quantlab::data
