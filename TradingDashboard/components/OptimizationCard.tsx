import React from 'react';
import { View, Text, StyleSheet } from 'react-native';
import { OptimizationResult } from '../types/trading';
import { Ionicons } from '@expo/vector-icons';

interface OptimizationCardProps {
  result: OptimizationResult;
}

const OptimizationCard: React.FC<OptimizationCardProps> = ({ result }) => {
  const isProfit = result.total_return > 0;
  const returnColor = isProfit ? '#4CAF50' : '#F44336';
  
  const formatPercent = (value: number) => `${value.toFixed(2)}%`;
  const formatNumber = (value: number) => value.toFixed(2);

  return (
    <View style={styles.card}>
      {/* Header */}
      <View style={styles.header}>
        <View style={styles.symbolContainer}>
          <Text style={styles.symbol}>{result.symbol}</Text>
          <Text style={styles.period}>Mean Reversion Strategy</Text>
        </View>
        <View style={styles.confidenceContainer}>
          <Text style={styles.confidenceLabel}>Thresholds</Text>
          <Text style={styles.confidence}>
            {result.oversold_threshold}/{result.overbought_threshold}
          </Text>
        </View>
      </View>

      {/* Main Return */}
      <View style={styles.returnSection}>
        <Text style={[styles.returnValue, { color: returnColor }]}>
          {isProfit ? '+' : ''}{formatPercent(result.total_return_pct)}
        </Text>
        <Ionicons 
          name={isProfit ? "trending-up" : "trending-down"} 
          size={20} 
          color={returnColor} 
        />
      </View>

      {/* Metrics Grid */}
      <View style={styles.metricsGrid}>
        <View style={styles.metric}>
          <Text style={styles.metricLabel}>Drawdown</Text>
          <Text style={styles.metricValue}>
            {formatPercent(result.max_drawdown)}
          </Text>
        </View>
        
        <View style={styles.metric}>
          <Text style={styles.metricLabel}>Sharpe</Text>
          <Text style={styles.metricValue}>
            {formatNumber(result.sharpe_ratio)}
          </Text>
        </View>
        
        <View style={styles.metric}>
          <Text style={styles.metricLabel}>Trades</Text>
          <Text style={styles.metricValue}>
            {result.total_trades}
          </Text>
        </View>
        
        <View style={styles.metric}>
          <Text style={styles.metricLabel}>Win Rate</Text>
          <Text style={styles.metricValue}>
            {formatPercent(result.win_rate)}
          </Text>
        </View>
      </View>

      {/* Profit Factor */}
      <View style={styles.profitFactorSection}>
        <Text style={styles.profitFactorLabel}>Profit Factor</Text>
        <Text style={[
          styles.profitFactorValue,
          { color: result.profit_factor > 2 ? '#4CAF50' : 
                   result.profit_factor > 1 ? '#FF9800' : '#F44336' }
        ]}>
          {result.profit_factor === 999.99 ? '∞' : formatNumber(result.profit_factor)}
        </Text>
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  card: {
    backgroundColor: 'rgba(255, 255, 255, 0.1)',
    borderRadius: 12,
    padding: 16,
    marginBottom: 12,
    borderWidth: 1,
    borderColor: 'rgba(255, 255, 255, 0.2)',
  },
  header: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 12,
  },
  symbolContainer: {
    flex: 1,
  },
  symbol: {
    fontSize: 18,
    fontWeight: 'bold',
    color: 'white',
  },
  period: {
    fontSize: 14,
    color: 'rgba(255, 255, 255, 0.7)',
  },
  confidenceContainer: {
    alignItems: 'flex-end',
  },
  confidenceLabel: {
    fontSize: 12,
    color: 'rgba(255, 255, 255, 0.6)',
  },
  confidence: {
    fontSize: 16,
    fontWeight: '600',
    color: '#2196F3',
  },
  returnSection: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    marginBottom: 16,
    paddingVertical: 8,
  },
  returnValue: {
    fontSize: 24,
    fontWeight: 'bold',
    marginRight: 8,
  },
  metricsGrid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    justifyContent: 'space-between',
    marginBottom: 12,
  },
  metric: {
    width: '48%',
    alignItems: 'center',
    paddingVertical: 8,
  },
  metricLabel: {
    fontSize: 12,
    color: 'rgba(255, 255, 255, 0.6)',
    marginBottom: 2,
  },
  metricValue: {
    fontSize: 16,
    fontWeight: '600',
    color: 'white',
  },
  profitFactorSection: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    paddingTop: 12,
    borderTopWidth: 1,
    borderTopColor: 'rgba(255, 255, 255, 0.1)',
  },
  profitFactorLabel: {
    fontSize: 14,
    color: 'rgba(255, 255, 255, 0.8)',
  },
  profitFactorValue: {
    fontSize: 18,
    fontWeight: 'bold',
  },
});

export default OptimizationCard;