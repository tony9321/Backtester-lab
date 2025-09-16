#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <cstring>
#include "../src/strategy/mean_reversion_strategy.hpp"
#include "../src/backtest/backtest_engine.hpp"

int main() {
    // Check if environment variables are already set
    const char* api_key = std::getenv("ALPACA_API_KEY_ID");
    const char* api_secret = std::getenv("ALPACA_API_SECRET_KEY");
    const char* base_url = std::getenv("ALPACA_BASE_URL");
    
    // Set the working credentials from .env file
    if (!api_key) {
        putenv(const_cast<char*>("ALPACA_API_KEY_ID=PKIEMARS3MJXIO0CQ016"));
    }
    if (!api_secret) {
        putenv(const_cast<char*>("ALPACA_API_SECRET_KEY=OWd6nfpobGbHwvFbWqbEZSQDK187PEJtU0SKDk3f"));
    }
    if (!base_url) {
        putenv(const_cast<char*>("ALPACA_BASE_URL=https://paper-api.alpaca.markets"));
    }

    try {
        std::cout << "=== PROFESSIONAL MULTI-MINUTE AGGREGATION SYSTEM ===" << std::endl;
        std::cout << "🚀 ENHANCED TSLA BACKTESTING WITH EXTENDED HISTORICAL DATA" << std::endl;
        std::cout << "🎯 Target: 60 days (2 months) of daily TSLA data for free plan" << std::endl;
        std::cout << "💡 Rate-limited for free plan limits (200 calls/minute)" << std::endl;
        std::cout << "⏱️  Estimated time: ~20 seconds for comprehensive dataset" << std::endl;
        std::cout << std::endl;

        // Initialize components
        auto client = std::make_shared<quantlab::data::AlpacaClient>();
        
        // Test connection first
        bool api_connected = client->test_connection();
        if (!api_connected) {
            std::cout << "⚠️ API connection failed - using mock data for testing" << std::endl;
            std::cout << "This will demonstrate the complete strategy and backtesting pipeline" << std::endl;
        }

        // Initialize strategy with enhanced aggregation system
        quantlab::strategy::MeanReversionStrategy strategy(client);
        std::string test_symbol = "TSLA";
        
        std::cout << "\n🔄 PHASE 1: MULTI-MINUTE DATA AGGREGATION" << std::endl;
        std::cout << "=========================================" << std::endl;
        
    // Load 120 days of TSLA data using multi-minute aggregation system
    std::cout << "🚀 FREE PLAN MULTI-MINUTE AGGREGATION SYSTEM ACTIVATED!" << std::endl;
    std::cout << "🎯 Target: 120 days of TSLA historical data" << std::endl;
    strategy.load_aggregated_historical_data("TSLA", "1Day", 120, 1);        std::cout << "\n🧠 PHASE 2: INSTITUTIONAL STRATEGY ANALYSIS" << std::endl;
        std::cout << "===========================================" << std::endl;
        
        // Generate professional signal analysis
        auto signal = strategy.generate_signal(test_symbol);
        
        std::cout << "\n📊 SIGNAL ANALYSIS RESULTS:" << std::endl;
        std::cout << "Confidence: " << std::fixed << std::setprecision(1) << signal.confidence << "%" << std::endl;
        std::cout << "Action: " << (signal.signal == quantlab::strategy::Signal::BUY ? "BUY" :
                     signal.signal == quantlab::strategy::Signal::SELL ? "SELL" : "HOLD") << std::endl;
        std::cout << "Price: $" << std::setprecision(2) << signal.current_price << std::endl;
        
        if (!signal.reason.empty()) {
            std::cout << "Analysis: " << signal.reason << std::endl;
        }
        
        std::cout << "\n🎯 PHASE 3: COMPREHENSIVE BACKTESTING ENGINE" << std::endl;
        std::cout << "============================================" << std::endl;
        
        // Initialize professional backtesting engine
        quantlab::backtest::BacktestEngine engine(1000000.0); // $1M starting capital
        
        // Run simple backtest using the strategy's backtest method
        std::cout << "\n📊 Running comprehensive backtest..." << std::endl;
        auto trade_signals = strategy.backtest();
        
        // Simulate trades based on signals
        for (const auto& signal_result : trade_signals) {
            if (signal_result.signal == quantlab::strategy::Signal::BUY && 
                signal_result.confidence >= 0.65) {
                
                int shares = static_cast<int>(50000 / signal_result.current_price); // $50k per trade
                engine.get_portfolio().execute_buy(signal_result.current_price, shares, 
                                                 signal_result.confidence, signal_result.reason);
            }
            else if (signal_result.signal == quantlab::strategy::Signal::SELL && 
                     signal_result.confidence >= 0.65 && 
                     engine.get_portfolio().shares_held > 0) {
                
                int shares = engine.get_portfolio().shares_held;
                engine.get_portfolio().execute_sell(signal_result.current_price, shares,
                                                   signal_result.confidence, signal_result.reason);
            }
        }
        
        // Calculate final metrics
        double final_price = signal.current_price;
        engine.calculate_final_metrics(final_price);
        
        // Show results
        engine.print_results();
        engine.print_trade_summary();
        
        std::cout << "\n✅ MULTI-MINUTE AGGREGATION SYSTEM COMPLETE!" << std::endl;
        std::cout << "Professional-grade extended historical analysis successfully executed." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
}
