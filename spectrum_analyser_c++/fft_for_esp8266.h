/*
  Spectrum analyser for Florian's LED display. 
  Author: Benjamin Negrevergne.
  Based on example from the  FFT libray (Copyright (C) 2014 Enrique Condes).
*/

#ifndef FFT_For_ESP8266_h
#define FFT_For_ESP8266_h

#include "Arduino.h"

//#define NDEBUG // Uncomment for production

class FFT_For_ESP8266 {
  public:

  typedef uint8_t output_t; 

  /* ctor */
  FFT_For_ESP8266(short analogPin, int numSamples,
		  int displayWidth, int displayHeight, int numBars = 0, 
		  int numLines = 0, int barWidth = 1); 

  /* Main functions */
  void sampleFromADC(double *data);

  /* Compute FFT from sampled data in data, and store magnitude for each band back into data (only the first size / 2 elements are used) */
  void computeFFT(double *data, double *dataImg);

  /* Build the encoded graph from the fft output data.
     - The total number of columns in the graph is numCols * colWidth.
     - numRows should be 8 for a display with 8 leds
  */
  void buildGraph(uint8_t *out, double *data);

  /* Print the output of build graph on Serial. Warning totalNumCols is numCols * colWidth (not just numCols).
     Does nothing if NDEBUG is defined
  */
  void printGraph(uint8_t *graphData);


  private:
  


  const short _analogPin;
  const int _numSamples; 
  const int _displayWidth; 
  const int _displayHeight;
  int _numBars; 
  int _numLines; 
  int _barWidth; 
  arduinoFFT _fft; 

  static const short _skipLowBands = 1; // skip first n bands  (lower frequencies)

  /* Parameters for the smooth scaling ...
   *
   * The vertical scale of the graph adjusted dynamically based on a
   * maximum value which depends on the average maxima computed over a
   * sliding window of size _windowSize.
   *
   * In order to increase the time span of the sliding window without
   * using too much extra RAM, frames are grouped together and a local
   * maxima is computed for each group of frames. The size of each group
   * is controled by _frameGroupSize
   *
   * The final maximum is computed with the following formula 
   * max( max(current_frame), avg(max(passed_n_frame_group)) ) 
   * 
   * Increasing _windowSize will increase the smoothing but will also
   * use more memory. Increasing _frameGroupSize will achieve a
   * similar result without incrinsing the memory usage, but may
   * result in more abrut changes of the scaling which can be
   * noticeable.
   */
  static const short _windowSize = 4;
  static const short _frameGroupSize = 16;

  double _previousValues[_windowSize]; 
  double _previousSum; 
  short _count; 



  #ifndef NDEBUG
  long _t0;
  #endif


  /* Helper functions (do not export) */
  double maxv(double *data, uint8_t size);

  // return max( max(current_frame), avg(max(passed_n_frames)) ) 
  double smoothMax(double *data, uint8_t size); 
  void startSampling();
  void printSamplingInfo(double *data, uint8_t size);
  void printVector(double *vData, uint8_t bufferSize, uint8_t scaleType);
  uint8_t encodeBar(uint8_t val, uint8_t numRows);



  };

#endif
