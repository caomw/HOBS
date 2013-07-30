# talk in bluetooth with client

import serial, glob, argparse, Queue
from Tkinter import *
from osax import OSAX
parser = argparse.ArgumentParser(description='System testing with slider')
parser.add_argument('--debug', default=False, action='store_true', help='Print More Errors and Launch interactive console on exception or forced exit')
parser.add_argument('--baud', type=int, action='store', default=9600, help='Specify the baud rate')
parser.add_argument('--timeout', type=float, action='store', default=1, help='Timeout parameter for serial connection')

availables = glob.glob('/dev/tty.*')

arguments = parser.parse_args()
try:
	baud = arguments.baud
	timeout = arguments.timeout
	print "  baud rate:", baud
	print "  serial timeout:", timeout
except Exception as e:
	print e
	exit(1)

print "All available ports:" 

index = 1
for port in availables:
	print '  ', index, port
	index += 1

try:
	num = int(raw_input('Enter the number:'))
	if num > 0 and num <= len(availables):
		selected = availables[num-1]
except Exception as e:
	print "invalid input!"
	exit(1)

try:
	ser = serial.Serial(selected, baud, timeout=timeout)
except Exception as e:
  print "failed to connect the serial port", e

sa = OSAX()
old_brightness = 0
old_volume = 0
def change_brightness(new_val):
  global old_brightness, sa
  if abs(int(new_val) - int(old_brightness)) > 1: 
    cmd = "01SBRI%3.0f" % float(new_val)
    ser.write(cmd)  
    old_brightness = new_val  

def change_volume(new_val):
  global old_volume, sa
  if abs(int(new_val) - int(old_volume)) > 1: 
    cmd = "01SVOL%3.0f\n" % float(new_val)
    ser.write(cmd)
    print cmd
    #    sa.set_volume( float(new_val) / 100.0 * 7.14)
    old_volume = new_val  


root = Tk()
var1 = IntVar()
scale1 = Scale( root, variable = var1, command=change_brightness, orient=HORIZONTAL, length=300, label='brightness' )
scale1.pack(anchor=CENTER)

var2 = IntVar()
scale2 = Scale( root, variable = var2, command=change_volume, orient=HORIZONTAL, length=300, label='volume' )
scale2.pack(anchor=CENTER)

root.mainloop()
