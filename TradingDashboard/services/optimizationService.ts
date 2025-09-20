import { getEndpointUrl, CLOUD_RUN_CONFIG } from '../config/cloudRun';
import { OptimizationResult, OptimizationParameters, OptimizationResponse } from '../types/trading';

export class OptimizationService {
  private static results: OptimizationResult[] = [];
  private static terminalOutput: string = '';
  private static listeners: ((results: OptimizationResult[], terminalOutput: string) => void)[] = [];

  // Run optimization with user parameters via Cloud Run API
  static async runOptimization(params: OptimizationParameters): Promise<OptimizationResult[]> {
    try {
      console.log('🚀 Starting optimization with Cloud Run API...');
      console.log('📊 Parameters:', params);
      
      // Create timeout controller for long-running optimizations
      const controller = new AbortController();
      const timeoutId = setTimeout(() => controller.abort(), CLOUD_RUN_CONFIG.DEFAULT_TIMEOUT);

      const response = await fetch(getEndpointUrl('OPTIMIZE'), {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(params),
        signal: controller.signal
      });

      clearTimeout(timeoutId);

      if (!response.ok) {
        const errorText = await response.text();
        throw new Error(`API request failed: ${response.status} - ${errorText}`);
      }

      const data: OptimizationResponse = await response.json();
      
      if (!data.success && !data.terminal_output) {
        throw new Error(data.error || 'Optimization failed');
      }

      const results = data.optimization_results || [];
      const terminalOutput = data.terminal_output || '';
      console.log('✅ Optimization completed:', results.length, 'results');
      console.log('📄 Terminal output captured:', terminalOutput.length, 'characters');
      
      // If we have terminal output but no structured results, that's still success
      if (terminalOutput && results.length === 0) {
        console.log('📋 Strategy optimizer returned terminal output (plain text results)');
      }
      
      // Store results and terminal output locally and notify listeners
      this.results = results;
      this.terminalOutput = terminalOutput;
      this.notifyListeners(results, terminalOutput);
      
      return results;
    } catch (error) {
      console.error('❌ Error running optimization:', error);
      throw error;
    }
  }

  // Get latest optimization results (from local storage)
  static async getLatestResults(): Promise<OptimizationResult[]> {
    return this.results;
  }

  // Subscribe to optimization results updates (local state management)
  static subscribeToResults(callback: (results: OptimizationResult[], terminalOutput: string) => void) {
    this.listeners.push(callback);
    
    // Immediately call with current results
    callback(this.results, this.terminalOutput);
    
    // Return unsubscribe function
    return () => {
      const index = this.listeners.indexOf(callback);
      if (index > -1) {
        this.listeners.splice(index, 1);
      }
    };
  }

  // Check if Cloud Run API is healthy
  static async checkApiHealth(): Promise<boolean> {
    try {
      const response = await fetch(getEndpointUrl('HEALTH'), {
        method: 'GET',
        headers: {
          'Content-Type': 'application/json'
        }
      });
      return response.ok;
    } catch (error) {
      console.error('❌ API health check failed:', error);
      return false;
    }
  }

  // Get API information
  static async getApiInfo(): Promise<any> {
    try {
      const response = await fetch(getEndpointUrl('INFO'), {
        method: 'GET',
        headers: {
          'Content-Type': 'application/json'
        }
      });
      
      if (response.ok) {
        return await response.json();
      }
      return null;
    } catch (error) {
      console.error('❌ Error fetching API info:', error);
      return null;
    }
  }

  // Get latest terminal output
  static getLatestTerminalOutput(): string {
    return this.terminalOutput;
  }

  // Notify all listeners of results update
  private static notifyListeners(results: OptimizationResult[], terminalOutput: string) {
    this.listeners.forEach(listener => {
      try {
        listener(results, terminalOutput);
      } catch (error) {
        console.error('❌ Error in result listener:', error);
      }
    });
  }
}