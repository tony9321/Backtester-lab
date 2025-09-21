import React from 'react';
import { View, Text, StyleSheet, FlatList } from 'react-native';
import { TradeAction } from '../types/trading';

interface TradeLogProps {
  tradeActions: TradeAction[];
}

const TradeLog: React.FC<TradeLogProps> = ({ tradeActions }) => {
  if (!tradeActions || tradeActions.length === 0) {
    return <Text style={styles.empty}>No trades executed in this run.</Text>;
  }
  return (
    <View style={styles.container}>
      <Text style={styles.title}>Trade Log</Text>
      <FlatList
        data={tradeActions}
        keyExtractor={(_, idx) => idx.toString()}
        renderItem={({ item }) => (
          <View style={styles.row}>
            <Text style={[styles.cell, { color: item.type === 'BUY' ? '#4CAF50' : '#F44336' }]}>{item.type}</Text>
            <Text style={styles.cell}>${item.price.toFixed(2)}</Text>
            <Text style={styles.cell}>{item.shares} shares</Text>
            <Text style={styles.cell}>Conf: {item.confidence.toFixed(2)}</Text>
            <Text style={styles.cell}>{item.reason}</Text>
          </View>
        )}
        ListHeaderComponent={() => (
          <View style={styles.headerRow}>
            <Text style={styles.headerCell}>Type</Text>
            <Text style={styles.headerCell}>Price</Text>
            <Text style={styles.headerCell}>Shares</Text>
            <Text style={styles.headerCell}>Confidence</Text>
            <Text style={styles.headerCell}>Reason</Text>
          </View>
        )}
      />
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    marginVertical: 16,
    backgroundColor: '#222',
    borderRadius: 8,
    padding: 12,
  },
  title: {
    color: 'white',
    fontWeight: 'bold',
    fontSize: 16,
    marginBottom: 8,
  },
  headerRow: {
    flexDirection: 'row',
    marginBottom: 4,
  },
  headerCell: {
    flex: 1,
    color: '#bbb',
    fontWeight: 'bold',
    fontSize: 13,
  },
  row: {
    flexDirection: 'row',
    marginBottom: 2,
  },
  cell: {
    flex: 1,
    color: 'white',
    fontSize: 13,
  },
  empty: {
    color: '#bbb',
    fontStyle: 'italic',
    marginVertical: 8,
  },
});

export default TradeLog;