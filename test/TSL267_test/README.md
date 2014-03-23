## Summary
This folder contains the code for testing TSL267 functionality and visualizing them.

## Quick start
You need to install [OSCulator](http://www.osculator.net/) and open it for visualization.

To setup the hardware, the procedures:
 
1.  Use an IR emitter.  
2.  Connect the TSL267 outputs to Arduino analog pins (here we use analog pin 4, 5).  
3.  Program Arduino with `TSL267_test.ino` file.  
4.  Open OSCulator.  
5.  Run the following Python code to connect serial output to OSCulator  
  
	`python serialToOSC.py --serial /dev/tty.usbmodem1411 --osc`
	
6.  Click on `/serial1` and `/serial2` in OSCulator to see the c7.  hanges of each TSL267 sensor.
