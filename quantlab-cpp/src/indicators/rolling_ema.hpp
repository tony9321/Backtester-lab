#pragma once
// Exponential Moving Average (EMA) indicator
// EMA_today = α * Price_today + (1 - α) * EMA_yesterday
// α = 2 / (N + 1) where N is the number of periods
// More weight to recent prices, reacts faster than SMA
// Common periods: 12, 26 (short-term), 50, 200 (long-term)
// Used in MACD, trend identification, support/resistance
// new price gets weight alpha, previous EMA gets weight (1-alpha)
// if current_price>EMA → uptrend, else downtrend
namespace quantlab::indicators {

class RollingEMA
{
private:
    double smoothing_factor(int period){
        return 2.0/(period+1);
    }
    double alpha_;
    bool initialized_;
    double current_ema_=0.0;// yesterday's ema
public:
    explicit RollingEMA(int period):alpha_(smoothing_factor(period)), initialized_(false), current_ema_(0.0)
    {

    }
    double update(double price)
    {
        if(!is_initialized())
        {
            current_ema_=price; // first price becomes initial EMA
            initialized_=true;
        }
        else  current_ema_=alpha_*price+(1-alpha_)*current_ema_;
        return current_ema_;
    }
    double value() const{
        return current_ema_;
    }
    void reset()
    {
        current_ema_=0.0;
        initialized_=false;
    }
    bool is_initialized() const {
        return initialized_;
    }
};
}