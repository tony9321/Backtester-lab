#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <cstring>
#include <string>
#include <algorithm>
#include "../src/strategy/mean_reversion_strategy.hpp"
#include "../src/backtest/backtest_engine.hpp"

int main(int argc, char* argv[]) {
    // Parse command line arguments for symbol (default: TSLA)
    std::string symbol = "TSLA";
    if (argc > 1) {
        symbol = argv[1];
        std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
    }
    int days=120; //default
    if(argc>2)
    {
        days=std::atoi(argv[2]);
        if(days<=30) days=120; //reset to default if invalid
    }
    double confidence_threshold = 0.65; // 65% confidence threshold for trades
    if(argc>3)
    {
        confidence_threshold=std::atof(argv[3]);
        if(confidence_threshold<=0.0 || confidence_threshold>1.0) confidence_threshold=0.65; //reset to default if invalid
    }
    
    // Additional parameters for strategy configuration
    int oversold_threshold = 30; // Default RSI oversold threshold
    int overbought_threshold = 70; // Default RSI overbought threshold
    if(argc>4) oversold_threshold = std::atoi(argv[4]);
    if(argc>5) overbought_threshold = std::atoi(argv[5]);
    // Environment variables should be set via .env file or shell
    // No hardcoded credentials for security

    try {
        std::cout << "QUANTLAB BACKTESTING ENGINE" << std::endl;
        std::cout << symbol << " Mean Reversion Strategy Analysis" << std::endl;
        std::cout << "Days: " << days << " | Confidence Threshold: " << (confidence_threshold * 100) << "%" << std::endl;

        // Initialize components
        auto client = std::make_shared<quantlab::data::AlpacaClient>();
        
        // Test connection
        client->test_connection();

        // Initialize strategy with enhanced aggregation system
        quantlab::strategy::MeanReversionStrategy strategy(client);
        strategy.set_confidence_threshold(confidence_threshold);
        std::string test_symbol = symbol;
        
        std::cout << "\n📊 Loading historical data..." << std::endl;
        strategy.load_aggregated_historical_data(symbol, "1Day", days, 1);
        
        std::cout << "Analyzing strategy signals..." << std::endl;
        
        // Generate professional signal analysis
        auto signal = strategy.generate_signal(test_symbol);
        
        std::cout << "\nCurrent Signal: " << (signal.signal == quantlab::strategy::Signal::BUY ? "BUY" :
                     signal.signal == quantlab::strategy::Signal::SELL ? "SELL" : "HOLD")
                  << " at $" << std::setprecision(2) << signal.current_price 
                  << " (Confidence: " << std::setprecision(1) << signal.confidence << "%)" << std::endl;
        
        std::cout << "\nRunning backtest..." << std::endl;
        
        // Initialize backtesting engine
        quantlab::backtest::BacktestEngine engine(1000000.0); // $1M starting capital
        auto trade_signals = strategy.backtest();
        
        // Execute trades based on signals
        int buy_signals = 0, sell_signals = 0, hold_signals = 0;
        
        for (const auto& signal_result : trade_signals) {
            // Count signal types
            if (signal_result.signal == quantlab::strategy::Signal::BUY) buy_signals++;
            else if (signal_result.signal == quantlab::strategy::Signal::SELL) sell_signals++;
            else hold_signals++;
            
            // Execute trades with configurable confidence threshold
            if (signal_result.signal == quantlab::strategy::Signal::BUY && 
                signal_result.confidence >= confidence_threshold) {
                
                int shares = static_cast<int>(50000 / signal_result.current_price); // $50k per trade
                engine.get_portfolio().execute_buy(signal_result.current_price, shares, 
                                                 signal_result.confidence, signal_result.reason);
            }
            else if (signal_result.signal == quantlab::strategy::Signal::SELL && 
                     signal_result.confidence >= confidence_threshold) {
                
                if (engine.get_portfolio().shares_held > 0) {
                    // Sell $50k worth of shares
                    int shares_to_sell = static_cast<int>(50000 / signal_result.current_price);
                    int shares = std::min(shares_to_sell, engine.get_portfolio().shares_held);
                    engine.get_portfolio().execute_sell(signal_result.current_price, shares,
                                                       signal_result.confidence, signal_result.reason);
                }
            }
        }
        
        // Signal counts calculated silently
        
        // Calculate final metrics using current market price (first signal = most recent)
        double final_price = trade_signals.empty() ? 413.51 : trade_signals.front().current_price;
        engine.calculate_final_metrics(final_price);
        
        // Get final metrics
        const auto& metrics = engine.get_metrics();
        const auto& portfolio = engine.get_portfolio();
        
        // Output JSON results for web integration
        std::cout << "\n{" << std::endl;
        std::cout << "  \"success\": true,\n";
        std::cout << "  \"timestamp\": \"" << __DATE__ << "T" << __TIME__ << "\",\n";
        std::cout << "  \"optimization_results\": [\n";
        std::cout << "    {\n";
        std::cout << "      \"symbol\": \"" << symbol << "\",\n";
        std::cout << "      \"rsi_period_min\": 14,\n";
        std::cout << "      \"rsi_period_max\": 14,\n";
        std::cout << "      \"oversold_threshold\": " << oversold_threshold << ",\n";
        std::cout << "      \"overbought_threshold\": " << overbought_threshold << ",\n";
        std::cout << "      \"total_return\": " << std::fixed << std::setprecision(4) << (metrics.total_return_pct / 100.0) << ",\n";
        std::cout << "      \"total_return_pct\": " << std::fixed << std::setprecision(2) << metrics.total_return_pct << ",\n";
        std::cout << "      \"max_drawdown\": " << std::fixed << std::setprecision(4) << (-metrics.max_drawdown_pct / 100.0) << ",\n";
        std::cout << "      \"sharpe_ratio\": " << std::fixed << std::setprecision(4) << metrics.sharpe_ratio << ",\n";
        std::cout << "      \"total_trades\": " << metrics.total_trades << ",\n";
        std::cout << "      \"winning_trades\": " << metrics.winning_trades << ",\n";
        std::cout << "      \"win_rate\": " << std::fixed << std::setprecision(2) << metrics.win_rate_pct << ",\n";
        std::cout << "      \"profit_factor\": " << std::fixed << std::setprecision(2) << metrics.profit_factor << "\n";
        std::cout << "    }\n";
        std::cout << "  ],\n";
        std::cout << "  \"summary\": {\n";
        std::cout << "    \"total_combinations\": 1,\n";
        std::cout << "    \"best_return\": " << std::fixed << std::setprecision(4) << (metrics.total_return_pct / 100.0) << ",\n";
        std::cout << "    \"avg_trades\": " << metrics.total_trades << "\n";
        std::cout << "  }\n";
        std::cout << "}" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
}
