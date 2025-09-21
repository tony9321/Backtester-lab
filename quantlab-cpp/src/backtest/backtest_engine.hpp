#pragma once
#include <iostream>
#include <vector>
#include <string>

namespace quantlab::backtest {

struct Trade {
    std::string timestamp;
    std::string action;        // "BUY" or "SELL"
    double price;
    int shares;
    double value;              // price * shares
    double confidence;
    std::string reason;
};

struct BacktestMetrics {
    // Performance Metrics
    double total_return_pct = 0.0;           // Total % return
    double annual_return_pct = 0.0;          // Annualized return
    double sharpe_ratio = 0.0;               // Risk-adjusted return
    double max_drawdown_pct = 0.0;           // Worst losing streak
    
    // Trade Statistics
    int total_trades = 0;
    int winning_trades = 0;
    int losing_trades = 0;
    double win_rate_pct = 0.0;               // % of profitable trades
    double avg_win = 0.0;                    // Average winning trade
    double avg_loss = 0.0;                   // Average losing trade
    double profit_factor = 0.0;              // Total wins / Total losses
    
    // Portfolio Metrics
    double starting_capital = 0.0;
    double ending_capital = 0.0;
    double max_capital = 0.0;                // Peak portfolio value
    double current_position_value = 0.0;     // Value of current holdings
};

struct Portfolio {
    double cash = 100000.0;                  // Starting with $100k
    int shares_held = 0;                     // Current stock position
    double last_buy_price = 0.0;             // Price of last purchase
    std::vector<Trade> trade_history;
    
    // Performance tracking
    std::vector<double> daily_values;        // Portfolio value each day
    double peak_value = 100000.0;            // Highest portfolio value seen
    
    double get_total_value(double current_price) const {
        return cash + (shares_held * current_price);
    }
    
    bool can_buy(double price, int shares) const {
        return cash >= (price * shares);
    }
    
    void execute_buy(double price, int shares, double confidence, const std::string& reason) {
        if (can_buy(price, shares)) {
            double cost = price * shares;
            cash -= cost;
            shares_held += shares;
            last_buy_price = price;
            
            trade_history.push_back({
                "timestamp", "BUY", price, shares, cost, confidence, reason
            });
        }
    }
    
    void execute_sell(double price, int shares, double confidence, const std::string& reason) {
        if (shares_held >= shares) {
            double proceeds = price * shares;
            cash += proceeds;
            shares_held -= shares;
            
            trade_history.push_back({
                "timestamp", "SELL", price, shares, proceeds, confidence, reason
            });
        }
    }
};

class BacktestEngine {
private:
    Portfolio portfolio_;
    BacktestMetrics metrics_;
    
public:
    BacktestEngine(double starting_capital = 100000.0) {
        portfolio_.cash = starting_capital;
        metrics_.starting_capital = starting_capital;
    }
    
    Portfolio& get_portfolio() { return portfolio_; }
    const BacktestMetrics& get_metrics() const { return metrics_; }
    
    void calculate_final_metrics(double final_price);
    void print_results() const;
    void print_trade_summary() const;
};

} // namespace quantlab::backtest