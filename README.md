# Backtester-lab

A quantitative finance learning laboratory for algorithmic trading, technical indicators, and backtesting strategies.

## What This Is

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

**[Detailed Learning Journey](./quantlab-cpp/README.md)**

## Getting Started

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

## quantlab-cpp — Deep technical notes

This section expands the high-level summary with detailed math, indicator derivations, and the C++ techniques used across `quantlab-cpp/`. It's intended as a technical reference for developers and quants who want to understand the precise algorithms and implementation trade-offs.

### Files (quick map)
- `quantlab-cpp/src/core/data_types.hpp` — fundamental market data types (Bar, Tick, Trade, Quote) and a templated `TimeSeries<T>` container for rolling operations.
- `quantlab-cpp/src/indicators/rolling_ema.hpp` — Exponential Moving Average (EMA) implementation (O(1) update).
- `quantlab-cpp/src/indicators/rsi.hpp` — RSI implemented with two EMAs applied to gains and losses.
- `quantlab-cpp/src/indicators/bollinger_bands.hpp` — SMA + standard deviation bands, sliding-window via `std::deque`.
- `quantlab-cpp/src/data/alpaca_client.*` — Alpaca HTTP client (rate-limited aggregation, backoff, JSON parsing).
- `quantlab-cpp/src/backtest/backtest_engine.*` — Portfolio, Trade structs, metrics calculation (P&L matching, drawdown, Sharpe placeholder).
- `quantlab-cpp/src/strategy/mean_reversion_strategy.hpp` — Strategy combining EMA, RSI, Bollinger Bands and an institutional-weighted confidence function.
- `quantlab-cpp/apps/*` — Example apps: `institutional_backtest` and `strategy_optimizer` that demonstrate end-to-end usage.

---

### Mathematical details (indicators & strategy)

1) Exponential Moving Average (EMA)

   - Definition: EMA_t = alpha * Price_t + (1 - alpha) * EMA_{t-1}
   - Smoothing factor: alpha = 2 / (N + 1) for period N.
   - Initialization: the code uses the first price observed as EMA_0 (common, simple and numerically stable).
   - Key behavior: EMA places exponentially more weight on recent prices vs SMA (reacts faster to changes).
   - Numerical note: keep alpha and current EMA as doubles; no history required -> O(1) per tick.

2) Relative Strength Index (RSI)

   - True RSI formula (Wilder): RS = avg_gain / avg_loss; RSI = 100 - (100 / (1 + RS)).
   - Implementation in repo: separate rolling EMAs are used to smooth gains and losses (two `RollingEMA` instances). For each new price:
      - change = price_t - price_{t-1}
      - gain = max(change, 0); loss = max(-change, 0)
      - avg_gain = EMA_gain.update(gain); avg_loss = EMA_loss.update(loss)
      - RS = avg_gain / avg_loss (guard for avg_loss == 0)
      - RSI computed with guards: if avg_loss == 0 and avg_gain == 0 -> RSI = 50 (neutral). If avg_loss == 0 -> RSI = 100.
   - Practical choices: period = 14 is default. Using EMAs to smooth gains/losses gives behavior close to Wilder's smoothing while remaining O(1).

3) Bollinger Bands

   - Definitions:
      - middle = SMA_N = (1/N) * sum_{i=0..N-1} price_{t-i}
      - std_dev = sqrt((1/N) * sum_{i}(price_i - middle)^2)
      - upper = middle + k * std_dev, lower = middle - k * std_dev (k typically 2)
   - Current implementation: maintains window via `std::deque<double> price_history_`, computes SMA via `std::accumulate` and variance by looping over the window -> O(N) per update.
   - Optimization note: replace windowed variance with Welford's (online) algorithm or rolling variance (O(1) per update) to scale to very high-frequency usage.

4) Strategy confidence (institutional-weighted)

   - The `MeanReversionStrategy::calculate_confidence` implements a weighted score across factors:
      - RSI momentum strength (weight ~35%) — normalized distance from extreme thresholds.
      - Bollinger band extremeness (30%) — normalized distance outside band relative to band width.
      - Trend context (20%) — absolute deviation of price from EMA, scaled to [0,1].
      - Volatility regime (15%) — BB width as fraction of middle band, scaled.
   - Weighted average scaled to [0.5, 0.95] to produce a production-friendly confidence score.

5) Backtest accounting & metrics

   - Trades: recorded as timestamp, action, price, shares, value, confidence, reason.
   - P&L matching: sells are paired with accumulated buys using average cost per share. Partial sells proportionally reduce cost basis.
   - Profit factor: total_wins / total_losses with guards (all-wins -> large sentinel; no wins/losses -> 0).
   - Max drawdown: if `daily_values` are present, the engine computes peak-to-trough drawdown across daily series. If not present, a conservative fallback approximation is used.
   - Sharpe ratio: current code contains a simplified approximation; for production compute Sharpe using periodic returns (daily), subtract risk-free rate, divide by stddev of returns, and annualize properly.

---

### C++ techniques, idioms and implementation notes (detailed)

1) Time representation

   - Timestamps are stored as `int64_t timestamp_ns` (nanoseconds since epoch). Rationale:
      - Avoids FP precision loss when representing large epoch times in double (IEEE-754 doubles have ~15-17 decimal digits of precision).
      - Integer arithmetic for time arithmetic is deterministic and faster in many cases.

2) Containers and windowing

   - `std::deque` for sliding-window (Bollinger): pop_front is O(1) which is convenient for fixed-window sizes.
   - `std::vector` for `TimeSeries<T>` internal storage — reserve() used to avoid reallocations; provides contiguous storage and fast random access.
   - `TimeSeries::tail(n)` returns a copy of the last n elements — useful but copies data; for very high-performance usage prefer views or indices.

3) O(1) vs O(N) indicator updates

   - EMA and RSI (via two EMAs) are O(1) per update and memory O(1).
   - Bollinger (current): O(N) per update because it recomputes variance across the window. For performance-critical systems use Welford or a rolling variance:

      Welford's online variance (single-pass) can be adapted to support sliding windows with a small additional data structure (or use two Welford accumulators and subtract the leaving element's effect).

4) Numerical robustness

   - Division-by-zero guards (avg_loss == 0). Clamping and capping scores to avoid out-of-range outputs.
   - Use of `double` everywhere for prices and derived math; integer volumes for counts.

5) Error handling & API resilience

   - `AlpacaClient::make_request` uses exponential backoff and jitter for 429 responses. Retries with increasing backoff and limited attempts.
   - `get_aggregated_historical_bars` includes per-minute rate-limiting to respect API quotas.

6) Build & dependency choices

   - `CMake` + `FetchContent` pulls `nlohmann/json`, `fmt`, `cpr`, and `Catch2`. This keeps a single build bootstrapping flow.
   - Compiler flags in `CMakeLists.txt`: Release uses `-O3 -march=native` for performance, Debug uses `-g -O0 -Wall -Wextra -Wpedantic` for safety.

7) API and ownership patterns

   - Strategies accept `std::shared_ptr<AlpacaClient>` for flexibility; store raw pointer inside for speed in hot paths while preserving lifetime.
   - Value types (Bar, Tick, Quote) are lightweight, moved/passed by const-ref when appropriate.

---

### File-by-file technical notes (high-value lines & rationale)

- `core/data_types.hpp`
   - Bar uses `int64_t timestamp_ns` and an ISO string timestamp for API compatibility.
   - `TimeSeries` includes `tail(n)` and iterators to make windowed computations straightforward. Consider adding non-copying span-like accessors.

- `indicators/rolling_ema.hpp`
   - alpha computed once per construction (`2/(period+1)`). First price seeds EMA.
   - Update path is tiny and suitable for microsecond-level streaming.

- `indicators/rsi.hpp`
   - Stores two `RollingEMA` objects to smooth gains/losses; first observed price initializes RSI state.
   - Edge-case handling prevents division-by-zero and yields sensible neutral/limit RSI values.

- `indicators/bollinger_bands.hpp`
   - Currently computes variance by iterating the deque. For high-frequency data replace with an online variance algorithm.
   - Exposes `is_initialized()` and `value()` for consumers to decide when bands are valid.

- `data/alpaca_client.*`
   - Rate-limited aggregator loops over day offsets while respecting per-minute call budgets.
   - Uses `cpr` (libcpr) for HTTP and `nlohmann::json` for parsing; robust error logging and retry/backoff.
   - Important: reverses aggregated bars into chronological order (oldest->newest) to ensure correct indicator/warmup semantics.

- `backtest/backtest_engine.*`
   - Portfolio tracks `cash`, `shares_held`, `trade_history`, and `daily_values`. Methods `execute_buy/execute_sell` update state and record trades.
   - `calculate_final_metrics()` pairs sells with prior buys using average cost per share and counts winning/losing cycles.
   - Max drawdown uses `daily_values` if available; otherwise falls back to a conservative heuristic.

- `strategy/mean_reversion_strategy.hpp`
   - Contains `calculate_confidence(...)` — a small ensemble scoring system combining multiple market signals into a single confidence number.
   - `generate_signal` uses mid-price from quote API and applies thresholds + confidence test.
   - `backtest()` warms up indicators for `warmup_periods` then iterates bars, producing a `StrategyResult` per bar.

---

### How to build and run (practical)

Prerequisites: CMake 3.20+, a modern C++ compiler (g++/clang with C++20), network access for `FetchContent` dependencies.

1) Build the library and apps

```bash
cd quantlab-cpp
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

2) Set Alpaca environment variables (copy `.env.template`)

```bash
cp .env.template .env
export ALPACA_API_KEY_ID=your_key_here
export ALPACA_API_SECRET_KEY=your_secret_here
export ALPACA_BASE_URL=https://paper-api.alpaca.markets
```

3) Run the institutional backtest example

```bash
# Default: TSLA 120 days, 65% confidence
./apps/institutional_backtest TSLA 120 0.65 30 70

# Example with other args
./apps/institutional_backtest AAPL 365 0.8 25 75
```

4) Run strategy optimizer (parameter sweep)

```bash
./apps/strategy_optimizer
# Outputs: optimization_results.csv and optimization_results.json
```

Notes: Both apps use `AlpacaClient` and will fail fast if required env vars are missing. The aggregator respects API rate limits and includes retries.

---

### Recommended next steps (prioritized)

1. Replace Bollinger variance computation with Welford/rolling variance to achieve O(1) updates for band width.
2. Replace simplified Sharpe with a proper daily-return-based Sharpe calculation and add annualization and risk-free parameter.
3. Add unit tests for each indicator (use Catch2) with deterministic synthetic data (sin waves, step functions, volatility bursts).
4. Add an integration test that runs `institutional_backtest` with a local stubbed `AlpacaClient` (or canned JSON) to avoid network in CI.
5. Add a `quantlab-cpp/README.md` with this material split across per-module deep dives and math derivations (I can create it next).

---

If you'd like, I will now:
- move this long form content into `quantlab-cpp/README.md` (recommended), and
- implement Welford-based Bollinger Bands + unit tests.

Tell me which of the next steps you want me to execute and I'll mark the todos and proceed.

## quantlab-cpp: math notes, C++ tricks, and how to run

This section collects important math notes and implementation details found in the `quantlab-cpp/` sources, plus quick commands to build and run the C++ apps.

High-level contents scanned:
- `src/core/` : data types and time-series container design (Bar, Tick, Trade, Quote, TimeSeries template)
- `src/indicators/` : Rolling EMA, RSI, Bollinger Bands (implementation notes and formulas)
- `src/data/` : `AlpacaClient` with rate-limited aggregation and robust HTTP retry/backoff
- `src/backtest/` : `BacktestEngine` and `Portfolio` accounting, P&L, drawdown, and simplified Sharpe approximation
- `apps/` : `institutional_backtest` and `strategy_optimizer` CLI apps

Key mathematical notes (from comments and inline explanations):
- Exponential Moving Average (EMA): alpha = 2 / (N + 1); EMA_today = alpha * Price_today + (1 - alpha) * EMA_yesterday. EMA initialized with first price.
- RSI (14): uses smoothed averages of gains and losses (implemented with two RollingEMA instances). RSI = 100 - 100/(1 + RS) where RS = avg_gain / avg_loss. edge-cases handled (zero loss -> RSI=100, no change -> 50).
- Bollinger Bands (typical period=20, k=2): middle = SMA(period), std_dev = sqrt(sum((xi - mean)^2)/N), upper = SMA + k * std_dev, lower = SMA - k * std_dev. Interpretation and volatility notes in comments.
- Backtest P&L cycle handling: matches BUY/SELL cycles by tracking position cost and reducing cost basis proportionally when partial sells occur. Win/loss aggregation computes profit factor, win rate, avg win/loss.
- Max drawdown: uses daily_values if available (peak -> trough calculation). Otherwise uses simplified peak vs current cash heuristic. Sharpe ratio approximated: (total_return_pct - 2%) / 15 (note: simplified placeholder, not daily-return based).

C++ patterns, tricks and noteworthy choices found in source code:
- Modern C++ (C++20) enabled via CMakeLists.txt and targetting -O3 -march=native for Release builds.
- Header-only style structs and templated `TimeSeries<T>` container optimized for rolling-window access: uses `std::vector` internally with reserve/tail helpers and `assert` bounds checks.
- Rolling indicators are O(1) per update where possible: RollingEMA keeps a single running EMA state; RSI uses two EMAs for gains/losses for smoothed RS calculation; Bollinger uses a fixed-size deque for the window and computes variance each update (could be optimized further with Welford/online variance).
- Use of `std::deque` for sliding window in Bollinger to pop_front efficiently; `std::accumulate` used for SMA calculation.
- Defensive coding for numeric edge cases: checks for division by zero in profit factor, RSI and Bollinger width checks, clamps scores to [0,1].
- Shared ownership with `std::shared_ptr` for the `AlpacaClient` passed to strategy classes; raw pointer used inside class for performance/ABI simplicity.
- API client uses `cpr` for HTTP, `nlohmann::json` for parsing, and a retry loop with exponential backoff + jitter for rate limiting.
- Small pragmatic tradeoffs (explicitly commented): storing timestamps as `int64_t` nanoseconds to avoid floating point precision, and storing ISO strings for API compatibility.
- String-based lightweight JSON printing for small CLI apps (manual formatting) for portability without spinning up heavy JSON serialization in production paths.

Developer quick-start (build & run)
1. Create a build directory and run CMake (from repo root):

```bash
cd quantlab-cpp
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

2. Run the institutional backtest (example):

```bash
./apps/institutional_backtest TSLA 120 0.65 30 70
# args: <SYMBOL> <DAYS> <CONFIDENCE_THRESHOLD> <RSI_OVERSOLD> <RSI_OVERBOUGHT>
```

3. Run the strategy optimizer (example):

```bash
./apps/strategy_optimizer
# This will run the small parameter grid and emit CSV/JSON results in the repo root
```

Environment variables and credentials
- Copy `quantlab-cpp/.env.template` to `.env` or set environment variables in your shell:
   - `ALPACA_API_KEY_ID`
   - `ALPACA_API_SECRET_KEY`
   - `ALPACA_BASE_URL` (e.g., https://paper-api.alpaca.markets)

Notes and next steps
- The Bollinger Bands implementation computes variance using an O(N) loop every update; consider converting to Welford's online algorithm to make it truly O(1) per update.
- The Sharpe ratio calculation in `BacktestEngine` is a simplified placeholder. For production, compute daily returns and standard deviation, then annualize.
- Consider adding unit tests (Catch2 is already fetched in CMake) for indicators (EMA/RSI/Bollinger) with deterministic inputs to validate edge cases.
