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
        
        std::cout << "\n� Loading historical data..." << std::endl;
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
        
        std::cout << "Generated " << buy_signals << " BUY, " << sell_signals << " SELL, " << hold_signals << " HOLD signals" << std::endl;
        
        // Calculate final metrics using current market price (first signal = most recent)
        double final_price = trade_signals.empty() ? 413.51 : trade_signals.front().current_price;
        engine.calculate_final_metrics(final_price);
        
        // Show results
        engine.print_results();
        engine.print_trade_summary();
        
        std::cout << "\nBacktest completed successfully." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
}
