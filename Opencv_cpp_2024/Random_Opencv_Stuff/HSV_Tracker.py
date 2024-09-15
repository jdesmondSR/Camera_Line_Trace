import cv2
import numpy as np


cap = cv2.VideoCapture(cv2.CAP_V4L2)
cap.set(cv2.CAP_PROP_EXPOSURE,-6)
def nothing(x):
    pass
# Creating a window for later use
cv2.namedWindow('result')

# Starting with 100's to prevent error while masking
h,s,v = 100,100,100

# Creating track bar
cv2.createTrackbar('hl', 'result',0,179,nothing)
cv2.createTrackbar('hh', 'result',179,179,nothing)
cv2.createTrackbar('sl', 'result',0,255,nothing)
cv2.createTrackbar('sh', 'result',255,255,nothing)
cv2.createTrackbar('vl', 'result',0,255,nothing)
cv2.createTrackbar('vh', 'result',255,255,nothing)

while(1):

    _, frame = cap.read()
    frame = cv2.flip(frame,-1)
    #converting to HSV
    hsv = cv2.cvtColor(frame,cv2.COLOR_BGR2HSV)

    # get info from track bar and appy to result
    hl = cv2.getTrackbarPos('hl','result')
    hh = cv2.getTrackbarPos('hh','result')
 
    sl = cv2.getTrackbarPos('sl','result')
    sh = cv2.getTrackbarPos('sh','result')
    
    vl = cv2.getTrackbarPos('vl','result')
    vh = cv2.getTrackbarPos('vh','result')

    # Normal masking algorithm
    lower_blue = np.array([hl,sl,vl])
    upper_blue = np.array([hh,sh,vh])

    mask = cv2.inRange(hsv,lower_blue, upper_blue)

    maskedimage = cv2.bitwise_and(frame,frame,mask = mask)
    
    cv2.imshow('result',maskedimage)

    cv2.imshow('result2',mask)

    k = cv2.waitKey(1) & 0xFF
    if k == ord('q'):
        break

print (lower_blue, upper_blue)

cap.release()

cv2.destroyAllWindows()

