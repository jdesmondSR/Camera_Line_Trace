import cv2
# import numpy as np
import time
import serial

ser = serial.Serial(
    port='/dev/ttyS0',
    baudrate=9600
)

if (ser.isOpen()):
    print("Opened")
else:
    print("Not opened")
    exit()


# ser.write(bytes('dennis',"ascii"))
time.sleep(1)
divisor = 1
width = 640//divisor
height = 480//divisor

cap = cv2.VideoCapture(cv2.CAP_V4L2)
cap.set(3,width)
cap.set(4,height)
toggle = False
count = 0
while(cap.isOpened()):
    ret,image = cap.read()
    image = cv2.flip(image,-1)
    blur = cv2.GaussianBlur(image,(3,3),0)
    hsv = cv2.cvtColor(blur,cv2.COLOR_BGR2HSV)
    low_hsv = ( 96, 157,  71)
    high_hsv = (179, 220, 255)

    #(centerX, centerY) = (image.shape[1] // 2, image.shape[0] // 2)
    #(B,G,R) = image[centerY,centerX]
    # print("Pixel at center - R {}, G {}, B {}".format(R, G, B))
    
    ball = cv2.inRange(hsv, low_hsv, high_hsv)
    contours, hierarchy = cv2.findContours(ball.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    #newImage = cv2.drawContours(image, contours, -1, (255, 0, 0), 3)
    #print(len(contours))
    if len(contours) > 0:
        #sortedContours = sorted(contours, key = cv2.contourArea, reverse = True)
        #biggestEl = sortedContours[0]
        
        biggestEl = max(contours, key=cv2.contourArea)
        '''
        rect = cv2.minAreaRect(biggestEl)
        box = cv2.boxPoints(rect)
        box = np.int0(box)
        cv2.drawContours(image,[box],0,(0,255,0),2)
        '''        
        (x, y), radius = cv2.minEnclosingCircle(biggestEl)
        x = int(x)
        y = int(y)
        radius = int(radius)
        cv2.circle(image, (x, y), radius, (0, 255, 0), 2)
        
        x = x-(width//2)
        y = y-(height//2)
        
        #str = f"{x},{y},{radius}\n"
        str = f"{x}\n"
        #if (count % 3 == 0):
        #print(radius)
        ser.write(bytes(str,"ascii"))
        count = count + 1
    else:
        #print ("-400")
        ser.write(bytes("-400\n","ascii"))

    cv2.imshow("Red Ball", image)
    
    key = cv2.waitKey(1) & 0xFF
    if key == ord("q"):
        break
cap.release()
cv2.destroyAllWindows()