/*
fht_adc.pde
guest openmusiclabs.com 9.5.12
example sketch for testing the fht library.
it takes in data on ADC0 (Analog0) and processes them
with the fht. the data is sent out over the serial
port at 115.2kb.  there is a pure data patch for
visualizing the data.
*/

//#define LOG_OUT 1 // use the log output function
#define LIN_OUT 1 // use the log output function
//#define OCTAVE 1
#define FHT_N 256// set to 256 point fht
#define BANDS_N 8 // should be log2(FHT_N/2)+1
#define ROW_N 32 // number of rows in the spectrum analyser 
#include <FHT.h> // include the library

void setup() {
  Serial.begin(115200); // use the serial port
  //Serial.begin(9600); // use the serial port
 
  // TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
}

void loop() {
  while(1) { // reduces jitter
 //   uint16_t x = analogRead(0); 

    long t0, t;
    t0 = micros(); 
    //    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      
      fht_input[i] = k; // put real data into bins
      delayMicroseconds(4000); // sample at 0.25Khz
    }
    //    sei();

    // freq in Khz
    float sampling_freq = (float)FHT_N*1000/t; 
    t = micros() - t0; 

    Serial.print("Time for all samples (ms): ");
    Serial.println((float)t / 1000); 
    Serial.print("Time per sample: ");
    Serial.println((float)t/FHT_N);
    Serial.print("Frequency (kHz): ");
    Serial.println(sampling_freq);
    Serial.println();
    

    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_lin(); // take the output of the fht
    //fht_mag_octave(); // take the output of the fht

    /* //Print octaves */
    /* Serial.println(); */
    /* for(int j = ROW_N-1; j >= 0 ; j--){ */
    /*   for(int i = 0; i < BANDS_N; i++){ */
    /*    if(fht_oct_out[i] / ROW_N > j) */
    /*      Serial.print("#"); */
    /*      else */
    /*     Serial.print(" "); */
    /*   } */
    /*   Serial.println(); */
    /* } */




    /* int start = 0; */
    /* int end = 1; */

    /* int maxima[BANDS_N]= {0}; */
    /* float frequencies[BANDS_N]= {0}; */
    /* for(int i = 0; i < BANDS_N; i++){ */

    /*   frequencies[i] = sampling_freq * start; */

    /*   Serial.print("computing max for frequency range : "); */
    /*   Serial.print(frequencies[i]); */
    /*   Serial.print("kHz, index "); */
    /*   Serial.print(start); */
    /*   Serial.print(" to "); */
    /*   Serial.print(end); */
    /*   Serial.print(" value: "); */

    /*   Compute the maximum value in this frequency window */
    /*   for(int j = start; j < end; j++){ */
    /* 	maxima[i] = fht_log_out[j]>maxima[i]?fht_log_out[j]:maxima[i]; */
    /*   } */

    /*   Serial.println(maxima[i]); */

    /*   Shift to the next window */
    /*   start = end; */
    /*   end <<= 1; */
    /* } */

    /* Serial.println(); */
    /* for(int j = ROW_N-1; j >= 0 ; j--){ */
    /*   for(int i = 0; i < BANDS_N; i++){ */
    /*    if(maxima[i] / ROW_N > j) */
    /*      Serial.print("#"); */
    /*      else */
    /*     Serial.print(" "); */
    /*   } */
    /*   Serial.println(); */
    /* } */



    /* Serial.println("frequencies: "); */
    /* for(int i = 0; i < FHT_N/2; i++){ */
    /*   float freq = sampling_freq * i; */
    /*   Serial.print(freq); */
    /*   Serial.print("khz "); */
    /* } */
    /* Serial.println(); */

    /* Serial.println(); */
    /* for(int j = ROW_N-1; j >= 0 ; j--){ */
    /*   for(int i = 0; i < BANDS_N; i++){ */
    /*    if(maxima[i] / ROW_N > j) */
    /*      Serial.print("#"); */
    /*    else */
    /*     Serial.print(" "); */
    /*   } */
    /*   Serial.println(); */
    /* } */


    /* Serial.println(); */
    /* for(int i = 0; i < FHT_N/2; i++){ */
    /*   Serial.println(fht_lin_out[i]); */
    /* } */
	

    int max = 0; 
    for(int i = 0; i < FHT_N/2; i++){
      max = fht_lin_out[i]>max?fht_lin_out[i]:max; 
    }
      


    Serial.println();
    for(int j = ROW_N-1; j >= 0 ; j--){
      for(int i = 0; i < FHT_N/2; i++){
    	//	if(fht_log_out[i] / (256/ROW_N) >= j)
    	if(((float)fht_lin_out[i] / max) * ROW_N >= j)
         Serial.print("#");
         else
        Serial.print(" ");
      }
      Serial.println();
    }

    Serial.println(max);

    delay(500);
    
  }
}
