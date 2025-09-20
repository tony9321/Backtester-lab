import React from 'react';
import {
  View,
  Text,
  StyleSheet,
  ScrollView,
  TouchableOpacity,
} from 'react-native';
import { Ionicons } from '@expo/vector-icons';

interface TerminalOutputProps {
  output: string;
  isVisible: boolean;
  onToggle?: () => void;
}

const TerminalOutput: React.FC<TerminalOutputProps> = ({ 
  output, 
  isVisible, 
  onToggle 
}) => {
  return (
    <View style={styles.container}>
      {/* Header with toggle */}
      <TouchableOpacity style={styles.header} onPress={onToggle}>
        <View style={styles.headerContent}>
          <Ionicons name="terminal-outline" size={20} color="white" />
          <Text style={styles.headerText}>Terminal Output</Text>
        </View>
        <Ionicons 
          name={isVisible ? "chevron-up-outline" : "chevron-down-outline"} 
          size={20} 
          color="white" 
        />
      </TouchableOpacity>

      {/* Terminal content */}
      {isVisible && (
        <ScrollView 
          style={styles.terminalContainer}
          showsVerticalScrollIndicator={true}
          persistentScrollbar={true}
        >
          <Text style={styles.terminalText}>
            {output || 'No terminal output available. Run a simulation to see detailed results.'}
          </Text>
        </ScrollView>
      )}
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    backgroundColor: 'rgba(0, 0, 0, 0.4)',
    borderRadius: 10,
    marginVertical: 10,
    overflow: 'hidden',
  },
  header: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    padding: 15,
    backgroundColor: 'rgba(0, 0, 0, 0.6)',
  },
  headerContent: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  headerText: {
    color: 'white',
    fontSize: 16,
    fontWeight: '600',
    marginLeft: 8,
  },
  terminalContainer: {
    maxHeight: 300,
    padding: 15,
  },
  terminalText: {
    fontFamily: 'Courier', // Monospace font for terminal look
    fontSize: 12,
    color: '#00FF00', // Classic terminal green
    lineHeight: 16,
  },
});

export default TerminalOutput;