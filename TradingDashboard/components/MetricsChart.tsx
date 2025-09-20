import React from 'react';
import { View, Text, StyleSheet, Dimensions } from 'react-native';
import { BarChart } from 'react-native-chart-kit';

interface MetricsChartProps {
  data: Array<{ label: string; value: number }>;
  title: string;
}

const screenWidth = Dimensions.get('window').width;

const MetricsChart: React.FC<MetricsChartProps> = ({ data, title }) => {
  const chartData = {
    labels: data.map(item => item.label),
    datasets: [
      {
        data: data.map(item => Math.max(0, item.value)), // Ensure non-negative values
      },
    ],
  };

  const chartConfig = {
    backgroundGradientFrom: 'rgba(255, 255, 255, 0.1)',
    backgroundGradientTo: 'rgba(255, 255, 255, 0.05)',
    decimalPlaces: 2,
    color: (opacity = 1) => `rgba(76, 175, 80, ${opacity})`,
    labelColor: (opacity = 1) => `rgba(255, 255, 255, ${opacity * 0.8})`,
    style: {
      borderRadius: 16,
    },
    propsForDots: {
      r: '6',
      strokeWidth: '2',
      stroke: '#4CAF50',
    },
  };

  if (data.length === 0) {
    return (
      <View style={styles.emptyContainer}>
        <Text style={styles.emptyText}>No data available</Text>
      </View>
    );
  }

  return (
    <View style={styles.container}>
      <Text style={styles.title}>{title}</Text>
      <View style={styles.chartContainer}>
        <BarChart
          data={chartData}
          width={screenWidth - 60}
          height={220}
          chartConfig={chartConfig}
          style={styles.chart}
          showValuesOnTopOfBars={true}
          fromZero={true}
          segments={4}
          yAxisLabel=""
          yAxisSuffix=""
        />
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    backgroundColor: 'rgba(255, 255, 255, 0.1)',
    borderRadius: 12,
    padding: 16,
    marginBottom: 20,
    borderWidth: 1,
    borderColor: 'rgba(255, 255, 255, 0.2)',
  },
  title: {
    fontSize: 18,
    fontWeight: 'bold',
    color: 'white',
    textAlign: 'center',
    marginBottom: 16,
  },
  chartContainer: {
    alignItems: 'center',
    overflow: 'hidden',
    borderRadius: 8,
  },
  chart: {
    borderRadius: 8,
  },
  emptyContainer: {
    backgroundColor: 'rgba(255, 255, 255, 0.1)',
    borderRadius: 12,
    padding: 40,
    alignItems: 'center',
    marginBottom: 20,
  },
  emptyText: {
    color: 'rgba(255, 255, 255, 0.6)',
    fontSize: 16,
  },
});

export default MetricsChart;