import Adafruit_BBIO.GPIO as GPIO
import subprocess
import time

GPIO.setup("P9_12", GPIO.IN)
GPIO.setup("P9_14", GPIO.IN)
GPIO.setup("P9_16", GPIO.IN)
GPIO.add_event_detect("P9_12", GPIO.RISING)
GPIO.add_event_detect("P9_14", GPIO.RISING)
GPIO.add_event_detect("P9_16", GPIO.RISING)

A=0
B=0
C=0
D=0

while True:
  if GPIO.event_detected("P9_12"):
    A=1
    B=0
    C=0
  if GPIO.event_detected("P9_14"):
    A=0
    B=1
    C=0
  if GPIO.event_detected("P9_16"):
    A=0
    B=0
    C=1
  if A==1:
    A=0
    subprocess.call("(mplayer -volume 100 c1.wav > /dev/null)&", shell=True)
    D=1
  if B==1:
    B=0
    subprocess.call("(mplayer -volume 100 c2.wav > /dev/null)&", shell=True)
    D=1
  if C==1:
    C=0
    subprocess.call("(mplayer -volume 100 c3.wav > /dev/null)&", shell=True)
    D=1
  if D==1:
    D=0
    time.sleep(3)
    A=0
    B=0
    C=0
