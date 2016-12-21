/*
  Spectrum analyser for Florian's LED display. Author Benjamin Negrevergne. 
  Based on example from the  FFT libray (Copyright (C) 2014 Enrique Condes). 
*/

#include "arduinoFFT.h"

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/* 
These values can be changed in order to evaluate the functions 
*/
const uint16_t samples = 128; //This value MUST ALWAYS be a power of 2
double signalFrequency = 1000;
double samplingFrequency = 5000;
uint8_t amplitude = 100;
/* 
These are the input and output vectors 
Input vectors receive computed results from FFT
*/
double vReal[samples]; 
double vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02

#define Theta 6.2831 //2*Pi

void printSpectrum(double *vData, uint8_t size, uint8_t num_rows); 


void setup()
{
  Serial.begin(115200);
  Serial.println("Ready");
  pinMode(0, INPUT);
}

void loop() 
{
  /*
  // Build raw data 
  double cycles = (((samples-1) * signalFrequency) / samplingFrequency); //Number of signal cycles that the sampling will read
  for (uint8_t i = 0; i < samples; i++) 
  {
    vReal[i] = uint8_t((amplitude * (sin((i * (Theta * cycles)) / samples))) / 2.0);// Build data with positive and negative values
    //vReal[i] = uint8_t((amplitude * (sin((i * (6.2831 * cycles)) / samples) + 1.0)) / 2.0);// Build data displaced on the Y axis to include only positive values
  }
  */


  
    long t0, t;
    t0 = micros(); 

    for(uint8_t i = 0; i < samples; i++){
      vReal[i] = (double)analogRead(0);
      vImag[i] = 0;   
      delayMicroseconds(1000); // sample at 0.25Khz
    }
    
    t = micros() - t0; 
    samplingFrequency = (float)samples*1000/t; 
    for(uint8_t i = 0; i < samples; i++){
      Serial.println(vReal[i]); 
    }
  
  Serial.println("Data:");
  //PrintVector(vReal, samples, SCL_TIME);
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */
  Serial.println("Weighed data:");
  //PrintVector(vReal, samples, SCL_TIME);
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
  Serial.println("Computed Real values:");
 // PrintVector(vReal, samples, SCL_INDEX);
  Serial.println("Computed Imaginary values:");
 // PrintVector(vImag, samples, SCL_INDEX);
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
  Serial.println("Computed magnitudes:");
  //PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);  
  //double x = FFT.MajorPeak(vReal, samples, samplingFrequency);
  //Serial.println(x, 6);
    printSpectrum(vReal, (samples>>1), 8);
  //  uint8_t bars[10];  
//printLCDSpectrum(vReal, (samples>>1), bars, 50, 10); 
  //while(1); /* Run Once */
   delay(1000); /* Repeat after delay */
}

int maxv(double *vData, uint8_t size){
  int max = 0;
  for(int i = 0; i < size; i++){
    max = vData[i]>max?vData[i]: max; 
  }
  return max; 
}

void printSpectrum(double *vData, uint8_t size, uint8_t num_rows){
    int max = maxv(vData, size);
    Serial.println();
    for(int j = num_rows - 1; j >= 0 ; j--){
      for(int i = 0; i < size; i++){
        if(((float)vData[i] / max) * num_rows >= j)
          Serial.print("#");
        else
          Serial.print(" ");
      }
      Serial.println();
    }
}


void printBar(uint8_t len){
  for(int i = 0; i < len; i ++)
    Serial.print("#"); 
  Serial.println(); 
}


uint8_t oneCol(uint8_t val){
  uint8_t res = 0;
  for(uint8_t i = 0; i < val; i++){
    res |= (1<<i); 
  }
  return res; 
}

void printLCDSpectrum(double *vData, uint8_t size, uint8_t *out, uint8_t num_rows, uint8_t num_cols){
    int max = maxv(vData, size);
    //for(int j = num_rows - 1; j >= 0 ; j--){
      int x = - 1; 
      int sum = 0;
      int count = 0;  
      for(int i = 0; i < size; i++){
        sum+= vData[i]; 
        count++; 
        if( x != (num_cols / i ) ){
          x = i / num_cols;
          printBar((float)(sum / count) / max * num_rows);
          out[i] = (float)(sum / count) / max * num_rows;           
          count = 0; 
          sum = 0;         
        }
    }
}

void PrintVector(double *vData, uint8_t bufferSize, uint8_t scaleType) 
{
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
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
  break;
    }
    Serial.print(abscissa, 6);
    Serial.print(" ");
    Serial.print(vData[i], 4);
    Serial.println();
  }
  Serial.println();
}
