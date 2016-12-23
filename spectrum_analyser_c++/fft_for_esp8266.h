/*
  Spectrum analyser for Florian's LED display. 
  Author: Benjamin Negrevergne.
  Based on example from the  FFT libray (Copyright (C) 2014 Enrique Condes).
*/

#ifndef FFT_For_ESP8266_h
#define FFT_For_ESP8266_h

#include "Arduino.h"

#define NDEBUG // Uncomment for production

class FFT_For_ESP8266 {
  public:

  /* ctor */
  FFT_For_ESP8266(int displayWidth, int displayHeight, short analogPin, int numSamples); 

  /* Main functions */
  void sampleFromADC(double *data);

  /* Compute FFT from sampled data in data, and store magnitude for each band back into data (only the first size / 2 elements are used) */
  void computeFFT(double *data, double *dataImg);

  /* Build the encoded graph from the fft output data.
     - The total number of columns in the graph is numCols * colWidth.
     - numRows should be 8 for a display with 8 leds
  */
  void buildGraph(uint8_t *out, double *data, uint8_t numRows, uint8_t numCols, uint8_t colWidth);

  /* Print the output of build graph on Serial. Warning totalNumCols is numCols * colWidth (not just numCols).
     Does nothing if NDEBUG is defined
  */
  void printGraph(uint8_t *graphData, uint8_t numRows, uint8_t totalNumCols);


  private:
  


  const int _displayWidth;
  const int _displayHeight;
  const short _analogPin;
  const int _numSamples; 
  arduinoFFT _fft; 

  static const short _skipLowBands = 1; // skip first n bands  (lower frequencies)
  static const short _windowSize = 3; 

  double _previousValues[_windowSize]; 
  double _previousSum; 
  short _count; 



  #ifndef NDEBUG
  long _t0;
  #endif


  /* Helper functions (do not export) */
  double maxv(double *data, uint8_t size);
  double maxSlidingWindow(double *data, uint8_t size); // compute the maximum of the maxima over a series of passed values
  double avgMax(double *data, uint8_t size); // compute the avg of the maxima 
  void startSampling();
  void printSamplingInfo(double *data, uint8_t size);
  void printVector(double *vData, uint8_t bufferSize, uint8_t scaleType);
  uint8_t encodeBar(uint8_t val, uint8_t numRows);



  };

#endif
