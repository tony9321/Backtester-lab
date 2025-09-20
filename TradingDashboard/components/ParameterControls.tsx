import React, { useState } from 'react';
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  TextInput,
  Alert,
} from 'react-native';
import { Ionicons } from '@expo/vector-icons';
import { OptimizationParameters } from '../types/trading';
import { LinearGradient } from 'expo-linear-gradient';

interface ParameterControlsProps {
  onRunOptimization: (params: OptimizationParameters) => void;
  isRunning: boolean;
}

const ParameterControls: React.FC<ParameterControlsProps> = ({ onRunOptimization, isRunning }) => {
  const [symbol, setSymbol] = useState('AAPL');
  const [days, setDays] = useState('120'); // Days back from today (matches C++ logic)
  const [rsiPeriodMin] = useState('14'); // Fixed - strategy uses RSI(14)
  const [rsiPeriodMax] = useState('14'); // Fixed - strategy uses RSI(14)
  const [confidenceThreshold, setConfidenceThreshold] = useState('65');
  const [oversoldThreshold, setOversoldThreshold] = useState('30');
  const [overboughtThreshold, setOverboughtThreshold] = useState('70');

  const handleRunOptimization = () => {
    // Validate inputs
    if (!symbol.trim()) {
      Alert.alert('Error', 'Please enter a symbol');
      return;
    }

    const params: OptimizationParameters = {
      symbol: symbol.toUpperCase(),
      days: parseInt(days),
      rsi_period_min: parseInt(rsiPeriodMin),
      rsi_period_max: parseInt(rsiPeriodMax),
      oversold_threshold: parseInt(oversoldThreshold),
      overbought_threshold: parseInt(overboughtThreshold),
      confidence_threshold: parseInt(confidenceThreshold),
    };

    // Validate parameters
    if (params.oversold_threshold >= params.overbought_threshold) {
      Alert.alert('Error', 'Oversold threshold must be less than Overbought threshold');
      return;
    }

    const confidenceValue = parseInt(confidenceThreshold);
    if (confidenceValue < 50 || confidenceValue > 95) {
      Alert.alert('Error', 'Confidence threshold must be between 50% and 95%');
      return;
    }

    onRunOptimization(params);
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>🎛️ Mean Reversion Strategy Parameters</Text>
      
      {/* Symbol Input */}
      <View style={styles.inputGroup}>
        <Text style={styles.label}>
          <Ionicons name="trending-up" size={16} color="#4CAF50" /> Symbol
        </Text>
        <TextInput
          style={styles.input}
          value={symbol}
          onChangeText={setSymbol}
          placeholder="e.g., AAPL"
          placeholderTextColor="rgba(255, 255, 255, 0.5)"
          editable={!isRunning}
        />
      </View>

      {/* Days Back */}
      <View style={styles.inputGroup}>
        <Text style={styles.label}>
          <Ionicons name="calendar" size={16} color="#2196F3" /> Days Back from Today
        </Text>
        <TextInput
          style={styles.input}
          value={days}
          onChangeText={setDays}
          placeholder="120"
          placeholderTextColor="rgba(255, 255, 255, 0.5)"
          keyboardType="numeric"
          editable={!isRunning}
        />
      </View>

      {/* Confidence Threshold */}
      <View style={styles.inputGroup}>
        <Text style={styles.label}>
          <Ionicons name="analytics" size={16} color="#FF9800" /> Confidence Threshold (%)
        </Text>
        <TextInput
          style={styles.input}
          value={confidenceThreshold}
          onChangeText={setConfidenceThreshold}
          placeholder="65"
          keyboardType="numeric"
          placeholderTextColor="rgba(255, 255, 255, 0.5)"
          editable={!isRunning}
        />
        <Text style={styles.summaryText}>
          Institutional confidence level required for trading signals (50-95%)
        </Text>
      </View>

      {/* RSI Thresholds */}
      <View style={styles.row}>
        <View style={[styles.inputGroup, { flex: 1, marginRight: 10 }]}>
          <Text style={styles.label}>
            <Ionicons name="arrow-down" size={16} color="#F44336" /> RSI Oversold
          </Text>
          <TextInput
            style={styles.input}
            value={oversoldThreshold}
            onChangeText={setOversoldThreshold}
            placeholder="30"
            keyboardType="numeric"
            placeholderTextColor="rgba(255, 255, 255, 0.5)"
            editable={!isRunning}
          />
        </View>
        
        <View style={[styles.inputGroup, { flex: 1, marginLeft: 10 }]}>
          <Text style={styles.label}>
            <Ionicons name="arrow-up" size={16} color="#4CAF50" /> RSI Overbought
          </Text>
          <TextInput
            style={styles.input}
            value={overboughtThreshold}
            onChangeText={setOverboughtThreshold}
            placeholder="70"
            keyboardType="numeric"
            placeholderTextColor="rgba(255, 255, 255, 0.5)"
            editable={!isRunning}
          />
        </View>
      </View>

      {/* Summary */}
      <View style={styles.summaryCard}>
        <Text style={styles.summaryText}>
          Will run Mean Reversion Strategy (RSI + EMA + Bollinger Bands) for {symbol} using last {days} days of data
        </Text>
        <Text style={styles.summaryText}>
          Strategy: RSI(14) thresholds {oversoldThreshold}/{overboughtThreshold}, EMA(20), BB(20,2.0), Conf={confidenceThreshold}%
        </Text>
      </View>

      {/* Run Button */}
      <TouchableOpacity 
        style={[styles.runButton, isRunning && styles.runButtonDisabled]}
        onPress={handleRunOptimization}
        disabled={isRunning}
      >
        <LinearGradient
          colors={isRunning ? ['#666', '#444'] : ['#4CAF50', '#2E7D32']}
          style={styles.runButtonGradient}
        >
          {isRunning ? (
            <Ionicons name="hourglass" size={20} color="white" />
          ) : (
            <Ionicons name="play" size={20} color="white" />
          )}
          <Text style={styles.runButtonText}>
            {isRunning ? 'Running Backtest...' : 'Run Mean Reversion Backtest'}
          </Text>
        </LinearGradient>
      </TouchableOpacity>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    backgroundColor: 'rgba(255, 255, 255, 0.1)',
    borderRadius: 15,
    padding: 20,
    marginBottom: 20,
    borderWidth: 1,
    borderColor: 'rgba(255, 255, 255, 0.2)',
  },
  title: {
    fontSize: 20,
    fontWeight: 'bold',
    color: 'white',
    marginBottom: 20,
    textAlign: 'center',
  },
  inputGroup: {
    marginBottom: 16,
  },
  label: {
    fontSize: 14,
    fontWeight: '600',
    color: 'white',
    marginBottom: 8,
  },
  input: {
    backgroundColor: 'rgba(255, 255, 255, 0.1)',
    borderRadius: 8,
    padding: 12,
    fontSize: 16,
    color: 'white',
    borderWidth: 1,
    borderColor: 'rgba(255, 255, 255, 0.2)',
  },
  row: {
    flexDirection: 'row',
    marginBottom: 16,
  },
  summaryCard: {
    backgroundColor: 'rgba(255, 255, 255, 0.05)',
    borderRadius: 10,
    padding: 16,
    marginBottom: 20,
  },
  summaryText: {
    fontSize: 14,
    color: 'rgba(255, 255, 255, 0.8)',
    textAlign: 'center',
    lineHeight: 20,
  },
  runButton: {
    borderRadius: 12,
    overflow: 'hidden',
  },
  runButtonDisabled: {
    opacity: 0.6,
  },
  runButtonGradient: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    paddingVertical: 16,
    paddingHorizontal: 20,
  },
  runButtonText: {
    fontSize: 18,
    fontWeight: 'bold',
    color: 'white',
    marginLeft: 8,
  },
});

export default ParameterControls;