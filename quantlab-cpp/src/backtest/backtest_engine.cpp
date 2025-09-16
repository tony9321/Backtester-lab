#include "backtest_engine.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

namespace quantlab::backtest {

void BacktestEngine::calculate_final_metrics(double final_price) {
    metrics_.ending_capital = portfolio_.get_total_value(final_price);
    metrics_.current_position_value = portfolio_.shares_held * final_price;
    
    // Calculate total return
    metrics_.total_return_pct = ((metrics_.ending_capital - metrics_.starting_capital) / metrics_.starting_capital) * 100.0;
    
    // Analyze trades - count all individual transactions
    metrics_.total_trades = portfolio_.trade_history.size();
    metrics_.winning_trades = 0;
    metrics_.losing_trades = 0;
    double total_wins = 0.0;
    double total_losses = 0.0;
    
    // Calculate P&L from complete trading cycles (BUY->SELL pairs)
    double current_position_cost = 0.0;
    int current_shares = 0;
    int completed_trades = 0;
    
    for (const auto& trade : portfolio_.trade_history) {
        if (trade.action == "BUY") {
            current_position_cost += trade.value;
            current_shares += trade.shares;
        } else if (trade.action == "SELL" && current_shares > 0) {
            // Calculate profit/loss for this sell
            double avg_cost_per_share = current_position_cost / current_shares;
            double profit_loss = (trade.price - avg_cost_per_share) * trade.shares;
            
            completed_trades++;
            
            if (profit_loss > 0) {
                metrics_.winning_trades++;
                total_wins += profit_loss;
            } else {
                metrics_.losing_trades++;
                total_losses += std::abs(profit_loss);
            }
            
            // Update position - proportionally reduce cost basis
            double shares_sold_ratio = static_cast<double>(trade.shares) / current_shares;
            current_position_cost -= (current_position_cost * shares_sold_ratio);
            current_shares -= trade.shares;
        }
    }
    
    // Calculate trade statistics
    if (completed_trades > 0) {
        metrics_.win_rate_pct = (static_cast<double>(metrics_.winning_trades) / completed_trades) * 100.0;
        metrics_.avg_win = (metrics_.winning_trades > 0) ? (total_wins / metrics_.winning_trades) : 0.0;
        metrics_.avg_loss = (metrics_.losing_trades > 0) ? (total_losses / metrics_.losing_trades) : 0.0;
        metrics_.profit_factor = (total_losses > 0) ? (total_wins / total_losses) : 0.0;
    }
    
    // Calculate max drawdown from daily values
    metrics_.max_drawdown_pct = 0.0;
    double peak = metrics_.starting_capital;
    
    for (double value : portfolio_.daily_values) {
        if (value > peak) {
            peak = value;
        }
        
        double drawdown = ((peak - value) / peak) * 100.0;
        if (drawdown > metrics_.max_drawdown_pct) {
            metrics_.max_drawdown_pct = drawdown;
        }
    }
    
    // Simple Sharpe ratio calculation (assumes 2% risk-free rate)
    if (metrics_.total_return_pct > 2.0) {
        metrics_.sharpe_ratio = (metrics_.total_return_pct - 2.0) / 15.0;  // Simple approximation
    }
}

void BacktestEngine::print_results() const {
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "BACKTEST RESULTS" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    // Portfolio Summary
    std::cout << "\nPORTFOLIO PERFORMANCE:" << std::endl;
    std::cout << "Starting Capital: $" << std::fixed << std::setprecision(2) << metrics_.starting_capital << std::endl;
    std::cout << "Ending Capital:   $" << std::fixed << std::setprecision(2) << metrics_.ending_capital << std::endl;
    std::cout << "Total Return:     " << std::fixed << std::setprecision(2) << metrics_.total_return_pct << "%" << std::endl;
    
    if (metrics_.total_return_pct > 0) {
        std::cout << "Status: PROFITABLE" << std::endl;
    } else {
        std::cout << "Status: UNPROFITABLE" << std::endl;
    }
    
    // Risk Metrics
    std::cout << "\nRISK METRICS:" << std::endl;
    std::cout << "Max Drawdown:     " << std::fixed << std::setprecision(2) << metrics_.max_drawdown_pct << "%" << std::endl;
    std::cout << "Sharpe Ratio:     " << std::fixed << std::setprecision(2) << metrics_.sharpe_ratio << std::endl;
    
    // Trade Statistics
    std::cout << "\nTRADE ANALYSIS:" << std::endl;
    std::cout << "Total Transactions: " << metrics_.total_trades << std::endl;
    std::cout << "Completed Cycles: " << (metrics_.winning_trades + metrics_.losing_trades) << std::endl;
    std::cout << "Winning Cycles:   " << metrics_.winning_trades << std::endl;
    std::cout << "Losing Cycles:    " << metrics_.losing_trades << std::endl;
    std::cout << "Win Rate:         " << std::fixed << std::setprecision(1) << metrics_.win_rate_pct << "%" << std::endl;
    std::cout << "Average Win:      $" << std::fixed << std::setprecision(2) << metrics_.avg_win << std::endl;
    std::cout << "Average Loss:     $" << std::fixed << std::setprecision(2) << metrics_.avg_loss << std::endl;
    std::cout << "Profit Factor:    " << std::fixed << std::setprecision(2) << metrics_.profit_factor << std::endl;
    
    // Current Position
    std::cout << "\nCURRENT POSITION:" << std::endl;
    std::cout << "Cash:             $" << std::fixed << std::setprecision(2) << portfolio_.cash << std::endl;
    std::cout << "Shares Held:      " << portfolio_.shares_held << std::endl;
    std::cout << "Position Value:   $" << std::fixed << std::setprecision(2) << metrics_.current_position_value << std::endl;
}

void BacktestEngine::print_trade_summary() const {
    if (portfolio_.trade_history.empty()) {
        std::cout << "\nNo trades executed during backtest period." << std::endl;
        return;
    }
    
    std::cout << "\nRECENT TRADES:" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    // Show last 10 trades
    int start_idx = std::max(0, static_cast<int>(portfolio_.trade_history.size()) - 10);
    
    for (size_t i = start_idx; i < portfolio_.trade_history.size(); ++i) {
        const auto& trade = portfolio_.trade_history[i];
        
        std::string action_symbol = (trade.action == "BUY") ? "🟢 BUY " : "🔴 SELL";
        
        std::cout << action_symbol << " " << trade.shares << " shares @ $" 
                  << std::fixed << std::setprecision(2) << trade.price 
                  << " | Value: $" << std::fixed << std::setprecision(2) << trade.value
                  << " | Conf: " << std::fixed << std::setprecision(0) << (trade.confidence * 100) << "%"
                  << std::endl;
        std::cout << "   Reason: " << trade.reason << std::endl;
    }
}

} // namespace quantlab::backtest