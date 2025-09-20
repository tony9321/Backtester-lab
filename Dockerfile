# Multi-stage build for faster Cloud Run deployment
FROM node:18-slim as cpp-builder

# Install system dependencies for C++ build
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    pkg-config \
    libssl-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy C++ source
WORKDIR /cpp
COPY quantlab-cpp/ ./

# Build existing strategy optimizer with backtesting engine
# Build institutional backtest (the real tested backtesting engine)
RUN rm -rf build CMakeCache.txt && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release \
             -DCMAKE_CXX_FLAGS="-O2 -DNDEBUG" && \
    make strategy_optimizer -j1 && \
    cp apps/strategy_optimizer ../strategy_optimizer

# Production stage
FROM node:18-slim

# Install minimal runtime dependencies
RUN apt-get update && apt-get install -y \
    curl \
    libssl3 \
    libcurl4 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy Node.js dependencies
COPY package.json ./
RUN npm install --production --no-audit

# Copy built C++ binary and required shared libraries from builder stage
# Copy built C++ binary and required shared libraries from builder stage
COPY --from=cpp-builder /cpp/strategy_optimizer ./strategy_optimizer
COPY --from=cpp-builder /cpp/build/_deps/cpr-build/cpr/libcpr.so* ./

# Copy Node.js server
COPY server.js ./

# Set environment variables
ENV NODE_ENV=production
ENV PORT=8080
ENV ALPACA_PAPER=1
ENV ALPACA_BASE_URL=https://paper-api.alpaca.markets/v2
ENV LD_LIBRARY_PATH=/app

# Expose port for Cloud Run
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Start the server
CMD ["node", "server.js"]