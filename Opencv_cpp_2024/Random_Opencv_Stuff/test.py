import cv2
import numpy as np
cap = cv2.VideoCapture(cv2.CAP_V4L2)


while(cap.isOpened()):
    ret,image = cap.read()
    #(centerX, centerY) = (image.shape[1] // 2, image.shape[0] // 2)
    cv2.imshow("joe", image)
    #print(len(contours))
    
    key = cv2.waitKey(1) & 0xFF
    if key == ord("q"):
        break
cap.release()
cv2.destroyAllWindows()