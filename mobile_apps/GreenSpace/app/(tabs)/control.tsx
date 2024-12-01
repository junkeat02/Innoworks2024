import { Text, View, StyleSheet } from 'react-native';
import { useRef, useState } from 'react';

export default function Control() {
  return (
    <View style={styles.container}>
      <Text style={styles.text}>Control screen</Text>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#25292e',
    justifyContent: 'center',
    alignItems: 'center',
  },
  text: {
    color: '#fff',
  },
});
