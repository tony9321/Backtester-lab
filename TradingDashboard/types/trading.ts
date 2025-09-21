export interface TradeAction {
  type: 'BUY' | 'SELL';
  price: number;
  shares: number;
  confidence: number;
  reason: string;
}
export interface OptimizationParameters {
  symbol: string;
  days: number; // Days back from today (matches C++ backend logic)
  rsi_period_min: number;
  rsi_period_max: number;
  oversold_threshold: number;
  overbought_threshold: number;
  confidence_threshold?: number; // Optional confidence threshold for strategy
}

export interface OptimizationResult {
  trade_actions?: TradeAction[];
  symbol: string;
  rsi_period_min: number;
  rsi_period_max: number;
  oversold_threshold: number;
  overbought_threshold: number;
  total_return: number;
  total_return_pct: number;
  max_drawdown: number;
  sharpe_ratio: number;
  total_trades: number;
  winning_trades: number;
  win_rate: number;
  profit_factor: number;
}

export interface OptimizationResponse {
  success: boolean;
  timestamp: string;
  request_id: string;
  optimization_results?: OptimizationResult[];
  terminal_output?: string; // Full terminal output from C++ strategy optimizer
  raw_stdout?: string;     // Raw stdout from process
  raw_stderr?: string;     // Raw stderr from process
  error?: string;
}