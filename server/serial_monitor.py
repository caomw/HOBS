# note, select is not guaranteed to be readily used in Windows
# it's Linux functionality

import serial, glob, argparse, Queue
import sys, select
from datetime import datetime

parser = argparse.ArgumentParser(description='Pyserial Monitor')
parser.add_argument('--baud', type=int, action='store', default=9600, help='Specify the baud rate')
parser.add_argument('--timeout', type=float, action='store', default=1, help='Timeout parameter for serial connection')
parser.add_argument('--serial', action='store', default=None, help='serial port')
parser.add_argument('--osc', default=False, action='store_true', help='send readings to OSC for visualization')

# we know for mac it will show as /dev/usb.tty*, so list all of them and ask user to choose

availables = glob.glob('/dev/tty.*')

arguments = parser.parse_args()
try:
  baud = arguments.baud
  timeout = arguments.timeout
  serial_port = arguments.serial
  is_osc_on = arguments.osc
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

if is_osc_on:
  import OSC
  def osc_send(addr, data):
    msg = OSC.OSCMessage() #  we reuse the same variable msg used above overwriting it
    msg.setAddress(addr)
    msg.append(data)
    client.send(msg) # now we dont need to tell the client the address anymore

  sys.stderr.write( "Starting up OSC...\n")
  client = OSC.OSCClient()
  client.connect( ('127.0.0.1', 8000) ) # note that the argument is a tupple and not two arguments

while True:
  # read line without blocking
  while sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
    line = sys.stdin.readline()
    ts = datetime.now().strftime('%Y-%m-%d %H:%M:%S %f')
    print '[', ts, '| Console]: ', line,
    ser.write(line)

  while ser.inWaiting() > 0:
    ts = datetime.now().strftime('%Y-%m-%d %H:%M:%S %f')
    print '[', ts, '| Serial]: ',
    line = ser.readline()
    sys.stdout.write(line)
    if len(line.strip()) > 0 and is_osc_on:
      try:
        osc_send(line[0], int(line[2:])/1024.0)
      except ValueError:
        pass
