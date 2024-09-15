import time
import serial

ser = serial.Serial(
    port='/dev/ttyS0',
    baudrate=9600
)

if (ser.isOpen()):
    print("Opened")
else:
    print("wtf")


# ser.write(bytes('dennis',"ascii"))
time.sleep(1)
while (True):
    while (ser.in_waiting < 6):
        print("nada")
        time.sleep(1)
    val = ser.read(6)
    print(val.decode("ascii"))
    ser.write(val)
    
    time.sleep(1)