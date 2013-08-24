# note, select is not guaranteed to be readily used in Windows
# it's Linux functionality

# console for conducting physical acquisition experiment
# sends out cue msg for clients to light up and logs timestamp

import serial, glob, argparse, Queue
import sys, select
import time

parser = argparse.ArgumentParser(description='Pyserial for XBee')
parser.add_argument('--debug', default=False, action='store_true', help='Print More Errors and Launch interactive console on exception or forced exit')
parser.add_argument('--baud', type=int, action='store', default=9600, help='Specify the baud rate')
parser.add_argument('--timeout', type=float, action='store', default=1, help='Timeout parameter for serial connection')
parser.add_argument('--serial', action='store', default=None, help='serial port')

# we know for mac it will show as /dev/usb.tty*, so list all of them and ask user to choose


target_id = "NA"
result_log = []
user_id = "NA"
start_time = time.time()

def logResult(action, obj):
  ts = time.time()
  global start_time
  if action == 'target':
    # reset start time everytime a target is sent
    start_time = ts
  tup = (ts-start_time, action, obj)
  result_log.append(tup)
  print tup

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

print "enter ID"
user_id = sys.stdin.readline()

while True:
  # read line without blocking
  while sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
    line = sys.stdin.readline()
    print '[Console]: ',line
    print 'line length: ',len(line)
    if ('t' in line or 'T' in line) and len(line) == 4:
      # target a client to turn on cue led
      target_id = line[1:3]
      out = target_id + "CSELTAR"
      logResult("target", target_id)
      print 'Target msg: ', out
      print 'out length: ',len(out)
      ser.write(out)
    elif line[0:3] == "end":
      print 'writing results'
      f = open("result_"+user_id[:-1]+".txt", 'w')
      for t in result_log:
        print t
        f.write(' '.join(str(s) for s in t) + '\n') 
      f.close()
      result_log = []

    else:
      ser.write(line)

  while ser.inWaiting() > 0:
    in_msg = ser.readline()
    print '[Serial]: ', in_msg
    if in_msg[0:2] == "FF":
      # a broadcast -> tring to trigger connection
      logResult("connect", '')
    elif in_msg[2:9] == "ASELTAR":
      # is ack for target cue, record timestamp
      logResult("cue shown", '')
    elif in_msg[3:9] == "SEL1st":
      logResult("multiple", in_msg[0:2])
    elif in_msg[3:9] == "SEL NA":
      logResult("miss", '')
    elif in_msg[3:9] == "SEL080":
      logResult("switch", in_msg[0:2])
    elif in_msg[3:9] == "SELAON":
      if in_msg[0:2] == target_id:
        logResult("correct_single", target_id)
        target_id = "NA"
      else:
        logResult("wrong_single", in_msg[0:2])
    elif in_msg[3:9] == "SEL ON":
      # a client is selected, check result
      if in_msg[0:2] == target_id:
        logResult("correct_mul", target_id)
        target_id = "NA"
      else:
        logResult("wrong_mul", in_msg[0:2])
    elif in_msg[3:9] == "SELCAN":
      logResult("cancel_mul", '')





