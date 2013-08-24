# note, select is not guaranteed to be readily used in Windows
# it's Linux functionality

import serial, glob, argparse, Queue
import sys, select

parser = argparse.ArgumentParser(description='Pyserial for XBee')
parser.add_argument('--debug', default=False, action='store_true', help='Print More Errors and Launch interactive console on exception or forced exit')
parser.add_argument('--baud', type=int, action='store', default=9600, help='Specify the baud rate')
parser.add_argument('--timeout', type=float, action='store', default=1, help='Timeout parameter for serial connection')
parser.add_argument('--serial', action='store', default=None, help='serial port')

# we know for mac it will show as /dev/usb.tty*, so list all of them and ask user to choose

availables = glob.glob('/dev/tty.*')

arguments = parser.parse_args()
try:
  baud = arguments.baud
  timeout = arguments.timeout
  serial_port = arguments.serial
  print "  baud rate:", baud
  print "  serial timeout:", timeout
  print "  serial port:", serial_port
except Exception as e:
	print e
	exit(1)

if serial_port:
  selected = serial_port
else:
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

while True:

  # read line without blocking
  while sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
    line = sys.stdin.readline()
    print '[Console]: ', line,
    ser.write(line)

  while ser.inWaiting() > 0:
    print '[Serial]: ',
    sys.stdout.write(ser.readline())
