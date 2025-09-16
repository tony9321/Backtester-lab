# QuantLab-CPP: Quantitative Finance Learning Lab

A C++ quantitative finance library for learning algorithmic trading, technical indicators, and backtesting strategies. Built with production-ready architecture but designed for educational exploration.

## 🎯 Project Overview

This is a step-by-step learning journey through quantitative finance concepts, implemented in modern C++ with performance optimizations used in real trading systems.

**Learning Path:**
1. **Core Math & Data Structures** → Understand financial time series and OHLCV data
2. **Synthetic Data Generation** → Master stochastic processes (GBM, Mean Reversion)  
3. **Technical Indicators** → Implement EMA, RSI, Bollinger Bands with O(1) updates
4. **Strategy Framework** → Combine indicators into trading signals
5. **Backtesting Engine** → Simulate trading with realistic costs and slippage
6. **Live Data Integration** → Connect to Alpaca Markets API for real-time data

---

## 📊 Financial Mathematics Foundations

### Key Symbols and Definitions

| Symbol | Name | Meaning | Example |
|--------|------|---------|---------|
| **μ** (mu) | **Drift/Mean** | Expected return rate (annual) | μ = 0.08 = 8% expected annual return |
| **σ** (sigma) | **Volatility/Standard Deviation** | Measure of price uncertainty (annual) | σ = 0.20 = 20% annual volatility |
| **α** (alpha) | **Smoothing Factor** | Weight for new vs old data in EMA | α = 2/(N+1) for N-period EMA |
| **θ** (theta) | **Reversion Speed** | How fast prices return to mean | θ = 2.0 = strong mean reversion |
| **dt** | **Time Increment** | Fraction of year per time step | dt = 1/252 for daily data |
| **Z** | **Random Shock** | Standard normal random variable | Z ~ N(0,1) |

### Financial Time Series Models

#### 1. **Geometric Brownian Motion (GBM)** - Classic Stock Price Model
```cpp
// Mathematical Formula:
// dS = μS dt + σS dW
// Solution: S(t) = S(0) × exp((μ - σ²/2)t + σ√t × Z)

// Code Implementation:
price *= std::exp((drift - 0.5*volatility*volatility)*dt + volatility*sqrt(dt)*z);
```

**Key Insights:**
- **Used for:** Stock prices, indices (S&P 500, NASDAQ)
- **Assumptions:** Constant drift and volatility, log-normal price distribution
- **Itô Correction:** The `σ²/2` term corrects for convexity (Jensen's inequality)

**Example:**
- μ = 8%, σ = 15% → Expected annual return = exp(0.08 - 0.5×0.15²) - 1 = **7.12%** (not 8%!)
- The 0.88% difference is "volatility drag" - higher volatility reduces compound returns

#### 2. **Ornstein-Uhlenbeck (Mean Reversion)** - Pairs Trading & Commodities
```cpp
// Mathematical Formula:
// dX = θ(μ - X)dt + σdW
// Solution: X pulls back toward mean μ with speed θ

// Code Implementation:
price += reversion_speed * (mean_price - price) * dt + volatility*sqrt(dt)*z;
```

**Key Insights:**
- **Used for:** Interest rates, commodity spreads, pairs trading
- **Behavior:** Price has "memory" of long-term average
- **θ parameter:** Higher values = faster reversion to mean

#### 3. **Trending with Regime Changes** - Bull/Bear Market Cycles
```cpp
// Switches between uptrend and downtrend every N periods
if (i % regime_change_every == 0) {
    current_trend = -current_trend; // Bull → Bear or Bear → Bull
}
```

**Key Insights:**
- **Used for:** Testing adaptive strategies that handle market regime changes
- **Real-world analogy:** 1990s tech boom → 2000s crash → 2010s recovery

---

## 💻 C++ Performance Techniques for Quantitative Finance

### 1. **Time Representation: Why `int64_t` vs `double`**

```cpp
// ❌ WRONG: Double timestamps lose precision
double timestamp = 1735689600.123456789;  // Only ~15-16 decimal digits
double next_tick = timestamp + 0.000001;  // Microsecond precision lost!

// ✅ CORRECT: Nanosecond precision with integers  
int64_t timestamp_ns = 1735689600123456789LL;  // Exact nanosecond precision
int64_t next_tick_ns = timestamp_ns + 1000LL;   // Perfect microsecond precision
```

**Why This Matters:**
- **High-Frequency Trading:** Microsecond differences determine order priority
- **Performance:** ALU (integer) operations are 20-30% faster than FPU (floating-point)
- **Precision:** With 10M ticks/day, double timestamps accumulate rounding errors

### 2. **Memory Optimization Patterns**

```cpp
// ❌ WRONG: Vector reallocations during data loading
std::vector<Bar> bars;
for (int i = 0; i < 1000000; ++i) {
    bars.push_back(bar);  // Triggers reallocations!
}

// ✅ CORRECT: Pre-allocate memory
std::vector<Bar> bars;
bars.reserve(1000000);  // One allocation, faster loading
for (int i = 0; i < 1000000; ++i) {
    bars.push_back(bar);  // No reallocations
}
```

### 3. **Rolling Statistics: O(1) vs O(N) Algorithms**

```cpp
// ❌ NAIVE: O(N) Simple Moving Average
double calculate_sma(const std::vector<double>& prices, int period) {
    double sum = 0.0;
    for (int i = 0; i < period; ++i) {
        sum += prices[prices.size() - 1 - i];  // Recalculate every time
    }
    return sum / period;
}

// ✅ OPTIMIZED: O(1) Rolling SMA using circular buffer
class RollingSMA {
    std::deque<double> window;
    double sum;
    int period;
public:
    double update(double price) {
        if (window.size() == period) {
            sum -= window.front();  // Remove old price
            window.pop_front();
        }
        window.push_back(price);    // Add new price
        sum += price;
        return sum / window.size();
    }
};
```

---

## 📈 Technical Indicators Mathematics

### **Exponential Moving Average (EMA)**

**Formula:**
```
EMA(t) = α × Price(t) + (1-α) × EMA(t-1)
where α = 2/(N+1) and N = period
```

**Implementation Strategy:**
```cpp
class RollingEMA {
private:
    double alpha;          // Smoothing factor: 2/(period+1)
    double current_ema;    // Current EMA value
    bool initialized;      // Has received first price?
    
public:
    explicit RollingEMA(int period) 
        : alpha(2.0 / (period + 1.0))
        , current_ema(0.0)
        , initialized(false) {}
    
    double update(double price) {
        if (!initialized) {
            current_ema = price;  // First price = initial EMA
            initialized = true;
        } else {
            current_ema = alpha * price + (1.0 - alpha) * current_ema;
        }
        return current_ema;
    }
};
```

**Why EMA over SMA?**
- ✅ **O(1) update time** (vs O(N) for naive SMA)
- ✅ **More responsive** to recent price changes
- ✅ **Foundation for MACD** = EMA(12) - EMA(26)

### **RSI (Relative Strength Index)**

**Formula:**
```
RSI = 100 - (100 / (1 + RS))
where RS = Average_Gain / Average_Loss over N periods
```

**Interpretation:**
- **RSI > 70:** Potentially overbought (sell signal)
- **RSI < 30:** Potentially oversold (buy signal)
- **RSI ≈ 50:** Neutral momentum

### **Bollinger Bands**

**Formula:**
```
Middle Band = SMA(20)
Upper Band = SMA(20) + 2×StdDev(20)
Lower Band = SMA(20) - 2×StdDev(20)
```

**Strategy Example:**
```cpp
// Mean reversion strategy
if (price <= lower_band && rsi < 30) {
    // Price touched lower band + oversold → BUY
}
if (price >= upper_band && rsi > 70) {
    // Price touched upper band + overbought → SELL
}
```

---

## 🏗️ Project Architecture

### Directory Structure
```
quantlab-cpp/
├── 🔧 CMakeLists.txt           # Modern C++20, performance flags
├── 🔒 .env.template           # Alpaca API credentials (never commit real keys!)
├── ⚙️  config/
│   └── alpaca.example.json    # Trading parameters, symbols, commissions
├── 📁 src/
│   ├── core/                  # Data types (Bar, TimeSeries)
│   ├── synthetic/             # Stochastic process generators
│   ├── indicators/            # EMA, RSI, Bollinger (O(1) implementations)
│   ├── io/                    # HTTP client, file I/O, environment loading
│   ├── data/                  # Alpaca Markets API integration
│   ├── signals/               # Strategy logic (combine indicators)
│   └── engine/                # Backtesting with realistic costs
├── 🛠️ apps/
│   ├── fetch-bars/           # Download historical data
│   └── backtest-cli/         # Run strategies, output results
└── 🧪 tests/                 # Unit tests with known test vectors
```

### Core Data Structures

```cpp
// OHLCV Bar - Standard financial time series representation
struct Bar {
    int64_t timestamp_ns;      // Nanosecond precision timestamp
    double open, high, low, close;  // Price data
    int64_t volume;           // Shares traded
    
    // Invariants: low ≤ min(open, close) ≤ max(open, close) ≤ high
};

// Time series container optimized for streaming updates
template<typename T>
class TimeSeries {
    std::vector<T> data_;
public:
    void reserve(size_t capacity);     // Pre-allocate for performance
    const T& operator[](size_t idx) const;
    std::vector<T> tail(size_t n) const;  // Get last N elements (for indicators)
};
```

---

## 💰 Key Financial Concepts Explained

### **Nominal vs Real Returns**

```cpp
// Example: Strategy performance evaluation
double nominal_return = 0.12;   // 12% raw return
double inflation_rate = 0.03;   // 3% inflation
double real_return = nominal_return - inflation_rate;  // 9% purchasing power

// For Sharpe ratio calculation, use real returns:
double risk_free_real = treasury_rate - inflation_rate;
double sharpe = (real_return - risk_free_real) / strategy_volatility;
```

### **Log Returns vs Arithmetic Returns**

```cpp
// Arithmetic returns (what people usually think of)
double arithmetic_return = (price_end - price_start) / price_start;

// Log returns (better for statistical modeling)
double log_return = std::log(price_end / price_start);

// Why log returns matter:
// 1. Log returns are additive: log_return_total = log_return_1 + log_return_2
// 2. Log returns are approximately normal distributed
// 3. Easier to model mathematically (used in Black-Scholes)
```

### **Volatility and Risk**

```cpp
// Volatility = Standard deviation of returns (usually annualized)
// Higher volatility = higher risk = higher expected return (risk premium)

// Example calculation:
std::vector<double> daily_returns = {0.01, -0.02, 0.015, -0.005, 0.008};
double mean_return = calculate_mean(daily_returns);
double variance = calculate_variance(daily_returns, mean_return);
double daily_vol = std::sqrt(variance);
double annual_vol = daily_vol * std::sqrt(252);  // 252 trading days per year
```

---

## 🔬 Testing and Validation

### **Synthetic Data for Learning**

Before using real market data, test your indicators with known synthetic data:

```cpp
// Generate predictable test series
SyntheticDataGenerator gen(42);  // Fixed seed for reproducibility

// Test mean reversion strategy on mean-reverting data
auto mean_rev_data = gen.generate_mean_reverting(
    80.0,    // start below mean
    100.0,   // long-term mean
    2.0,     // fast reversion
    0.15,    // 15% volatility
    252      // 1 year of data
);

// Your RSI + Bollinger strategy should perform well on this data
```

### **Unit Testing Best Practices**

```cpp
// Test EMA with known values
TEST_CASE("EMA matches hand-calculated values") {
    RollingEMA ema(3);  // 3-period EMA, α = 2/4 = 0.5
    
    REQUIRE(ema.update(10.0) == Approx(10.0));     // First value
    REQUIRE(ema.update(20.0) == Approx(15.0));     // 0.5*20 + 0.5*10 = 15
    REQUIRE(ema.update(30.0) == Approx(22.5));     // 0.5*30 + 0.5*15 = 22.5
}
```

---

## 🚀 Next Steps

1. **Implement Rolling EMA** in `src/indicators/rolling_ema.hpp`
2. **Write comprehensive tests** to verify mathematical accuracy
3. **Add RSI and Bollinger Bands** using the same O(1) pattern
4. **Create a simple mean-reversion strategy** combining RSI + Bollinger
5. **Integrate Alpaca API** for real historical data
6. **Build backtesting engine** with realistic transaction costs

---

## 📚 References and Further Reading

### **Quantitative Finance:**
- Hull, John C. "Options, Futures, and Other Derivatives" (Stochastic calculus, Black-Scholes)
- Wilmott, Paul "Paul Wilmott Introduces Quantitative Finance" (Accessible math)
- Gatheral, Jim "The Volatility Surface" (Advanced volatility modeling)

### **Technical Analysis:**
- Murphy, John J. "Technical Analysis of the Financial Markets" (Classic reference)
- Pring, Martin J. "Technical Analysis Explained" (Comprehensive indicator guide)

### **C++ for Finance:**
- Duffy, Daniel J. "Financial Instrument Pricing Using C++" (Industry techniques)
- Palley, Mark "C++ Design Patterns and Derivatives Pricing" (Design patterns)

### **Algorithmic Trading:**
- Chan, Ernest P. "Quantitative Trading" (Practical strategy development)
- Lopez de Prado, Marcos "Advances in Financial Machine Learning" (Modern techniques)

---