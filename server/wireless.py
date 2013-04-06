import serial, glob, argparse

parser = argparse.ArgumentParser(description='Pyserial for XBee')
parser.add_argument('--debug', default=False, action='store_true', help='Print More Errors and Launch interactive console on exception or forced exit')
parser.add_argument('--baud', type=int, action='store', default=9600, help='Specify the baud rate')
parser.add_argument('--timeout', type=float, action='store', default=1, help='Timeout parameter for serial connection')

# we know for mac it will show as /dev/usb.tty*, so list all of them and ask user to choose

availables = glob.glob('/dev/tty.usb*')

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
	
while True:
	text = raw_input("text message to send:")
	ser.write(text)
	print ser.readline()
	
