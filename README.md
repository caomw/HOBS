CS294-84Project
===============

Repo for CS294-84 Project

Configuration v1.0:

connect digital pin 2 and 3 to XBee RX, TX
connect power and ground to XBee
when XBee receives a radio signal, it'll print to usb serial port
when XBee receives a usb serial port signal, it'll print to radio, plus a '!'
(what it reads from usb serial seems to be weird encoding, that's not an error in code)
