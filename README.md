# drawing-circles-interactive-pi

The goal of this assignment was to get openFrameworks on the raspberry pi with access to the GPIO pins. 

To accomplish this, I started out with my DrawingCirlces assignment from week 2 and got that up and running on a fresh Pi. I then attached a distance sensor to the device and started to read those values into the app. Getting this far was surprisingly cumbersome, so I took notes on the necessary steps and shared them with the class: https://ual-cci.slack.com/archives/C02V3NFET60/p1644580317747919

I then made the size, shape and movements of the central organic shape responsive to the user’s distance from the sensor. The result was output on a projector. My takeaway is that cheap ultra-sonic sensors are not ideal for body object tracking, and a Kinect or camera based tracking system might be more ideal.

<img width="637" alt="Full body test — still" src="https://user-images.githubusercontent.com/5685294/159941014-2291bb69-3afd-48e1-a219-8410a7d7b096.png">


https://user-images.githubusercontent.com/5685294/159942190-81ee7aed-e2c3-4927-a5f8-aa42ee29f46c.mp4

