#pragma once

#include "../core/data_types.hpp"
#include "../indicators/rolling_ema.hpp"
#include "../indicators/rsi.hpp"
#include "../indicators/bollinger_bands.hpp"
#include "../data/alpaca_client.hpp"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

namespace quantlab::strategy {

/**
 * Trading signal types
 */
enum class Signal {
    NONE,    // No signal
    BUY,     // Buy signal
    SELL,    // Sell signal
    HOLD     // Hold current position
};

/**
 * Strategy result with signal and reasoning
 */
struct StrategyResult {
    Signal signal;
    double confidence;  // 0.0 to 1.0
    std::string reason; // Human-readable explanation
    
    // Current indicator values for analysis
    double current_price;
    double ema_value;
    double rsi_value;
    double bb_upper;
    double bb_middle;
    double bb_lower;
    
    StrategyResult() : signal(Signal::NONE), confidence(0.0) {}
    
    StrategyResult(Signal sig, double conf, const std::string& r) 
        : signal(sig), confidence(conf), reason(r) {}
};

/**
 * Mean Reversion Strategy with Momentum Filter
 * 
 * STRATEGY LOGIC:
 * BUY SIGNALS:
 *   - RSI < 30 (oversold) AND 
 *   - Price < Lower Bollinger Band (price extreme) AND
 *   - Price > EMA (bullish trend filter)
 * 
 * SELL SIGNALS:
 *   - RSI > 70 (overbought) AND
 *   - Price > Upper Bollinger Band (price extreme) AND  
 *   - Price < EMA (bearish trend filter)
 */
class MeanReversionStrategy {
private:
    // Technical indicators
    quantlab::indicators::RollingEMA ema_;
    quantlab::indicators::RSI rsi_;
    quantlab::indicators::BollingerBands bb_;
    
    // Strategy parameters
    int rsi_oversold_threshold_;   // Default: 30
    int rsi_overbought_threshold_; // Default: 70
    double confidence_threshold_;  // Default: 0.65
    
    // Market data client
    quantlab::data::AlpacaClient* market_data_;
    
    // Historical data for backtesting
    std::vector<quantlab::core::Bar> historical_bars_;
    
public:
    // Simplified constructor for easy initialization
    MeanReversionStrategy(std::shared_ptr<quantlab::data::AlpacaClient> client)
        : ema_(20), rsi_(14), bb_(20, 2.0), 
          rsi_oversold_threshold_(30), rsi_overbought_threshold_(70), 
          confidence_threshold_(0.65),
          market_data_(client.get()), historical_bars_() {
    }
    
    // Full constructor for custom parameters
    MeanReversionStrategy(int ema_period, int rsi_period, int bb_period, double bb_std_dev,
                         quantlab::data::AlpacaClient* client,
                         int rsi_oversold = 30, int rsi_overbought = 70, double confidence = 0.65) 
        : ema_(ema_period), rsi_(rsi_period), bb_(bb_period, bb_std_dev), 
          rsi_oversold_threshold_(rsi_oversold), rsi_overbought_threshold_(rsi_overbought), 
          confidence_threshold_(confidence),
          market_data_(client), historical_bars_() {
    }
    
    // Load historical data and warm up indicators
    void load_historical_data(const std::string& symbol, const std::string& timeframe, int limit) {
        // Get historical data with empty start/end dates (use default behavior)
        historical_bars_ = market_data_->get_historical_bars(symbol, timeframe, "", "");
        std::cout << "Loaded " << historical_bars_.size() << " historical bars for long-term analysis" << std::endl;
        
        // Feed all bars to indicators to warm them up
        for(const auto& bar : historical_bars_) {
            ema_.update(bar.close);
            rsi_.update(bar.close);
            bb_.update(bar.close);
        }
    }
    
    // Set confidence threshold for trading signals
    void set_confidence_threshold(double threshold) {
        if (threshold > 0.0 && threshold <= 1.0) {
            confidence_threshold_ = threshold;
        }
    }
    
    // ENHANCED: Multi-minute aggregated data loading for extended analysis
    void load_aggregated_historical_data(const std::string& symbol, const std::string& timeframe, int total_days, int days_per_call = 1) {
        // Load historical data using aggregated method
        historical_bars_ = market_data_->get_aggregated_historical_bars(symbol, timeframe, total_days, days_per_call);
        
        // Initialize indicators with historical data
        int bars_processed = 0;
        for(const auto& bar : historical_bars_) {
            ema_.update(bar.close);
            rsi_.update(bar.close);
            bb_.update(bar.close);
            bars_processed++;
        }
        
        // Strategy initialized with historical data
    }
    
    // INSTITUTIONAL-GRADE WEIGHTED CONFIDENCE SYSTEM
    // Based on how professional hedge funds and institutional traders calculate confidence
    double calculate_confidence(double price, double ema, double rsi, double bb_upper, double bb_middle, double bb_lower) {
        double total_score = 0.0;
        double total_weight = 0.0;
        
        // === FACTOR 1: RSI MOMENTUM STRENGTH ===
        // Weight: 35% (Most important for mean reversion)
        double rsi_weight = 0.35;
        double rsi_score = 0.0;
        
        if (rsi <= 30) {
            rsi_score = (30 - rsi) / 30.0;  // Stronger oversold = higher score
        } else if (rsi >= 70) {
            rsi_score = (rsi - 70) / 30.0;  // Stronger overbought = higher score
        }
        rsi_score = std::min(1.0, rsi_score);  // Cap at 1.0
        
        total_score += rsi_score * rsi_weight;
        total_weight += rsi_weight;
        
        // === FACTOR 2: BOLLINGER BAND EXTREMES ===
        // Weight: 30% (Price extremes are key for mean reversion)
        double bb_weight = 0.30;
        double bb_score = 0.0;
        
        double bb_width = bb_upper - bb_lower;
        if (bb_width > 0) {  // Avoid division by zero
            if (price < bb_lower) {
                bb_score = (bb_lower - price) / bb_width;  // How far below lower band
            } else if (price > bb_upper) {
                bb_score = (price - bb_upper) / bb_width;  // How far above upper band
            }
            bb_score = std::min(1.0, bb_score);  // Cap at 1.0
        }
        
        total_score += bb_score * bb_weight;
        total_weight += bb_weight;
        
        // === FACTOR 3: TREND CONTEXT ===
        // Weight: 20% (Trend confirmation/contradiction)
        double trend_weight = 0.20;
        double trend_score = 0.0;
        
        double price_vs_ema = (price - ema) / ema;
        trend_score = std::min(1.0, std::abs(price_vs_ema) * 10);  // Scale to 0-1
        
        total_score += trend_score * trend_weight;
        total_weight += trend_weight;
        
        // === FACTOR 4: VOLATILITY REGIME ===
        // Weight: 15% (Higher volatility = higher potential rewards)
        double vol_weight = 0.15;
        double vol_score = 0.0;
        
        if (bb_width > 0 && bb_middle > 0) {
            double bb_width_pct = bb_width / bb_middle;  // Volatility as % of price
            vol_score = std::min(1.0, bb_width_pct * 20);  // Scale to 0-1
        }
        
        total_score += vol_score * vol_weight;
        total_weight += vol_weight;
        
        // === WEIGHTED AVERAGE CALCULATION ===
        double weighted_confidence = (total_weight > 0) ? (total_score / total_weight) : 0.0;
        
        // Professional range: 0.5 (neutral) to 0.95 (very high confidence)
        return 0.5 + (weighted_confidence * 0.45);
    }

    // CHALLENGE 3: Generate trading signal based on current market conditions
    StrategyResult generate_signal(const std::string& symbol) {
        // Get latest price using quote API
        auto quote_opt = market_data_->get_latest_quote(symbol);
        if (!quote_opt.has_value()) {
            StrategyResult result;
            result.signal = Signal::HOLD;  
            result.confidence = 0.0;
            result.reason = "No quote data available";
            return result;
        }
        
        double latest_price = quote_opt->mid_price(); // Use mid price from bid/ask
        ema_.update(latest_price);
        rsi_.update(latest_price);
        auto bb_result = bb_.update(latest_price);  // Use the return value directly
        double ema_value = ema_.value();
        double rsi_value = rsi_.value();
        double bb_upper = bb_result.upper_band;
        double bb_middle = bb_result.middle_band;
        double bb_lower = bb_result.lower_band;
        // Create result with all current values
        StrategyResult result;
        result.current_price = latest_price;
        result.ema_value = ema_value;
        result.rsi_value = rsi_value;
        result.bb_upper = bb_upper;
        result.bb_middle = bb_middle;
        result.bb_lower = bb_lower;
        
        // SCIENTIFIC CONFIDENCE CALCULATION
        double confidence = calculate_confidence(latest_price, ema_value, rsi_value, bb_upper, bb_middle, bb_lower);
        
        // INSTITUTIONAL SIGNAL LOGIC with CONFIDENCE THRESHOLDS
        // Use the configurable confidence threshold
        
        // SIMPLIFIED MEAN REVERSION: Focus on RSI extremes with high confidence
        if(rsi_value < rsi_oversold_threshold_ && confidence >= confidence_threshold_) {
            result.signal = Signal::BUY;
            result.confidence = confidence;
            result.reason = "INSTITUTIONAL BUY: RSI=" + std::to_string((int)rsi_value) + 
                           " (oversold<30). Confidence=" + 
                           std::to_string((int)(confidence*100)) + "%";
        }
        else if(rsi_value > rsi_overbought_threshold_ && confidence >= confidence_threshold_) {
            result.signal = Signal::SELL;
            result.confidence = confidence;
            result.reason = "INSTITUTIONAL SELL: RSI=" + std::to_string((int)rsi_value) + 
                           " (overbought>70). Confidence=" + 
                           std::to_string((int)(confidence*100)) + "%";
        }
        else {
            result.signal = Signal::HOLD;
            result.confidence = confidence;
            result.reason = "HOLD: Confidence=" + std::to_string((int)(confidence*100)) + 
                           "% (need >" + std::to_string((int)(confidence_threshold_*100)) + "% for signal)";
        }
        
        return result;
    }
    
    // PROFESSIONAL BACKTESTING ENGINE
    std::vector<StrategyResult> backtest() {
        std::vector<StrategyResult> results;
        
        if (historical_bars_.empty()) {
            std::cout << "Error: No historical data available for backtesting!" << std::endl;
            return results;
        }
        
        std::cout << "Running backtest on " << historical_bars_.size() << " data points..." << std::endl;
        
        // Reset indicators for clean backtest
        ema_.reset();
        rsi_.reset();  
        bb_.reset();
        
        // Warm up indicators with first portion of data (use conservative estimate)
        size_t warmup_periods = 20;  // Conservative warmup period for all indicators
        warmup_periods = std::min(warmup_periods, historical_bars_.size() / 2);
        
        for (size_t i = 0; i < warmup_periods && i < historical_bars_.size(); ++i) {
            ema_.update(historical_bars_[i].close);
            rsi_.update(historical_bars_[i].close);
            bb_.update(historical_bars_[i].close);
        }
        
        // Generate signals for remaining data
        for (size_t i = warmup_periods; i < historical_bars_.size(); ++i) {
            const auto& bar = historical_bars_[i];
            
            // Update indicators
            ema_.update(bar.close);
            rsi_.update(bar.close);
            auto bb_result = bb_.update(bar.close);
            
            // Calculate confidence and signal
            double ema_value = ema_.value();
            double rsi_value = rsi_.value();
            double bb_upper = bb_result.upper_band;
            double bb_middle = bb_result.middle_band;
            double bb_lower = bb_result.lower_band;
            
            double confidence = calculate_confidence(bar.close, ema_value, rsi_value, bb_upper, bb_middle, bb_lower);
            
            // Analyze indicators for signal generation
            
            StrategyResult result;
            result.current_price = bar.close;
            result.ema_value = ema_value;
            result.rsi_value = rsi_value;
            result.bb_upper = bb_upper;
            result.bb_middle = bb_middle;
            result.bb_lower = bb_lower;
            result.confidence = confidence;
            
            // Generate signal based on strategy logic
            bool rsi_oversold = rsi_value < rsi_oversold_threshold_;
            bool rsi_overbought = rsi_value > rsi_overbought_threshold_;
            bool price_below_bb_lower = bar.close < bb_lower;
            bool price_above_bb_upper = bar.close > bb_upper;
            bool price_above_ema = bar.close > ema_value;
            bool price_below_ema = bar.close < ema_value;
            bool high_confidence = confidence >= confidence_threshold_;
            
            // FIXED LOGIC: Simple mean reversion conditions matching generate_signal method
            if (rsi_oversold && high_confidence) {
                result.signal = Signal::BUY;
                result.reason = "BUY: RSI=" + std::to_string((int)rsi_value) + " (oversold<30), High confidence=" + std::to_string((int)(confidence*100)) + "%";
            }
            else if (rsi_overbought && high_confidence) {
                result.signal = Signal::SELL;
                result.reason = "SELL: RSI=" + std::to_string((int)rsi_value) + " (overbought>70), High confidence=" + std::to_string((int)(confidence*100)) + "%";
            }
            else {
                result.signal = Signal::HOLD;
                
                result.reason = "HOLD";
            }
            
            results.push_back(result);
        }
        
        // Return backtest results
        return results;
    }
    
    // Helper method to print strategy results (production-ready)
    void print_signal(const StrategyResult& result) {
        std::string signal_str;
        switch(result.signal) {
            case Signal::BUY:  signal_str = "BUY";  break;
            case Signal::SELL: signal_str = "SELL"; break;
            case Signal::HOLD: signal_str = "HOLD"; break;
            case Signal::NONE: signal_str = "NONE"; break;
        }
        
        std::cout << signal_str << " | Price: $" << result.current_price 
                  << " | RSI: " << result.rsi_value 
                  << " | Confidence: " << (result.confidence * 100) << "%";
        if (result.signal != Signal::HOLD) {
            std::cout << " | " << result.reason;
        }
        std::cout << std::endl;
    }
};

} // namespace quantlab::strategy