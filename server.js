const express = require('express');
const cors = require('cors');
const helmet = require('helmet');
const compression = require('compression');
const { spawn } = require('child_process');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 8080;

// Security and performance middleware
app.use(helmet());
app.use(compression());
app.use(cors({
  origin: true, // Allow all origins for now
  credentials: true
}));
app.use(express.json({ limit: '10mb' }));

// Health check endpoint (required for Cloud Run)
app.get('/health', (req, res) => {
  res.status(200).json({ 
    status: 'healthy', 
    timestamp: new Date().toISOString(),
    service: 'strategy-optimizer-api',
    version: '1.0.0'
  });
});

// Service info endpoint
app.get('/info', (req, res) => {
  res.json({
    service: 'Strategy Optimizer API',
    version: '1.0.0',
    environment: process.env.NODE_ENV || 'development',
    capabilities: [
      'Mean Reversion Strategy Optimization',
      'Multi-symbol backtesting',
      'Parameter sweeping',
      'Performance metrics calculation'
    ],
    supported_symbols: ['AAPL', 'TSLA', 'NVDA', 'MSFT', 'GOOGL'],
    endpoints: {
      health: '/health',
      info: '/info',
      optimize: '/optimize'
    }
  });
});

// Main optimization endpoint
app.post('/optimize', async (req, res) => {
  try {
    console.log('🚀 Starting strategy optimization...');
    console.log('📊 Parameters:', req.body);

    const {
      symbol = 'AAPL',
      days = 120,
      rsi_period_min = 10,
      rsi_period_max = 14,
      oversold_threshold = 30,
      overbought_threshold = 70,
      confidence_threshold = 65
    } = req.body;

    // Validate required parameters
    if (!symbol || !days) {
      return res.status(400).json({
        success: false,
        error: 'Missing required parameters',
        required: ['symbol', 'days']
      });
    }

    // Run C++ strategy optimizer
    const results = await runCppOptimizer({
      symbol,
      days,
      rsi_period_min,
      rsi_period_max,
      oversold_threshold,
      overbought_threshold,
      confidence_threshold
    });

    console.log(`✅ Optimization completed with ${results.optimization_results?.length || 0} results`);

    // Add cache-busting headers to ensure fresh responses
    res.set({
      'Cache-Control': 'no-store, no-cache, must-revalidate, proxy-revalidate',
      'Pragma': 'no-cache',
      'Expires': '0',
      'Surrogate-Control': 'no-store'
    });

    res.json({
      success: true,
      timestamp: new Date().toISOString(),
      request_id: Math.random().toString(36).substring(7), // Add unique request ID
      ...results
    });

  } catch (error) {
    console.error('❌ Optimization failed:', error);
    
    // Add cache-busting headers for error responses too
    res.set({
      'Cache-Control': 'no-store, no-cache, must-revalidate, proxy-revalidate',
      'Pragma': 'no-cache',
      'Expires': '0',
      'Surrogate-Control': 'no-store'
    });
    
    res.status(500).json({
      success: false,
      error: error.message,
      timestamp: new Date().toISOString(),
      request_id: Math.random().toString(36).substring(7)
    });
  }
});

// Helper function to run C++ backtesting engine
async function runCppOptimizer(parameters) {
  return new Promise((resolve, reject) => {
    const backtestPath = path.join(__dirname, 'strategy_optimizer');
    
    console.log('🔧 Running real backtesting engine:', backtestPath);
    console.log('📊 Parameters:', parameters);

    // Set environment variables for Alpaca API
    const env = {
      ...process.env,
      ALPACA_API_KEY_ID: process.env.ALPACA_API_KEY_ID || 'DEMO_KEY',
      ALPACA_API_SECRET_KEY: process.env.ALPACA_API_SECRET_KEY || 'DEMO_SECRET',
      ALPACA_BASE_URL: process.env.ALPACA_BASE_URL || 'https://paper-api.alpaca.markets',
      ALPACA_PAPER: '1'
    };

    // The institutional_backtest takes: symbol, days, confidence_threshold, oversold_threshold, overbought_threshold
    const symbol = parameters.symbol || 'AAPL';
    const days = parameters.days || 120; // Use days directly from frontend
    const confidenceThreshold = (parameters.confidence_threshold || 65) / 100.0; // Convert % to decimal
    const oversoldThreshold = parameters.oversold_threshold || 30;
    const overboughtThreshold = parameters.overbought_threshold || 70;
    
    const requestId = Math.random().toString(36).substring(7);
    console.log(`🔧 [${requestId}] Calling: ${backtestPath} ${symbol} ${days} ${confidenceThreshold} ${oversoldThreshold} ${overboughtThreshold}`);
    console.log(`🔧 [${requestId}] Full command: ${backtestPath} ${symbol} ${days} ${confidenceThreshold} ${oversoldThreshold} ${overboughtThreshold}`);
    
    const childProcess = spawn(backtestPath, [symbol, days.toString(), confidenceThreshold.toString(), oversoldThreshold.toString(), overboughtThreshold.toString()], { 
      env,
      cwd: __dirname
    });

    let stdout = '';
    let stderr = '';
    let fullOutput = '';

    childProcess.stdout.on('data', (data) => {
      const output = data.toString();
      stdout += output;
      fullOutput += output;
      console.log(`📈 [${requestId}] C++ Output:`, output.trim());
    });

    childProcess.stderr.on('data', (data) => {
      const output = data.toString();
      stderr += output;
      fullOutput += output;
      console.log(`🔍 [${requestId}] C++ Debug:`, output.trim());
    });

    childProcess.on('close', (code) => {
      if (code !== 0) {
        console.error('❌ C++ optimizer failed with code:', code);
        console.error('❌ Error output:', stderr);
        
        // Return error result instead of mock data
        console.log('❌ C++ failed, returning empty results');
        resolve({
          success: false,
          error: `C++ process failed with code ${code}`,
          timestamp: new Date().toISOString(),
          optimization_results: [],
          total_combinations: 0
        });
        return;
      }

      try {
        // Extract JSON from C++ output
        const jsonStart = stdout.indexOf('{');
        const jsonEnd = stdout.lastIndexOf('}');
        
        if (jsonStart !== -1 && jsonEnd !== -1) {
          const jsonString = stdout.substring(jsonStart, jsonEnd + 1).trim();
          console.log(`🔧 [${requestId}] Extracted JSON:`, jsonString);
          const results = JSON.parse(jsonString);
          // Add full terminal output to response
          results.terminal_output = fullOutput;
          results.raw_stdout = stdout;
          results.raw_stderr = stderr;
          resolve(results);
        } else {
          // Return raw terminal output as success (strategy_optimizer outputs plain text, not JSON)
          console.log(`✅ [${requestId}] No JSON found, but C++ completed successfully - returning terminal output`);
          resolve({
            success: true,
            message: 'Strategy optimization completed - see terminal output for results',
            terminal_output: fullOutput,
            raw_stdout: stdout,
            raw_stderr: stderr,
            timestamp: new Date().toISOString(),
            optimization_results: [],
            total_combinations: 0
          });
        }
      } catch (error) {
        console.error('❌ Failed to parse C++ output:', error);
        console.log('Raw stdout:', stdout);
        // Return raw terminal output and parsing error
        resolve({
          success: false,
          error: `JSON parsing failed: ${error.message}`,
          raw_output: stdout,
          raw_error: stderr,
          timestamp: new Date().toISOString(),
          optimization_results: [],
          total_combinations: 0
        });
      }
    });

    childProcess.on('error', (error) => {
      console.error('❌ Failed to start C++ optimizer:', error);
      resolve({
        success: false,
        error: `Failed to start C++ process: ${error.message}`,
        timestamp: new Date().toISOString(),
        optimization_results: [],
        total_combinations: 0
      });
    });
  });
}

// Mock data function REMOVED - only real C++ results or errors now

// Error handling middleware
app.use((error, req, res, next) => {
  console.error('🚨 Unhandled error:', error);
  res.status(500).json({
    success: false,
    error: 'Internal server error',
    timestamp: new Date().toISOString()
  });
});

// Start server
app.listen(PORT, '0.0.0.0', () => {
  console.log(`🚀 Strategy Optimizer API running on port ${PORT}`);
  console.log(`🌐 Health check: http://localhost:${PORT}/health`);
  console.log(`📊 Optimization endpoint: http://localhost:${PORT}/optimize`);
  console.log(`ℹ️  Info endpoint: http://localhost:${PORT}/info`);
});

module.exports = app;