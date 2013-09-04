# note, select is not guaranteed to be readily used in Windows
# it's Linux functionality

# console for conducting physical acquisition experiment
# sends out cue msg for clients to light up and logs timestamp

import serial, glob, argparse, Queue
import sys, select
import time
import random
from time import sleep

parser = argparse.ArgumentParser(description='Pyserial for XBee')
parser.add_argument('--debug', default=False, action='store_true', help='Print More Errors and Launch interactive console on exception or forced exit')
parser.add_argument('--baud', type=int, action='store', default=9600, help='Specify the baud rate')
parser.add_argument('--timeout', type=float, action='store', default=1, help='Timeout parameter for serial connection')
parser.add_argument('--serial', action='store', default=None, help='serial port')

# we know for mac it will show as /dev/usb.tty*, so list all of them and ask user to choose


target_id = "NA"
result_log = []
user_id = "NA"
ser = None
start_time = time.time()

target_list = []
predefined_list = ['01', '02', '01', '01', '02']
total_client_no = 2
total_task_round = 10
list_cursor = 0
task_interval = 3  # after selecting a correct candidate, wait for couple seconds till next target shows
state_cue = True
correct_target = False

MODE_PREDEFINE = 1
MODE_RANDOM = 2
MODE_MANUAL = 3
exp_mode = 0

def logResult(action, obj):
  ts = time.time()
  global start_time
  if action == 'target':
    # reset start time everytime a target is sent
    start_time = ts
  tup = (ts-start_time, action, obj)
  result_log.append(tup)
  print tup

def prepare_target_list(type):
  print 'populate, type: ', type
  result = []
  if type == 1: # predefined
    result = predefined_list
  elif type == 2: # random
    for i in range(total_task_round):
      t = random.randint(1,total_client_no)
      result.append("%02d" % t)
  elif type == 3:
    restult = []
  print 'list: ', result
  return result

def end_exp():
  print 'writing results'
  global user_id, result_log, ser
  f = open("result_"+user_id[:-1]+".txt", 'w')
  for t in result_log:
    print t
    f.write(' '.join(str(s) for s in t) + '\n') 
  f.close()
  result_log = []
  ser.write('XXCSELTAR')  # turn off all cue leds
  exit(0)

def send_cue(tid):
  # print "debug:", tid
  out = tid + "CSELTAR"
  logResult("target", tid)
  global list_cursor
  print 'round', list_cursor+1
  print 'Target msg:', out
  print 'out length:', len(out)
  list_cursor += 1
  return out

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

print "1. auto from predefined list"
print "2. auto from random"
print "3. manually enter (e.g. T01)"

try:
  num = int(raw_input('Enter the number:'))
  if num > 0 and num <= 3:
    exp_mode = num
    target_list = prepare_target_list(exp_mode)
  else:
    print 'out of range'
    exit(1)
except Exception as e:
  print "invalid input!"
  exit(1)

while True:
  if state_cue == True:
    if exp_mode == MODE_MANUAL:
    # read line without blocking
      # while sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
      #   line = sys.stdin.readline()
      #   print '[Console]: ',line
      #   print 'line length: ',len(line)
      #   if ('t' in line or 'T' in line) and len(line) == 4:
      #     # target a client to turn on cue led
      #     target_id = line[1:3]
      #     state_cue = False
      #   elif line[0:3] == "end":
      #     end_exp()
      #   else:
      #     ser.write(line)

      cmd = raw_input('Enter command:')  
      print '[Console]: ',cmd
      print 'line length: ',len(cmd)
      if ('t' in cmd or 'T' in cmd) and len(cmd) == 3:
        # target a client to turn on cue led
        print 'target set to', cmd[1:3]
        target_id = cmd[1:3]
        state_cue = False
      elif cmd[0:3] == "end":
        end_exp()
      else:
        ser.write(cmd)
    else: # predefined or random list
      if list_cursor > len(target_list) - 1:
        print 'exp done'
        end_exp()
        exit(0)
      target_id = target_list[list_cursor]
      state_cue = False

    out = send_cue(target_id)
    ser.write(out)
  
  # can reset or end exp within a task phase
  while sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
    line = sys.stdin.readline()
    print '[Console]: ',line
    print 'line length: ',len(line)
    if line[0:5] == 'reset':
      # repeat the cue stage (if cue ack isn't received)
      list_cursor -= 1
      state_cue = True

    elif line[0:3] == "end":
      end_exp()
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
      logResult("cue shown", in_msg[0:2])
    elif in_msg[3:9] == "SEL1st":
      logResult("multiple", in_msg[0:2])
    elif in_msg[3:9] == "SEL NA":
      logResult("miss", '')
    elif in_msg[3:9] == "SEL080":
      logResult("switch", in_msg[0:2])
    elif in_msg[3:9] == "SELAON":
      if in_msg[0:2] == target_id:
        logResult("correct_single", target_id)
      else:
        logResult("wrong_single", in_msg[0:2])
    elif in_msg[3:9] == "SEL ON":
      # a client is selected, check result
      if in_msg[0:2] == target_id:
        logResult("correct_mul", target_id)
      else:
        logResult("wrong_mul", in_msg[0:2])
    elif in_msg[3:9] == "SELCAN":
      logResult("cancel_mul", '')
    elif in_msg[3:9] == "SELOFF":
      # determine whether disconnected from correct target
      if in_msg[0:2] == target_id:
        logResult("disconnected_right", target_id)
        target_id = "NA"
        state_cue = True
        print 'waiting', task_interval, 'sec to cue next target'
        sleep(task_interval)  # wait 3s to cue next target
      else:
        logResult("disconnected_wrong", target_id)





