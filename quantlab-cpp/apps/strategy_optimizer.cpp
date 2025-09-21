#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <thread>
#include <set>
#include "../src/strategy/mean_reversion_strategy.hpp"
#include "../src/backtest/backtest_engine.hpp"

namespace quantlab::optimization {

/**
 * Parameter set for strategy optimization
 */
struct ParameterSet {
    std::string symbol;
    int days;
    double confidence_threshold;
    
    ParameterSet(const std::string& sym, int d, double conf) 
        : symbol(sym), days(d), confidence_threshold(conf) {}
};

/**
 * Optimization result for a single parameter set
 */
struct OptimizationResult {
    ParameterSet parameters;
    double total_return;
    double max_drawdown;
    double sharpe_ratio;
    int total_trades;
    int winning_trades;
    double win_rate;
    double profit_factor;
    
    OptimizationResult(const ParameterSet& params) 
        : parameters(params), total_return(0.0), max_drawdown(0.0), 
          sharpe_ratio(0.0), total_trades(0), winning_trades(0), 
          win_rate(0.0), profit_factor(0.0) {}
};

/**
 * Strategy Optimizer - Automated Parameter Sweep Framework
 * 
 * This class performs systematic testing of strategy parameters across:
 * - Multiple symbols (AAPL, TSLA, NVDA, etc.)
 * - Different time periods (30-365 days)
 * - Various confidence thresholds (0.3-0.9)
 * 
 * Features:
 * - Parallel parameter testing
 * - Performance metrics collection
 * - CSV export for analysis
 * - Progress tracking
 */
class StrategyOptimizer {
private:
    std::shared_ptr<quantlab::data::AlpacaClient> client_;
    std::vector<ParameterSet> parameter_grid_;
    std::vector<OptimizationResult> results_;
    
public:
    StrategyOptimizer(std::shared_ptr<quantlab::data::AlpacaClient> client) 
        : client_(client) {
    }
    
    // Build parameter grid for optimization
    void build_parameter_grid(const std::vector<std::string>& symbols,
                             const std::vector<int>& days_range,
                             const std::vector<double>& confidence_range) {
        parameter_grid_.clear();
        
        for (const auto& symbol : symbols) {
            for (int days : days_range) {
                for (double confidence : confidence_range) {
                    parameter_grid_.emplace_back(symbol, days, confidence);
                }
            }
        }
        
        std::cout << "📊 Built parameter grid with " << parameter_grid_.size() 
                  << " combinations" << std::endl;
        std::cout << "Symbols: " << symbols.size() 
                  << " | Days: " << days_range.size() 
                  << " | Confidence: " << confidence_range.size() << std::endl;
    }
    
    // Run optimization across all parameter combinations
    void run_optimization() {
        results_.clear();
        results_.reserve(parameter_grid_.size());
        
        std::cout << "\n🚀 Starting optimization run..." << std::endl;
        std::cout << "Total combinations to test: " << parameter_grid_.size() << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < parameter_grid_.size(); ++i) {
            const auto& params = parameter_grid_[i];
            
            // Progress indicator
            if (i % 10 == 0 || i == parameter_grid_.size() - 1) {
                double progress = (double)(i + 1) / parameter_grid_.size() * 100.0;
                std::cout << "Progress: " << std::fixed << std::setprecision(1) 
                          << progress << "% (" << (i + 1) << "/" 
                          << parameter_grid_.size() << ")" << std::endl;
            }
            
            // Run single backtest
            auto result = run_single_backtest(params);
            results_.push_back(result);
            
            // Rate limiting - small delay between tests
            if (i < parameter_grid_.size() - 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
        
        std::cout << "\n✅ Optimization completed in " << duration.count() 
                  << " seconds" << std::endl;
        std::cout << "Generated " << results_.size() << " optimization results" << std::endl;
    }
    
    // Run backtest for single parameter set
    OptimizationResult run_single_backtest(const ParameterSet& params) {
        OptimizationResult result(params);
        
        try {
            // Initialize strategy with parameters
            quantlab::strategy::MeanReversionStrategy strategy(client_);
            strategy.set_confidence_threshold(params.confidence_threshold);
            
            // Load historical data (suppress output for cleaner optimization logs)
            strategy.load_aggregated_historical_data(params.symbol, "1Day", params.days, 1);
            
            // Run backtest
            auto trade_signals = strategy.backtest();
            
            // Initialize backtesting engine
            quantlab::backtest::BacktestEngine engine(1000000.0); // $1M starting capital
            
            // Execute trades and collect metrics
            int buy_signals = 0, sell_signals = 0;
            
            for (const auto& signal_result : trade_signals) {
                if (signal_result.signal == quantlab::strategy::Signal::BUY) buy_signals++;
                else if (signal_result.signal == quantlab::strategy::Signal::SELL) sell_signals++;
                
                // Execute trades with configured confidence threshold
                if (signal_result.signal == quantlab::strategy::Signal::BUY && 
                    signal_result.confidence >= params.confidence_threshold) {
                    
                    int shares = static_cast<int>(50000 / signal_result.current_price);
                    engine.get_portfolio().execute_buy(signal_result.current_price, shares, 
                                                     signal_result.confidence, signal_result.reason);
                }
                else if (signal_result.signal == quantlab::strategy::Signal::SELL && 
                         signal_result.confidence >= params.confidence_threshold) {
                    
                    if (engine.get_portfolio().shares_held > 0) {
                        int shares_to_sell = static_cast<int>(50000 / signal_result.current_price);
                        int shares = std::min(shares_to_sell, engine.get_portfolio().shares_held);
                        engine.get_portfolio().execute_sell(signal_result.current_price, shares,
                                                           signal_result.confidence, signal_result.reason);
                    }
                }
            }
            
            // Calculate final metrics
            double final_price = trade_signals.empty() ? 100.0 : trade_signals.front().current_price;
            engine.calculate_final_metrics(final_price);
            
            // Extract performance metrics
            const auto& portfolio = engine.get_portfolio();
            const auto& metrics = engine.get_metrics();
            
            result.total_return = ((portfolio.cash + portfolio.shares_held * final_price) - 1000000.0) / 1000000.0;
            result.total_trades = portfolio.trade_history.size();
            result.winning_trades = metrics.winning_trades;
            result.win_rate = metrics.win_rate_pct;
            result.profit_factor = metrics.profit_factor;
            result.max_drawdown = metrics.max_drawdown_pct;
            result.sharpe_ratio = metrics.sharpe_ratio;
            
        } catch (const std::exception& e) {
            std::cerr << "❌ Error testing " << params.symbol << " " << params.days 
                      << " days, " << (params.confidence_threshold * 100) << "% confidence: " 
                      << e.what() << std::endl;
        }
        
        return result;
    }
    
    // Export results to CSV file
    void export_to_csv(const std::string& filename) {
        std::ofstream file(filename);
        
        if (!file.is_open()) {
            std::cerr << "❌ Error: Could not open " << filename << " for writing" << std::endl;
            return;
        }
        
        // CSV header
        file << "Symbol,Days,Confidence_Threshold,Total_Return,Max_Drawdown,Sharpe_Ratio,"
             << "Total_Trades,Winning_Trades,Win_Rate,Profit_Factor\n";
        
        // Data rows
        for (const auto& result : results_) {
            file << result.parameters.symbol << ","
                 << result.parameters.days << ","
                 << std::fixed << std::setprecision(3) << result.parameters.confidence_threshold << ","
                 << std::fixed << std::setprecision(4) << result.total_return << ","
                 << std::fixed << std::setprecision(4) << result.max_drawdown << ","
                 << std::fixed << std::setprecision(4) << result.sharpe_ratio << ","
                 << result.total_trades << ","
                 << result.winning_trades << ","
                 << std::fixed << std::setprecision(2) << result.win_rate << ","
                 << std::fixed << std::setprecision(2) << result.profit_factor << "\n";
        }
        
        file.close();
        std::cout << "📄 Results exported to " << filename << std::endl;
    }
    
    // Export results to JSON file (web-friendly)
    void export_to_json(const std::string& filename) {
        std::ofstream file(filename);
        
        if (!file.is_open()) {
            std::cerr << "❌ Error: Could not open " << filename << " for writing" << std::endl;
            return;
        }
        
        file << "{\n";
        file << "  \"optimization_results\": [\n";
        
        for (size_t i = 0; i < results_.size(); ++i) {
            const auto& result = results_[i];
            file << "    {\n";
            file << "      \"symbol\": \"" << result.parameters.symbol << "\",\n";
            file << "      \"days\": " << result.parameters.days << ",\n";
            file << "      \"confidence_threshold\": " << std::fixed << std::setprecision(3) << result.parameters.confidence_threshold << ",\n";
            file << "      \"total_return\": " << std::fixed << std::setprecision(4) << result.total_return << ",\n";
            file << "      \"total_return_pct\": " << std::fixed << std::setprecision(2) << (result.total_return * 100) << ",\n";
            file << "      \"max_drawdown\": " << std::fixed << std::setprecision(4) << result.max_drawdown << ",\n";
            file << "      \"sharpe_ratio\": " << std::fixed << std::setprecision(4) << result.sharpe_ratio << ",\n";
            file << "      \"total_trades\": " << result.total_trades << ",\n";
            file << "      \"winning_trades\": " << result.winning_trades << ",\n";
            file << "      \"win_rate\": " << std::fixed << std::setprecision(2) << result.win_rate << ",\n";
            file << "      \"profit_factor\": " << std::fixed << std::setprecision(2) << result.profit_factor << "\n";
            file << "    }";
            if (i < results_.size() - 1) file << ",";
            file << "\n";
        }
        
        file << "  ],\n";
        file << "  \"summary\": {\n";
        file << "    \"total_combinations\": " << results_.size() << ",\n";
        file << "    \"symbols_tested\": [";
        
        // Get unique symbols
        std::set<std::string> unique_symbols;
        for (const auto& result : results_) {
            unique_symbols.insert(result.parameters.symbol);
        }
        
        auto it = unique_symbols.begin();
        for (size_t i = 0; i < unique_symbols.size(); ++i) {
            file << "\"" << *it << "\"";
            if (i < unique_symbols.size() - 1) file << ", ";
            ++it;
        }
        
        file << "],\n";
        file << "    \"date_generated\": \"" << __DATE__ << "\"\n";
        file << "  }\n";
        file << "}\n";
        
        file.close();
        std::cout << "📄 Web-friendly JSON results exported to " << filename << std::endl;
    }
    
    // Print top performing parameter combinations
    void print_top_results(int top_n = 10) {
        if (results_.empty()) {
            std::cout << "No results to display" << std::endl;
            return;
        }
        
        // Sort by total return (descending)
        auto sorted_results = results_;
        std::sort(sorted_results.begin(), sorted_results.end(),
                  [](const OptimizationResult& a, const OptimizationResult& b) {
                      return a.total_return > b.total_return;
                  });
        
        std::cout << "\n📈 TOP " << std::min(top_n, (int)sorted_results.size()) 
                  << " PERFORMING PARAMETER COMBINATIONS:\n";
        std::cout << std::string(100, '=') << std::endl;
        std::cout << std::left << std::setw(8) << "Symbol" 
                  << std::setw(6) << "Days" 
                  << std::setw(10) << "Conf%" 
                  << std::setw(12) << "Return%" 
                  << std::setw(8) << "Trades" 
                  << std::setw(10) << "Win%" 
                  << std::setw(12) << "Profit" << std::endl;
        std::cout << std::string(100, '-') << std::endl;
        
        for (int i = 0; i < std::min(top_n, (int)sorted_results.size()); ++i) {
            const auto& result = sorted_results[i];
            std::cout << std::left << std::setw(8) << result.parameters.symbol
                      << std::setw(6) << result.parameters.days
                      << std::setw(10) << std::fixed << std::setprecision(1) 
                      << (result.parameters.confidence_threshold * 100)
                      << std::setw(12) << std::fixed << std::setprecision(2) 
                      << (result.total_return * 100)
                      << std::setw(8) << result.total_trades
                      << std::setw(10) << std::fixed << std::setprecision(1) << result.win_rate
                      << std::setw(12) << std::fixed << std::setprecision(2) << result.profit_factor
                      << std::endl;
        }
        std::cout << std::string(100, '=') << std::endl;
    }
    
    // Get optimization results
    const std::vector<OptimizationResult>& get_results() const {
        return results_;
    }
};

} // namespace quantlab::optimization

int main(int argc, char* argv[]) {
    try {
        std::cout << "🎯 QUANTLAB STRATEGY OPTIMIZER" << std::endl;
        std::cout << "Automated Parameter Sweep Framework\n" << std::endl;
        
        // Initialize API client
        auto client = std::make_shared<quantlab::data::AlpacaClient>();
        client->test_connection();
        
        // Initialize optimizer
        quantlab::optimization::StrategyOptimizer optimizer(client);
        
        // Define focused parameter ranges for web-friendly testing
        std::vector<std::string> symbols = {"AAPL"};  // Focus on 1 symbol for validation
        std::vector<int> days_range = {60, 120, 365};         // 3 meaningful time periods
        std::vector<double> confidence_range = {0.5, 0.65, 0.8}; // 3 confidence levels
        
        // Build parameter grid (1 * 3 * 3 = 9 combinations)
        optimizer.build_parameter_grid(symbols, days_range, confidence_range);
        
        // Run optimization
        optimizer.run_optimization();
        
        // Display top results
        optimizer.print_top_results(10);
        
        // Export to both CSV and JSON
        std::string csv_filename = "optimization_results.csv";
        std::string json_filename = "optimization_results.json";
        optimizer.export_to_csv(csv_filename);
        optimizer.export_to_json(json_filename);
        
        std::cout << "\n✅ Strategy optimization completed successfully!" << std::endl;
        std::cout << "📊 Data available in CSV format for analysis and JSON format for web integration" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
}