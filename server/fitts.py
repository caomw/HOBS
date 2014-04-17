## this is the automatic script to perform Fitts' selection
import serial, glob, argparse, Queue
import sys, select, random, threading, time
from datetime import datetime
from random import shuffle

## standard helper for this script
parser = argparse.ArgumentParser(description='Fitts\' Selection script')
parser.add_argument('--baud', type=int, action='store', default=9600, help='Specify the baud rate')
parser.add_argument('--timeout', type=float, action='store', default=1, help='Timeout parameter for serial connection')
parser.add_argument('--serial', action='store', default=None, help='serial port')
parser.add_argument('--serial2', action='store', default=None, help='serial port 2')

availables = glob.glob('/dev/tty.*')

arguments = parser.parse_args()
try:
  baud = arguments.baud
  timeout = arguments.timeout
  serial_port = arguments.serial
  serial_2 = arguments.serial2
  print "  baud rate:", baud
  print "  serial timeout:", timeout
  print "  serial port:", serial_port
  print "  serial for Arduino:", serial_2
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

target_list = [3, 4, 5, 6, 7, 8, 11, 12, 14, 17]
## target_list = [3, 12, 4]
random.seed( datetime.now().microsecond )

id = random.sample(target_list, 1)[0]

goal = "C" + "%02d" % id

print >> sys.stderr, goal
start_time = datetime.now()
print '[', start_time.strftime('%Y-%m-%d %H:%M:%S %f'), ', Command]:', goal
seconds = 0
line = ""

## reset Arduino and start light
time.sleep(1)
ser.write("L" + "%02d" % id)

while goal not in line:
  if ( ( datetime.now() - start_time).seconds > 20 ):
    line = "D\n"
    ser.write(line)
    break
    
  if ( ( datetime.now() - start_time).seconds  == seconds + 1 ):
    seconds = seconds + 1
    ser.write("L" + "%02d" % id)

  # read line without blocking
  while sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
    line = sys.stdin.readline()
    ts = datetime.now().strftime('%Y-%m-%d %H:%M:%S %f')
    print '[', ts, ', Console]: ', line,
    ser.write(line)
      
  while ser.inWaiting() > 0:
    ts = datetime.now().strftime('%Y-%m-%d %H:%M:%S %f')
    print '[', ts, ', Serial]: ',
    line = ser.readline()
    sys.stdout.write(line)

ser.write("D\n")
ts = datetime.now().strftime('%Y-%m-%d %H:%M:%S %f')
print '[', ts, ', Command]:', "Done"

