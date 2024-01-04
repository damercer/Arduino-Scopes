#
# test script to place XIAO SAMD21 boards into boot loader
#
import serial
import serial.tools.list_ports
import os
import sys
import time

ports = serial.tools.list_ports.comports()
for port in ports: # ports:
    # looking for this ID: USB\VID:PID=2886:802F
    if "VID:PID=2886:802F" in port[2]:
        print("Found: ", port[0])
        SerComPort = port[0]
        # Setup connection
print("Trying to open ", SerComPort)

ser = serial.Serial(SerComPort)  # open serial port

ser.baudrate = 1200 # Opening serial port at 1200 Baud for a short while will reset board
time.sleep(0.05)
##ser.write(b'I\n') # send something?
##time.sleep(0.05)
if ser.in_waiting > 0:
    IDstring = str(ser.readline()) # read something
    print(IDstring)
time.sleep(0.05)
ser.close()
# if this worked a USB drive window should open.
time.sleep(1.0) # wait 1 sec
# attempt to copy .uf2 file to USB drive
os.system("copy XIAO_Scope_pwm_awg.uf2 E:")
#
          
