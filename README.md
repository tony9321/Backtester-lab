# Backtester-lab

A quantitative finance learning laboratory for algorithmic trading, technical indicators, and backtesting strategies.

## 🎯 What This Is

This repository contains implementations and experiments in quantitative finance, starting with a C++ high-performance backtesting engine and technical indicator library.

**Learning Goals:**
- Master financial mathematics (stochastic processes, volatility modeling)
- Implement technical indicators with optimal algorithms (O(1) updates)
- Build realistic backtesting with transaction costs and slippage
- Integrate with real market data APIs (Alpaca Markets)
- Apply modern C++ techniques for performance-critical financial systems

## 📁 Current Projects

### [`quantlab-cpp/`](./quantlab-cpp/) - C++ Quantitative Finance Library
A production-ready C++ library implementing:
- **Stochastic Data Generation:** GBM, Mean Reversion, Regime Changes
- **Technical Indicators:** EMA, RSI, Bollinger Bands (O(1) rolling implementations)
- **Strategy Framework:** Composable signals and backtesting engine
- **Market Data:** Alpaca API integration for historical and live data

**[📚 Detailed Learning Guide →](./quantlab-cpp/README.md)**

## 🚀 Getting Started

1. **Prerequisites:** Modern C++ compiler (C++20), CMake 3.20+
2. **Clone and build:**
   ```bash
   git clone <this-repo>
   cd Backtester-lab/quantlab-cpp
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j4
   ```
3. **Run tests:** `ctest`
4. **Start learning:** Open the [detailed guide](./quantlab-cpp/README.md)

## 🎓 Learning Path

1. **[Mathematical Foundations](./quantlab-cpp/README.md#-financial-mathematics-foundations)** - Understand GBM, mean reversion, Itô's lemma
2. **[C++ Performance Techniques](./quantlab-cpp/README.md#-c-performance-techniques-for-quantitative-finance)** - ALU vs FPU, memory optimization, O(1) algorithms
3. **[Technical Indicators](./quantlab-cpp/README.md#-technical-indicators-mathematics)** - EMA, RSI, Bollinger Bands implementation
4. **[Strategy Development](./quantlab-cpp/README.md#-project-architecture)** - Combine indicators into trading signals
5. **[Backtesting & Validation](./quantlab-cpp/README.md#-testing-and-validation)** - Realistic simulation with costs

## ⚠️ Disclaimer

This is an educational project for learning quantitative finance concepts. All strategies and indicators are for learning purposes only. Trading involves risk of loss.

---