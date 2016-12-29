
#include <arduinoFFT.h>

#include "fft_for_esp8266.h"

/* Required for speeding up the ADC */ 
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

void setup()
{

  Serial.begin(115200);
  Serial.println("Ready");
  pinMode(0, INPUT);

  /* Speedup the ADC */
  // See http://i187.photobucket.com/albums/x269/jmknapp/adc_prescale.png
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);
}

void loop() 
{

  //  double buffer[128]; 

  //FFT_For_ESP8266 fft(0, 128, 48, 8, NULL, 0, 8, 2, 0 );
  FFT_For_ESP8266 fft(0, 128, 48, 8, NULL, 10, 8, 2, 0);

  for(;;){

    fft.sampleFromADC();
    fft.computeFFT(); 
    output_t *bars = fft.buildGraph();
    fft.printGraph(bars);

    delay(1000);
  }
  

}
