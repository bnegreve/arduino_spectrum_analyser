Sound spectral analysis on arduino. 
Tested on arduino uno, and on an esp8266. 

There are three subproject.  Spectrum_analyser_c++ is the main subproject, the two other are tests subprojects. 

spectrum_analyser_c++
=====================


Driver for LED Screen. 

For production use, uncomment out NDEBUG in .h file (much faster!).
Note: despite the name of the main class (FFT_For_ESP8266) this should work on most Arduinos. At least, this was tested on Uno. 

![spectrum analyser](https://raw.githubusercontent.com/bnegreve/arduino_spectrum_analyser/master/img/sa.jpg)

Dependencies:
-------------
https://github.com/kosme/arduinoFFT

Compile/run:
------------

Requires arduino-mk to compile with make (sudo apt-get install arduino-mk). 
Also compiles with the arduino software. 

Move to the fft directory
    $ cd spectrum_analyser_c++

To compile type
    $ make

To view output on serial
    $ make console

fft
===

Analog data is picked up from ADC pin A0. The resulting spectrum is visible ASCII stylel on the Serial output. 

Dependencies:
-------------
https://github.com/kosme/arduinoFFT


Compile/run:
------------

Move to the fft directory
    $ cd fft

To compile type
    $ make


To view the output type
    $ make monitor


fht
===

Alternative library, got better results with fft. 

Dependencies:
-------------
http://wiki.openmusiclabs.com/wiki/ArduinoFFT

Compile/run:
------------

Move to the fht directory
    $ cd fht

To compile type
    $ make


To view the output type
    $ make monitor


Known bugs: 
-----------
The fht library isn't compatible with the most recent version of the arduino software. Thus it cannot run on the esp8266 (needed for a certain project). 

I generally got better results (less noisy) with the fht library, altough I suspect it requires more CPU.