#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdlib>  // for getenv
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "../core/data_types.hpp"

namespace quantlab::data {

class AlpacaClient {
private:
    std::string trading_base_url_;     // For account/trading operations
    std::string market_data_base_url_; // For market data (prices, bars)
    std::string api_key_;
    std::string api_secret_;
    
    // HTTP helper methods
    std::string make_request(const std::string& endpoint, bool use_market_data_api = false);
    
    // Utility functions
    int64_t parse_iso8601_to_nanoseconds(const std::string& iso_timestamp);
    std::string get_date_string(int days_ago = 0);
    
public:
    // Constructor - reads API keys from environment variables
    AlpacaClient() {
        const char* key_id = std::getenv("ALPACA_API_KEY_ID");
        if (!key_id) {
            throw std::runtime_error("Missing ALPACA_API_KEY_ID environment variable");
        }
        api_key_ = key_id;
        
        const char* secret = std::getenv("ALPACA_API_SECRET_KEY");
        if (!secret) {
            throw std::runtime_error("Missing ALPACA_API_SECRET_KEY environment variable");
        }
        api_secret_ = secret;
        
        const char* base_url = std::getenv("ALPACA_BASE_URL");
        if (!base_url) {
            throw std::runtime_error("Missing ALPACA_BASE_URL environment variable");
        }
        trading_base_url_ = base_url;
        
        // Market data API uses different base URL
        market_data_base_url_ = "https://data.alpaca.markets/v2";
    }
    
    // Market data methods
    bool test_connection();
    
    std::vector<quantlab::core::Bar> get_historical_bars(
        const std::string& symbol,
        const std::string& timeframe = "1Day",
        const std::string& start_date = "",
        const std::string& end_date = ""
    );
    
    std::optional<quantlab::core::Quote> get_latest_quote(const std::string& symbol);
    
    // ENHANCED: Multi-minute rate-limited aggregation system
    std::vector<quantlab::core::Bar> get_aggregated_historical_bars(
        const std::string& symbol,
        const std::string& timeframe = "1Day",
        int total_days = 250,
        int days_per_call = 1
    );
};

} // namespace quantlab::data