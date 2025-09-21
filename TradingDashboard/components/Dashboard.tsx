import React, { useState, useEffect } from 'react';
import {
  View,
  Text,
  StyleSheet,
  ScrollView,
  TouchableOpacity,
  RefreshControl,
  Alert,
} from 'react-native';
import { LinearGradient } from 'expo-linear-gradient';
import { Ionicons } from '@expo/vector-icons';
import { OptimizationResult } from '../types/trading';
import { OptimizationService } from '../services/optimizationService';
import OptimizationCard from './OptimizationCard';
import MetricsChart from './MetricsChart';
import ParameterControls from './ParameterControls';
import TerminalOutput from './TerminalOutput';
import TradeLog from './TradeLog';
import { OptimizationParameters, TradeAction } from '../types/trading';

interface DashboardProps {
  // Future props for user settings, etc.
}

const Dashboard: React.FC<DashboardProps> = () => {
  const [results, setResults] = useState<OptimizationResult[]>([]);
  const [terminalOutput, setTerminalOutput] = useState<string>('');
  const [showTerminal, setShowTerminal] = useState<boolean>(false);
  const [loading, setLoading] = useState(false);
  const [refreshing, setRefreshing] = useState(false);
  const [selectedMetric, setSelectedMetric] = useState<'return' | 'drawdown' | 'sharpe'>('return');
  const [isOptimizationRunning, setIsOptimizationRunning] = useState(false);

  // Load results from localStorage on mount
  useEffect(() => {
    const savedResults = localStorage.getItem('optimizationResults');
    const savedTerminal = localStorage.getItem('terminalOutput');
    if (savedResults) setResults(JSON.parse(savedResults));
    if (savedTerminal) setTerminalOutput(savedTerminal);

    // Set up results listener for Cloud Run API
    const unsubscribe = OptimizationService.subscribeToResults((newResults, newTerminalOutput) => {
      setResults(newResults);
      setTerminalOutput(newTerminalOutput);
      setLoading(false);
      // Save to localStorage
      localStorage.setItem('optimizationResults', JSON.stringify(newResults));
      localStorage.setItem('terminalOutput', newTerminalOutput);
      // Auto-show terminal when new results come in
      if (newTerminalOutput && newTerminalOutput.length > 0) {
        setShowTerminal(true);
      }
    });
    return () => unsubscribe();
  }, []);

  const loadOptimizationResults = async () => {
    try {
      setLoading(true);
      // Load latest results from Cloud Run API
      const data = await OptimizationService.getLatestResults();
      const terminalData = OptimizationService.getLatestTerminalOutput();
      setResults(data);
      setTerminalOutput(terminalData);
    } catch (error) {
      Alert.alert('Error', 'Failed to load optimization results');
      console.error(error);
    } finally {
      setLoading(false);
    }
  };

  const onRefresh = async () => {
    setRefreshing(true);
    await loadOptimizationResults();
    setRefreshing(false);
  };

  const handleRunOptimization = async (params: OptimizationParameters) => {
    try {
      setIsOptimizationRunning(true);
      setLoading(true);
      const optimizationResults = await OptimizationService.runOptimization(params);
      setResults(optimizationResults);
      // Save to localStorage
      localStorage.setItem('optimizationResults', JSON.stringify(optimizationResults));
      localStorage.setItem('terminalOutput', terminalOutput);
      Alert.alert('Success', `Optimization completed with ${optimizationResults.length} results`);
    } catch (error) {
      Alert.alert('Error', 'Failed to run optimization');
      console.error(error);
    } finally {
      setIsOptimizationRunning(false);
      setLoading(false);
    }
  };

  const getBestResult = () => {
    return results.reduce((best, current) => 
      current.total_return > best.total_return ? current : best,
      results[0]
    );
  };

  const getMetricData = () => {
    return results.map((result, index) => ({
      label: `MeanRev-${result.oversold_threshold}/${result.overbought_threshold}`,
      value: selectedMetric === 'return' ? result.total_return_pct :
             selectedMetric === 'drawdown' ? result.max_drawdown :
             result.sharpe_ratio
    }));
  };

  if (loading && results.length === 0 && !terminalOutput) {
    return (
      <LinearGradient
        colors={['#1e3c72', '#2a5298']}
        style={styles.loadingContainer}
      >
        <Ionicons name="analytics-outline" size={50} color="white" />
        <Text style={styles.loadingText}>Loading Optimization Results...</Text>
      </LinearGradient>
    );
  }

  const bestResult = results.length > 0 ? getBestResult() : null;

  return (
    <LinearGradient colors={['#1e3c72', '#2a5298']} style={styles.container}>
      <ScrollView
        contentContainerStyle={styles.scrollContent}
        refreshControl={
          <RefreshControl refreshing={refreshing} onRefresh={onRefresh} />
        }
      >
        {/* Header */}
        <View style={styles.header}>
          <Text style={styles.title}>🎯 Mean Reversion Backtester</Text>
          <Text style={styles.subtitle}>Quantlab Strategy Results</Text>
        </View>

        {/* Parameter Controls */}
        <ParameterControls 
          onRunOptimization={handleRunOptimization}
          isRunning={isOptimizationRunning}
        />

        {/* Terminal Output */}
        <TerminalOutput 
          output={terminalOutput}
          isVisible={showTerminal}
          onToggle={() => setShowTerminal(!showTerminal)}
        />

        {/* Best Performance Card & Trade Log */}
        {bestResult ? (
          <View style={styles.bestResultCard}>
            <Text style={styles.bestResultTitle}>🏆 Best Performance</Text>
            <View style={styles.bestResultContent}>
              <Text style={styles.bestResultSymbol}>{bestResult.symbol}</Text>
              <Text style={styles.bestResultReturn}>
                {bestResult.total_return_pct.toFixed(2)}%
              </Text>
              <Text style={styles.bestResultDetails}>
                RSI Period: {bestResult.rsi_period_min}-{bestResult.rsi_period_max} • 
                Thresholds: {bestResult.oversold_threshold}/{bestResult.overbought_threshold}
              </Text>
              <Text style={styles.bestResultTrades}>
                {bestResult.total_trades} trades • {bestResult.win_rate.toFixed(1)}% win rate
              </Text>
            </View>
            {/* Trade Log for best result */}
            <TradeLog tradeActions={bestResult.trade_actions || []} />
          </View>
        ) : (
          <Text style={{ color: 'white', marginBottom: 10 }}>No optimization results yet. Run a simulation to see results.</Text>
        )}

        {/* Metric Selection */}
        <View style={styles.metricSelector}>
          {(['return', 'drawdown', 'sharpe'] as const).map((metric) => (
            <TouchableOpacity
              key={metric}
              style={[
                styles.metricButton,
                selectedMetric === metric && styles.metricButtonActive
              ]}
              onPress={() => setSelectedMetric(metric)}
            >
              <Text style={[
                styles.metricButtonText,
                selectedMetric === metric && styles.metricButtonTextActive
              ]}>
                {metric === 'return' ? 'Return %' :
                 metric === 'drawdown' ? 'Drawdown %' : 'Sharpe Ratio'}
              </Text>
            </TouchableOpacity>
          ))}
        </View>

        {/* Chart */}
        <MetricsChart 
          data={getMetricData()}
          title={selectedMetric === 'return' ? 'Total Return %' :
                 selectedMetric === 'drawdown' ? 'Max Drawdown %' : 'Sharpe Ratio'}
        />

        {/* Results Grid */}
        <View style={styles.resultsGrid}>
          <Text style={styles.sectionTitle}>📊 All Results</Text>
          {results.length > 0 ? (
            results.map((result, index) => (
              <OptimizationCard key={index} result={result} />
            ))
          ) : (
            <Text style={{ color: 'white', marginTop: 10 }}>No data available. Run a simulation to see results.</Text>
          )}
        </View>

        {/* Refresh Button */}
        <TouchableOpacity style={styles.refreshButton} onPress={onRefresh}>
          <Ionicons name="refresh-outline" size={20} color="white" />
          <Text style={styles.refreshButtonText}>Refresh Data</Text>
        </TouchableOpacity>
      </ScrollView>
    </LinearGradient>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
  loadingContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
  },
  loadingText: {
    color: 'white',
    fontSize: 18,
    marginTop: 16,
  },
  scrollContent: {
    padding: 20,
    paddingTop: 60,
  },
  header: {
    alignItems: 'center',
    marginBottom: 30,
  },
  title: {
    fontSize: 28,
    fontWeight: 'bold',
    color: 'white',
    marginBottom: 8,
  },
  subtitle: {
    fontSize: 16,
    color: 'rgba(255, 255, 255, 0.8)',
  },
  bestResultCard: {
    backgroundColor: 'rgba(255, 255, 255, 0.1)',
    borderRadius: 15,
    padding: 20,
    marginBottom: 20,
    borderWidth: 1,
    borderColor: 'rgba(255, 255, 255, 0.2)',
  },
  bestResultTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: 'white',
    marginBottom: 15,
  },
  bestResultContent: {
    alignItems: 'center',
  },
  bestResultSymbol: {
    fontSize: 24,
    fontWeight: 'bold',
    color: '#4CAF50',
    marginBottom: 5,
  },
  bestResultReturn: {
    fontSize: 36,
    fontWeight: 'bold',
    color: '#4CAF50',
    marginBottom: 10,
  },
  bestResultDetails: {
    fontSize: 14,
    color: 'rgba(255, 255, 255, 0.8)',
    marginBottom: 5,
  },
  bestResultTrades: {
    fontSize: 14,
    color: 'rgba(255, 255, 255, 0.8)',
  },
  metricSelector: {
    flexDirection: 'row',
    backgroundColor: 'rgba(255, 255, 255, 0.1)',
    borderRadius: 10,
    padding: 5,
    marginBottom: 20,
  },
  metricButton: {
    flex: 1,
    paddingVertical: 10,
    alignItems: 'center',
    borderRadius: 8,
  },
  metricButtonActive: {
    backgroundColor: 'rgba(255, 255, 255, 0.2)',
  },
  metricButtonText: {
    color: 'rgba(255, 255, 255, 0.7)',
    fontSize: 12,
    fontWeight: '600',
  },
  metricButtonTextActive: {
    color: 'white',
  },
  sectionTitle: {
    fontSize: 20,
    fontWeight: 'bold',
    color: 'white',
    marginBottom: 15,
  },
  resultsGrid: {
    marginTop: 20,
  },
  refreshButton: {
    flexDirection: 'row',
    backgroundColor: 'rgba(255, 255, 255, 0.2)',
    borderRadius: 10,
    padding: 15,
    alignItems: 'center',
    justifyContent: 'center',
    marginTop: 20,
    marginBottom: 40,
  },
  refreshButtonText: {
    color: 'white',
    fontSize: 16,
    fontWeight: '600',
    marginLeft: 8,
  },
});

export default Dashboard;