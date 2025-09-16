#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <cassert>

namespace quantlab::core {

/**
 * OHLCV bar with nanosecond timestamp precision
 */
struct Bar {
    // we use int64_t for timestamps to avoid floating point precision issues becuase IEEE 754 doubles only have 15 to 17 decimal digits of precision, also math operation on int is faster than double
    int64_t timestamp_ns;  // Unix epoch nanoseconds for microsecond precision
    std::string timestamp; // ISO timestamp string for API compatibility 
    double open; // OHLC prices
    double high; 
    double low;
    double close;
    int64_t volume; // # of shares traded

    Bar() = default;
    Bar(int64_t ts, double o, double h, double l, double c, int64_t v = 0)
        : timestamp_ns(ts), open(o), high(h), low(l), close(c), volume(v) {}
};

/**
 * Tick data for higher frequency operations
 */
struct Tick {
    int64_t timestamp_ns;
    double price;
    int32_t quantity;
    char side;  // 'B'uy, 'S'ell, 'T'rade

    Tick() = default;
    Tick(int64_t ts, double px, int32_t qty, char s = 'T')
        : timestamp_ns(ts), price(px), quantity(qty), side(s) {}
};

/**
 * Complete trade information from market data feed
 * Captures all metadata for professional analysis
 */
struct Trade {
    std::string symbol;           // Stock symbol (e.g., "AAPL")
    int64_t timestamp_ns;         // Unix epoch nanoseconds  
    double price;                 // Trade price
    int32_t size;                 // Number of shares
    int64_t trade_id;             // Unique trade identifier
    std::string exchange;         // Exchange code (e.g., "V" for NASDAQ)
    std::string tape;             // Market tape identifier
    std::vector<std::string> conditions; // Trade conditions/flags
    
    Trade() = default;
    Trade(const std::string& sym, int64_t ts, double p, int32_t s, int64_t id = 0)
        : symbol(sym), timestamp_ns(ts), price(p), size(s), trade_id(id) {}

    // Convert to simple Tick for compatibility
    Tick to_tick() const {
        return Tick(timestamp_ns, price, size, 'T');
    }
};

/**
 * Market quote with bid/ask pricing
 * Used for spread analysis and order execution
 */
struct Quote {
    std::string symbol;           // Stock symbol
    std::string timestamp;        // ISO timestamp string for API compatibility
    double bid_price;             // Best bid price
    double ask_price;             // Best ask price
    int32_t bid_size;             // Bid size (shares)
    int32_t ask_size;             // Ask size (shares)
    
    Quote() = default;
    Quote(const std::string& sym, const std::string& ts, double bid, double ask, int32_t bid_sz = 0, int32_t ask_sz = 0)
        : symbol(sym), timestamp(ts), bid_price(bid), ask_price(ask), bid_size(bid_sz), ask_size(ask_sz) {}
        
    double mid_price() const { return (bid_price + ask_price) / 2.0; }
    double spread() const { return ask_price - bid_price; }
};

/**
 * Time series container optimized for rolling operations
 * Maintains sorted order by timestamp
 */
template<typename T>
class TimeSeries {
private:
    std::vector<T> data_;
    
public:
    void reserve(size_t capacity) { data_.reserve(capacity); }
    
    void push_back(const T& item) {
        data_.push_back(item);
    }
    
    // Safe access with bounds checking in debug
    const T& operator[](size_t idx) const {
        assert(idx < data_.size());
        return data_[idx]; 
    }
    
    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }
    
    const T& back() const { return data_.back(); }
    const T& front() const { return data_.front(); }
    
    // Iterator support for range-based loops
    auto begin() const { return data_.begin(); }
    auto end() const { return data_.end(); }
    auto rbegin() const { return data_.rbegin(); }
    auto rend() const { return data_.rend(); }
    
    // Get slice of last n elements (for windowed operations)
    std::vector<T> tail(size_t n) const {
        if (n >= size()) return data_;
        return std::vector<T>(data_.end() - n, data_.end());
    }
};

using BarSeries = TimeSeries<Bar>;
using TickSeries = TimeSeries<Tick>;

} // namespace quantlab::core