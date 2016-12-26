#include <arduinoFFT.h>
#include <assert.h>

#include "fft_for_esp8266.h"

/* Required for speeding up the ADC */
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

/* Fake signal generation */
//#define GENERATE_FAKE_SIGNAL 1 //Generate fake signal instead of reading from ADC
#define Theta 6.2831 //2*Pi
double signalFrequency = 2000;
double samplingFrequency = 5000;
uint8_t amplitude = 50;

/* Misc values for printVector */
#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02



FFT_For_ESP8266::FFT_For_ESP8266(short analogPin, int numSamples,
				 int displayWidth, int displayHeight,
				 int numLines, int numBars, int barWidth):
  _analogPin(analogPin), 
  _numSamples(numSamples), 
  _displayWidth(displayWidth), 
  _displayHeight(displayHeight),
  _numBars(numBars),
  _numLines(numLines),
  _barWidth(barWidth), 
  _fft(arduinoFFT()),
  _previousValues({0}), 
  _previousSum(0), 
  _count(0){

  // TODO add skipLine
  if(numBars == 0)
    _numBars = displayWidth / barWidth;

  if(numLines == 0)
    _numLines = displayHeight; 

  assert(_numBars * _barWidth <= _displayWidth); 
  assert(_numLines <= _displayHeight);

  /* output_t should be big enough to store one column stored as zeros and ones */
  assert(sizeof(output_t) * 8 >= log(_displayHeight) / log(2));
  assert(_numBars <= _numSamples / 2); 
}

void FFT_For_ESP8266::sampleFromADC(double *data){
  startSampling();

  #ifdef GENERATE_FAKE_SIGNAL
  double cycles = (((_numSamples-1) * signalFrequency) / samplingFrequency); //Number of signal cycles that the sampling will read
  for (uint8_t i = 0; i < _numSamples; i++){
    data[i] = uint8_t((amplitude * (sin((i * (Theta * cycles)) / _numSamples))) / 2.0);// Build data with positive and negative values
    //data[i] = uint8_t((amplitude * (sin((i * (6.2831 * cycles)) / _numSamples) + 1.0)) / 2.0);// Build data displaced on the Y axis to include only positive values
  }
  #else
  for(uint8_t i = 0; i < _numSamples; i++){
    data[i] = (double)analogRead(_analogPin);
  }
  #endif

  printSamplingInfo(data, _numSamples);

}

void FFT_For_ESP8266::computeFFT(double *data, double *dataImg){

  _fft.Windowing(data, _numSamples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */
  //Serial.println("Weighed data:");
  //printVector(vReal, _numSamples, SCL_TIME);

  _fft.Compute(data, dataImg, _numSamples, FFT_FORWARD); /* Compute FFT */
  //Serial.println("Computed Real values:");
  //printVector(vReal, _numSamples, SCL_INDEX);
  //Serial.println("Computed Imaginary values:");
  //printVector(dataImg, _numSamples, SCL_INDEX);

  _fft.ComplexToMagnitude(data, dataImg, _numSamples); /* Compute magnitudes */

  #ifndef NDEBUG
  Serial.println("Computed magnitudes:");
  //  printVector(data, (_numSamples >> 1), SCL_FREQUENCY);
  #endif

}

void FFT_For_ESP8266::buildGraph(uint8_t *out, double *data){
  assert(_numLines <= _displayHeight);
  assert(_numBars * _barWidth <= _displayWidth);

  int numBands = _numSamples / 2 - _skipLowBands; 
  
  double scalingFactor = (_numLines + 1) / avgMax(data, numBands);  // used to bring all values back in the
                                // interval [0 - _numLines] (inclusive because
                                // we can display 9 distinct values with 8 leds.)

  /* Loop through display columns */
  for(int i = 0; i < _numBars; i++){
    uint8_t bandStart = ((float)i / _numBars * numBands) + _skipLowBands; 
    uint8_t bandEnd = ((float)(i+1) / _numBars * numBands) + _skipLowBands;

    double barHeight = 0;
    for(uint8_t j = bandStart; j < bandEnd; j++){
      /* take the maximum value of all bands displayed by this column */
      barHeight = max(data[j],  barHeight);
    }

    #ifndef NDEBUG
    Serial.print("Col ");
    Serial.print(i);
    Serial.print(" : Band from ");
    Serial.print(bandStart);
    Serial.print(" to ");
    Serial.print(bandEnd);
    Serial.print(": ");
    Serial.print(barHeight);
    Serial.println();
    #endif

    for(uint8_t j = i * _barWidth; j < (i+1) * _barWidth; j++){
      out[j] = encodeBar(barHeight * scalingFactor, _numLines);
    }
  }
}

void FFT_For_ESP8266::printGraph(uint8_t *graphData){
  //#ifndef NDEBUG
  Serial.println("--------------- spectrum analyser input-------------------");
  for(int i = 0; i < _displayWidth; i++){
    Serial.print(graphData[i], HEX);
  }
  Serial.println();

  Serial.println("------------------ spectrum analyser ---------------------");
  for(int i = _displayHeight - 1 ; i >= 0; i--){
    for(int j = 0; j < _displayWidth; j++){
      if( graphData[j] & (1<<i) )
          Serial.print("#");
      else
          Serial.print(" ");
    }
    Serial.println();
  }
  for(int i = 0; i < _displayWidth; i++)
    Serial.print("-");
  Serial.println();
  //#endif
}


uint8_t FFT_For_ESP8266::encodeBar(uint8_t val, uint8_t _numLines){

  if(val >= _numLines) {
    // Serial.println((1<<_numLines) - 1, HEX);
    return (1<<_numLines) - 1; // all led on.
  }

  uint8_t res = 0;
  for(uint8_t i = 0; i < val; i++){
    res |= (1<<i);
  }
  return res;
}

/* Compute the maximum value of a vector of size > 0 */
double FFT_For_ESP8266::maxv(double *data, uint8_t size){
  assert(size != 0);
  assert(size - _skipLowBands > 0);

  int i = _skipLowBands; 
  double max = data[i];
  for(i = _skipLowBands+1; i < size; i++){
    max = data[i]>max?data[i]: max;
  }
  return max;
}

double FFT_For_ESP8266::avgMax(double *data, uint8_t size){
  /* every 16 frames, shift all values by one, drop the oldest value and set the new value to 0 */

  if( _count++ % 16  == 0) {

    // Substract oldest max, add newst max
    _previousSum -= _previousValues[0]; 
    _previousSum += _previousValues[_windowSize-1];

    // shift all values by one in previousValues
    for(int i = 1; i < _windowSize; i++){
      _previousValues[i-1] = _previousValues[i];
    }

    // Reset most recent max to 0.
    _previousValues[ _windowSize -1 ] = 0; 
  }

    // set new value
    _previousValues[ _windowSize - 1 ] = max(_previousValues[ _windowSize - 1 ],
					     maxv(data, size));
    // printVector(_previousValues, _windowSize, SCL_INDEX); 
 
    double sum = _previousSum + _previousValues[ _windowSize - 1 ];
    Serial.println(sum / _windowSize ); 
    return sum / _windowSize; 
}


void FFT_For_ESP8266::startSampling(){
#ifndef NDEBUG
    _t0 = micros();
#endif
}

void FFT_For_ESP8266::printSamplingInfo(double *data, uint8_t size){
#ifndef NDEBUG
    float t = (micros() - _t0) / 1.0E3; // time (ms)

    Serial.print("Sampling time : ");
    Serial.println(t);

    Serial.print("Time per sample (ms) : ");
    Serial.println(t/size);

    Serial.print("Sampling frequency (hz) : ");
    Serial.println((1 / (t/1E3)) * size);

    Serial.print("Max ADC value: ");
    Serial.println(maxv(data, size));
#endif
}


void FFT_For_ESP8266::printVector(double *vData, uint8_t bufferSize, uint8_t scaleType)
{
#ifndef NDEBUG
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
  break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
  break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / _numSamples);
  break;
    }
    Serial.print(abscissa, 6);
    Serial.print(" ");
    Serial.print(vData[i], 4);
    Serial.println();
  }
  Serial.println();
#endif
}


