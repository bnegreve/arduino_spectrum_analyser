/*
  Spectrum analyser for Florian's LED display. Author Benjamin Negrevergne. 
  Based on example from the  FFT libray (Copyright (C) 2014 Enrique Condes). 
  Author: Benjamin Negrevergne
*/

#include <assert.h>
#include "arduinoFFT.h"

/* Global vars & defines  */

//#define NDEBUG // Uncomment for production 
#define DISPLAY_WIDTH 48
#define DISPLAY_HEIGHT 8
#define WINDOW_SIZE 16 // Compute the maximum signal over the WIDOW_SIZE passed values // do not increase too much, uses memory

/* FFT variables */
const uint16_t numSamples = 128; //This value MUST ALWAYS be a power of 2
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */


/* Required for speeding up the ADC */ 
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

/* Fake signal generation */
#define GENERATE_FAKE_SIGNAL 1 //Generate fake signal instead of reading from ADC
#define Theta 6.2831 //2*Pi
double signalFrequency = 2000;
double samplingFrequency = 5000;
uint8_t amplitude = 50;


/* Misc values for printVector */
#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02

/*Timing */
long t0;


/* Main functions */ 
void sampleFromADC(double *data, uint8_t numSamples);

/* Compute FFT from sampled data in data, and store magnitude for each band back into data (only the first size / 2 elements are used) */
void computeFFT(double *data, uint8_t numSamples);

/* Build the encoded graph from the fft output data. 
- The total number of columns in the graph is numCols * colWidth. 
- numRows should be 8 for a display with 8 leds 
*/
void buildGraph(uint8_t *out, double *data,  uint8_t numBands, uint8_t numRows, uint8_t numCols, uint8_t colWidth); 

/* Print the output of build graph on Serial. Warning totalNumCols is numCols * colWidth (not just numCols). 
   Does nothing if NDEBUG is defined 
*/
void printGraph(uint8_t *graphData, uint8_t numRows, uint8_t totalNumCols); 


/* Helper functions (do not export) */ 
static double maxv(double *data, uint8_t size);
static double maxSlidingWindow(double *data, uint8_t size); // compute the maximum value over a series of past values. 
static void startSampling();
static void printSamplingInfo(double *data, uint8_t size);
static void printVector(double *vData, uint8_t bufferSize, uint8_t scaleType); 
static uint8_t encodeBar(uint8_t val, uint8_t numRows); 

void setup()
{

#ifndef NDEBUG
  Serial.begin(115200);
  Serial.println("Ready");
  pinMode(0, INPUT);
#endif 

  /* Speedup the ADC */
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
}

void loop() 
{

  double data[numSamples]; 
  uint8_t bars[DISPLAY_WIDTH] = {0};  
  uint8_t colWidth = 1; 

  sampleFromADC(data, numSamples); 
  computeFFT(data, numSamples); 
  buildGraph(bars, data, numSamples/2, DISPLAY_HEIGHT, DISPLAY_WIDTH, colWidth);
  printGraph(bars, 8, DISPLAY_WIDTH); 
  delay(1000); /* Repeat after delay */

}

void sampleFromADC(double *data, uint8_t numSamples){
  startSampling(); 
    
  #ifdef GENERATE_FAKE_SIGNAL
  double cycles = (((numSamples-1) * signalFrequency) / samplingFrequency); //Number of signal cycles that the sampling will read
  for (uint8_t i = 0; i < numSamples; i++){
    data[i] = uint8_t((amplitude * (sin((i * (Theta * cycles)) / numSamples))) / 2.0);// Build data with positive and negative values
    //data[i] = uint8_t((amplitude * (sin((i * (6.2831 * cycles)) / numSamples) + 1.0)) / 2.0);// Build data displaced on the Y axis to include only positive values
  }
  #else
  for(uint8_t i = 0; i < numSamples; i++){
    data[i] = (double)analogRead(0);
  }
  #endif 

  printSamplingInfo(data, numSamples); 
 
}

void computeFFT(double *data, uint8_t numSamples){

  double vImag[numSamples] = {0}; // required!
  //data is vReal[numSamples] ..
  
  FFT.Windowing(data, numSamples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */
  //Serial.println("Weighed data:");
  //printVector(vReal, numSamples, SCL_TIME);

  FFT.Compute(data, vImag, numSamples, FFT_FORWARD); /* Compute FFT */
  //Serial.println("Computed Real values:");
  //printVector(vReal, numSamples, SCL_INDEX);
  //Serial.println("Computed Imaginary values:");
  //printVector(vImag, numSamples, SCL_INDEX);

  FFT.ComplexToMagnitude(data, vImag, numSamples); /* Compute magnitudes */

  #ifdef NDEBUG
  Serial.println("Computed magnitudes:");
  printVector(data, (numSamples >> 1), SCL_FREQUENCY);
  #endif

}

void buildGraph(uint8_t *out, double *data,  uint8_t numBands, uint8_t numRows, uint8_t numCols, uint8_t colWidth){

  assert(numRows <= DISPLAY_HEIGHT); 
  assert(numCols * colWidth <= DISPLAY_WIDTH); 


  double scalingFactor = (numRows + 1) / maxSlidingWindow(data, numBands);  // used to bring all values back in the
								// interval [0 - numRows] (inclusive because
								// we can display 9 distinct values with 8 leds.)

  /* Loop through display columns */
  for(int i = 0; i < numCols; i++){
    uint8_t bandStart = (float)i / numCols * numBands;
    uint8_t bandEnd = (float)(i+1) / numCols * numBands;
    double barHeight = 0; 
    for(uint8_t j = bandStart; j < bandEnd; j++){
      /* take the maximum value of all bands displayed by this column */ 
      barHeight = data[j] > barHeight ? data[j] : barHeight; 
    }

    #ifndef DNDEBUG
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

    for(uint8_t j = i * colWidth; j < (i+1) * colWidth; j++){
      uint8_t x = barHeight * scalingFactor;
      out[j] = encodeBar(barHeight * scalingFactor, numRows); 
    }
  }
}

void printGraph(uint8_t *graphData, uint8_t numRows, uint8_t totalNumCols){
#ifndef NDEBUG
  assert(totalNumCols <= DISPLAY_WIDTH); 
  assert(numRows <= DISPLAY_HEIGHT); 

  Serial.println("--------------- spectrum analyser input-------------------"); 
  for(int i = 0; i < totalNumCols; i++){
    Serial.print(graphData[i], HEX); 
  }
  Serial.println(); 

  Serial.println("------------------ spectrum analyser ---------------------"); 
  for(int i = numRows - 1 ; i >= 0; i--){
    for(int j = 0; j < totalNumCols; j++){
      if( graphData[j] & (1<<i) )
      	Serial.print("#");
      else
      	Serial.print(" ");
    }
    Serial.println(); 
  }
  for(int i = 0; i < totalNumCols; i++)
    Serial.print("-"); 
  Serial.println();
#endif
}


static uint8_t encodeBar(uint8_t val, uint8_t numRows){
  if(val >= numRows) {
    Serial.println((1<<numRows) - 1, HEX); 
    return (1<<numRows) - 1; // all led on.
  }

  uint8_t res = 0;
  for(uint8_t i = 0; i < val; i++){
    res |= (1<<i); 
  }
  return res; 
}

/* Compute the maximum value of a vector of size > 0 */
static double maxv(double *data, uint8_t size){
  assert(size != 0); 
  double max = data[0];
  for(int i = 1; i < size; i++){
    max = data[i]>max?data[i]: max; 
  }
  return max; 
}

static double maxSlidingWindow(double *data, uint8_t size){
  /* Maybe a linked list would be better ...*/
  
  static double previousValues[WINDOW_SIZE] = {0}; 
  static double previousMax = 0;  // always contains the maximum for all value except the last record. 
  static uint8_t count = 0; 


  /* every 255 frames, shift all values by one, drop the oldest value and set the new value to 0 */
  if( count++ == 0 ) { 
    previousMax = 0; // reset previousMax
    for(int i = 1; i < WINDOW_SIZE; i++){
      previousValues[i-1] = previousValues[i]; 
      previousMax = max(previousValues[i-1], previousMax); 
    }
  }
  previousValues[WINDOW_SIZE - 1 ] = 0; 

  /* compute the new value for data and keep the local maximum of this value */ 
  double newVal = maxv(data, size);  
  previousValues[WINDOW_SIZE - 1] =  max(previousValues[WINDOW_SIZE - 1], newVal);
  
  double currentMax = max(previousMax,   previousValues[WINDOW_SIZE - 1]); 
  Serial.print("Current sliding maximum : ");
  Serial.println(currentMax); 

  return currentMax;   
}



static void startSampling(){
#ifndef NDEBUG
    t0 = micros(); 
#endif
}
static void printSamplingInfo(double *data, uint8_t size){
#ifndef NDEBUG    
    float t = (micros() - t0) / 1.0E3; // time (ms) 

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


static void printVector(double *vData, uint8_t bufferSize, uint8_t scaleType) 
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
        abscissa = ((i * 1.0 * samplingFrequency) / numSamples);
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

