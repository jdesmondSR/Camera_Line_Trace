

import cv2
import numpy as np
from picamera2 import Picamera2
import time

# for timing the key presses
from datetime import datetime

camdict = dict(zip([Picamera2(0)], ["Cam0"]))

for picam2, name in camdict.items():
    print(name)
    config = picam2.create_preview_configuration(lores={"size": (640, 480)})
    picam2.configure(config)
    picam2.start()
    
keepLooping = True


def nothing(x):
    pass

prevTime = datetime.now().second
note = False
while(keepLooping):
    for picam2, name in camdict.items():
        yuv420 = picam2.capture_array("lores")
        frame = cv2.cvtColor(yuv420, cv2.COLOR_YUV420p2RGB)
        
        #dims = (frame.shape[1]//2,frame.shape[0]//2)
        #frame_small = cv2.resize(frame,dims,interpolation=cv2.INTER_AREA)
        #frame_small=cv2.flip(frame_small,-1)
        
        #frame = cv2.flip(frame, -1)
        
        #fullTime = datetime.now()
        currentTime = datetime.now().strftime("%H:%M:%S")
        
        # get key input
        k = cv2.waitKey(1) & 0xFF
        
        if k == ord(' '):
            note = True
            #prevTime = datetime.now()
        
        if note == True:
            if datetime.now().second <= prevTime:
                # not time to play the note
                text_color = (0, 0, 255)
            else:
                #play the note... it is a new second?
                textColor = (0, 255, 0)
                #prevTime = datetime.now()
                note = False
                prevTime = datetime.now().second 
        else:
            textColor = (0, 0, 255)
            
        
        cv2.putText(frame, currentTime, (50,150), cv2.FONT_HERSHEY_SIMPLEX,
                    3, textColor, 4, cv2.LINE_AA)
        cv2.imshow("frame", frame)


        if k == ord('q'):
            keepLooping=False
            break
        
        time.sleep(0.8)


cv2.destroyAllWindows()

for picam2, name in camdict.items():
    picam2.stop()