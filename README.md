CS294-84Project
===============

Repo for CS294-84 Project

Configuration v1.1:
connect digital pin 2 and 3 to XBee RX, TX
connect power and ground to XBee
connect led to pin 13
connect IR sensor to pin 8

Configuration v1.0:

connect digital pin 2 and 3 to XBee RX, TX
connect power and ground to XBee
when XBee receives a radio signal, it'll print to usb serial port
when XBee receives a usb serial port signal, it'll print to radio, plus a '!'
(what it reads from usb serial seems to be weird encoding, that's not an error in code)


## updates
For the amplification circuit of IR, we use the third from this [diagram](http://www.blogcdn.com/www.diylife.com/media/2008/01/fet-diagram2.png).
