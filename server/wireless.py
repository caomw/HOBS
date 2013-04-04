import serial, glob

# we know for mac it will show as /dev/usb.tty*, so list all of them and ask user to choose

availables = glob.glob('/dev/tty.usb*')

print "All available ports:" 

index = 1
for port in availables:
	print '  ', index, port
	index += 1

try:
	num = int(raw_input('Enter the number:'))
	if num > 0 and num < len(availables):
		selected = availables[num-1]
except:
	print 'invalid input!'

try:
	baud = int(raw_input('Baud rate (9600):'))
except:
	baud = 9600


ser = serial.Serial(selected, baud)
while True:
	print ser.read()



	
