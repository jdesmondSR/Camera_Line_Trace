import cv2
import numpy as np

divisor = 1
width = 640//divisor
height = 480//divisor

cap = cv2.VideoCapture(cv2.CAP_V4L2)
cap.set(3,width)
cap.set(4,height)
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
        sortedContours = sorted(contours, key = cv2.contourArea, reverse = True)
        biggestEl = sortedContours[0]
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
        print (x-(width//2),y-(height//2),radius)
    #cv2.imshow("gray", imggray)
    cv2.imshow("Red Ball", image)
    #print(len(contours))
    
    key = cv2.waitKey(1) & 0xFF
    if key == ord("q"):
        break
cap.release()
cv2.destroyAllWindows()