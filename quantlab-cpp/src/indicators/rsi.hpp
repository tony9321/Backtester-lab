#pragma once

#include "rolling_ema.hpp"

namespace quantlab::indicators {

class RSI {
private:
    // QUESTION 1: What member variables do you need?
    // Think about:
    // - Two EMA objects for gains and losses?
    // - Previous price to calculate changes?
    // - Current RSI value?
    // - Initialization state?
    RollingEMA gains_ema_; // EMA of gains
    RollingEMA losses_ema_; // EMA of losses
    double previous_price_;
    double current_rsi_=50.0; // RSI starts neutral
    bool initialized_=false; // Have we seen the first price yet?
public:
    // QUESTION 2: Constructor
    explicit RSI(int period = 14): gains_ema_(period), losses_ema_(period), previous_price_(0.0), current_rsi_(50.0), initialized_(false) {
        // How do you initialize the member variables?
        // How do you create the EMA objects?
    }
    
    // QUESTION 3: The core RSI algorithm
    double update(double price) {
        // How do you handle the first price?
        // How do you calculate price changes?
        // How do you separate gains from losses?
        // How do you update the EMAs?
        // How do you calculate the final RSI value?
        if(!is_initialized())
        {
            previous_price_=price; // first price, just store it
            initialized_=true;
            return current_rsi_; // RSI is neutral at start
        }
        double change = price-previous_price_;
        double gain = (change > 0) ? change : 0.0;
        double loss = (change < 0) ? -change : 0.0;  // Make positive!
        gains_ema_.update(gain);
        losses_ema_.update(loss);
        double avg_gain = gains_ema_.value();
        double avg_loss = losses_ema_.value();
        if(avg_loss==0.0&&avg_gain==0.0) current_rsi_=50.0; // if no change, RSI is neutral;
        else if(avg_loss==0.0) 
            current_rsi_=100.0; // prevent division by 0
        else 
        {   
            double rs=avg_gain/avg_loss;
            current_rsi_=100.0-(100./(1.0+rs));
        }
        previous_price_=price; // store for the next call
        return current_rsi_;
    }
    
    // QUESTION 4: Simple getters and utilities
    double value() const {
        // Return current RSI value
        return current_rsi_;
    }
    
    void reset() {
        // Reset all internal state
        gains_ema_.reset();
        losses_ema_.reset();
        previous_price_=0.0;
        current_rsi_=50.0;
        initialized_=false;
    }
    
    bool is_initialized() const {
        // Check if RSI has enough data
        return initialized_;
    }
};

} // namespace quantlab::indicators