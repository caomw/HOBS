# keystroke map
# http://i.stack.imgur.com/z8boX.png

import serial, glob, argparse
import sys
import re
from osax import OSAX
from appscript import app, k
import applescript 

parser = argparse.ArgumentParser(description='Adapter that controls laptop through USB')
parser.add_argument('--debug', default=False, action='store_true', help='Print More Errors and Launch interactive console on exception or forced exit')
parser.add_argument('--baud', type=int, action='store', default=9600, help='Specify the baud rate')
parser.add_argument('--timeout', type=float, action='store', default=1, help='Timeout parameter for serial connection')

# we know for mac it will show as /dev/usb.tty*, so list all of them and ask user to choose

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

def change_volume(sa, amount):
  current_settings = sa.get_volume_settings()
  original = -1
  # hacky way of finding out current volume
  for key in current_settings:
    if str(key) == 'k.output_volume':
      original = current_settings[key]
  if original == -1:
    raise Exception('Problem in parsing get_volume_settings output')
  print original, amount
  # a number between 0 to 7
  # original + amount is 0 to 100
  sa.set_volume((original+amount) / 100.0 * 7)

def get_volume():
  current_settings = sa.get_volume_settings()
  original = -1
  # hacky way of finding out current volume
  for key in current_settings:
    if str(key) == 'k.output_volume':
      original = current_settings[key]
  if original == -1:
    raise Exception('Problem in parsing get_volume_settings output')
  return original
  
def set_brightness(val):
  try:
    scpt.call('changeBrightness', val)
  except applescript.ScriptError:
    pass

  
def key_stroke(kcode):
  try:
    return scpt.call('doKeyStorke', kcode)
  except applescript.ScriptError:
    return 0
  
def get_brightness():
  try:
    return scpt.call('getBrightness')
  except applescript.ScriptError:
    return 0

sa = OSAX()
scpt = applescript.AppleScript('''
    on doKeyStorke(keycode)
      tell application "System Events" to key code keycode
    end doKeyStorke
    on changeBrightness(BrightnessValue)	
	    tell application "System Preferences"
		    --activate
		    set current pane to pane "com.apple.preference.displays"
    	end tell
    	tell application "System Events" to tell process "System Preferences"		
    		tell slider 1 of group 1 of tab group 1 of window 1 to set value to BrightnessValue
    	end tell
    	tell application "System Preferences" to quit
    end changeBrightness

    on getBrightness()
	    tell application "System Preferences"
		    --activate
		    set current pane to pane "com.apple.preference.displays"
	    end tell
	    tell application "System Events" to tell process "System Preferences"
		    set x to value of slider 1 of group 1 of tab group 1 of window 1
	    end tell
	    tell application "System Preferences" to quit
      return x
    end getBrightness
''')


class command():
  """Used to parse and save the command"""
  def __init__(self, str):
    self.id = str[0:2]
    self.func = str[2]
    self.var = str[3:6]
    self.data = str[6:9]
    
  def to_string (self):
    str_list = [self.id, self.func, self.var, self.data]
    return ''.join(str_list)
    
def validate(str):
  if re.match(r'^\d\d(R|S|C)(BRI|VOL|VID)...\r?\n$', str):
    return True
  
class bcolors:
  HEADER = '\033[95m'
  OKBLUE = '\033[94m'
  OKGREEN = '\033[92m'
  WARNING = '\033[93m'
  FAIL = '\033[91m'
  ENDC = '\033[0m'

  def disable(self):
    self.HEADER = ''
    self.OKBLUE = ''
    self.OKGREEN = ''
    self.WARNING = ''
    self.FAIL = ''
    self.ENDC = ''

        
last_volume = get_volume()
last_brightness = get_brightness()
  
while True:
  char = ser.readline()
  if len(char) > 0:
    print len(char), validate(char), " || ", char.strip()
    
  # packet format + CR LF
  if len(char) == 10 or len(char) == 11 and validate(char):
      cmd = command(char)
      print bcolors.WARNING + cmd.func, cmd.var, cmd.data  + bcolors.ENDC


      if cmd.func == 'R':
        if cmd.var == 'BRI':
          cmd.data = "%03d" % (100 * get_brightness())
          print cmd.data
        elif cmd.var == 'VOL':
          cmd.data = "%03d" % (get_volume())
          # TODO: make the string to be 0XX if it's less than 100
        elif cmd.var == 'VID':
          cmd.data = '000'
        else:
          raise NotImplementedError
        cmd.func = 'A'
        print "from python to XBee:", cmd.to_string()
        ser.write(cmd.to_string())
      elif cmd.func == 'S':
        try:
          val = float(cmd.data)
        except:
          continue
        if cmd.var == 'BRI':
          set_brightness( val / 100.0);
        elif cmd.var == 'VOL':
          sa.set_volume( val / 100.0 * 7.14)
        elif cmd.var == 'VID' and val == 0.0:
            app('System Events').keystroke('0') 
        else:
          raise NotImplementedError         
      elif cmd.func == 'C':
        if cmd.var == 'VID':
          if cmd.data == ' ON' or cmd.data == 'OFF':
            app('System Events').keystroke(' ') 
          elif cmd.data == 'INC':
            key_stroke(124)
          elif cmd.data == 'DEC':
            key_stroke(123)                      
        elif cmd.var == 'VOL':
          if cmd.data == ' ON':
            print "controling volume on", last_volume 
            sa.set_volume( last_volume/100.0 * 7.14 )
          elif cmd.data == 'OFF':
            last_volume = get_volume()
            print "turning volume off", last_volume 
            sa.set_volume(0.0)
        elif cmd.var == 'BRI':
          if cmd.data == ' ON':
            print "brightness on", last_brightness 
            set_brightness(last_brightness)
          elif cmd.data == 'OFF':
            last_brightness = get_brightness()
            print "brightness off", last_brightness
            set_brightness(0)
        else:
          raise NotImplementedError
      cmd.func = 'A'
      # print "from python to XBee:", cmd.to_string()
      # ser.write(cmd.to_string())        
  else:
    pass  
  # app('System Events').keystroke(' ')
