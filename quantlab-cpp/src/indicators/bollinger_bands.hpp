#pragma once
#include <vector>
#include <deque>
#include <cmath>
#include <numeric>

namespace quantlab::indicators {

// QUESTION 1: How should we represent the three bands?
// Think about what Bollinger Bands returns:
// - Upper band
// - Middle band (Simple Moving Average)  
// - Lower band
// 
// Should this be a struct? What should it contain?

struct BollingerBandsResult {
    // TODO: Define the three bands
    // HINT: What are the three values that Bollinger Bands produces?
    double upper_band;
    double middle_band;  // This is the SMA
    double lower_band;
};

class BollingerBands {
private:
    // QUESTION 2: What member variables do you need?
    // Think about:
    // - How many periods to calculate over? (typical: 20)
    int period_;
    // - What multiplier for standard deviation? (typical: 2.0)
    double std_dev_multiplier_;
    // - How to store the recent price history?
    std::deque<double> price_history_;
    // - Current calculated bands?
    BollingerBandsResult current_bands_;
    // - Initialization state?
    bool initialized_;
    // TODO: Add your member variables here
    // HINT: Unlike EMA/RSI, you need to store multiple prices

public:
    // QUESTION 3: Constructor
    // What parameters should it take?
    // How do you initialize the member variables?
    explicit BollingerBands(int period = 20, double std_dev_multiplier = 2.0):period_(period), std_dev_multiplier_(std_dev_multiplier), current_bands_{0.0, 0.0, 0.0}, initialized_(false) {
        // TODO: Initialize your member variables
    }
    
    // QUESTION 4: The core algorithm - this is where the learning happens!
    BollingerBandsResult update(double price) {
        // ===== STEP 1: PRICE HISTORY MANAGEMENT =====
        // Add the newest price to our rolling window
        // This creates a "sliding window" of the most recent N prices
        price_history_.push_back(price);
        
        // ===== STEP 2: MAINTAIN ROLLING WINDOW =====
        // Keep only the last 'period_' prices (e.g., last 20 prices)
        // Remove oldest price when we have too many
        // Example: If period=20 and we have 21 prices, remove the oldest
        if(price_history_.size() > static_cast<size_t>(period_))
            price_history_.pop_front();  // Remove from front (oldest price)
            
        // ===== STEP 3: DATA VALIDATION =====
        // Need at least 'period_' prices for valid Bollinger Bands calculation
        // Example: For 20-period bands, need at least 20 price points
        if(price_history_.size() < static_cast<size_t>(period_))
            return BollingerBandsResult{0.0, 0.0, 0.0}; // Not enough data yet
        // ===== STEP 4: CALCULATE SIMPLE MOVING AVERAGE (SMA) =====
        // SMA = (sum of all prices) / number of prices
        // Example: prices [90, 95, 100, 105, 110] → SMA = 500/5 = 100
        // This is the MIDDLE BAND of Bollinger Bands
        // Unlike EMA, SMA gives equal weight to all prices in the window
        double sma = std::accumulate(price_history_.begin(), price_history_.end(), 0.0) / period_;
        // ===== STEP 5: CALCULATE STANDARD DEVIATION (VOLATILITY MEASURE) =====
        // Standard deviation measures how "spread out" prices are from the average
        // Formula: σ = sqrt(Σ(xi - μ)² / n)
        // 
        // LOW std_dev = prices close to average = LOW VOLATILITY = tight bands
        // HIGH std_dev = prices far from average = HIGH VOLATILITY = wide bands
        //
        // Example with prices [90, 95, 100, 105, 110] and SMA=100:
        // Differences: [-10, -5, 0, +5, +10]
        // Squared: [100, 25, 0, 25, 100] → Sum = 250
        // Variance = 250/5 = 50 → std_dev = sqrt(50) ≈ 7.07
        
        double variance_sum = 0.0;
        for (double p : price_history_) {
            double diff = p - sma;              // How far is this price from average?
            variance_sum += diff * diff;        // Square it (removes negatives, emphasizes larger moves)
        }
        double std_dev = std::sqrt(variance_sum / period_);  // Take square root to get standard deviation
        // ===== STEP 6: CALCULATE THE THREE BOLLINGER BANDS =====
        // The three bands create a "channel" around price action
        // 
        // MIDDLE BAND = Simple Moving Average (trend direction)
        // UPPER BAND = SMA + (k × std_dev) [typically k=2.0 for ~95% confidence]
        // LOWER BAND = SMA - (k × std_dev)
        //
        // TRADING INTERPRETATION:
        // • Price near UPPER band = potentially OVERBOUGHT (consider selling)
        // • Price near LOWER band = potentially OVERSOLD (consider buying)  
        // • Price at MIDDLE band = neutral/trend-following
        // • Bands NARROW = low volatility, potential breakout coming
        // • Bands WIDE = high volatility, market is moving strongly
        //
        // Example: SMA=100, std_dev=7.07, multiplier=2.0
        // → Upper = 100 + (2.0 × 7.07) = 114.14
        // → Lower = 100 - (2.0 × 7.07) = 85.86
        
        current_bands_.middle_band = sma;                                    // The trend line
        current_bands_.upper_band = sma + std_dev_multiplier_ * std_dev;     // Resistance level
        current_bands_.lower_band = sma - std_dev_multiplier_ * std_dev;     // Support level
        
        initialized_ = true;
        return current_bands_;  // Return all three bands for analysis
    }
    
    // QUESTION 5: Utility methods
    const BollingerBandsResult& value() const {
        // TODO: Return current bands
        return current_bands_;
    }
    
    bool is_initialized() const {
        // TODO: When do you have enough data for valid calculation?
        return initialized_;
    }
    
    void reset() {
        // TODO: Clear all state
        price_history_.clear();
        current_bands_ = {0.0, 0.0, 0.0};
        initialized_ = false;
    }
};

} // namespace quantlab::indicators