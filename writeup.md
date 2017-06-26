# PID Control Project
[image1]: ./speed2.png "Speed function"
[image2]: ./SDCT2P4-Diagrama2.png "Hyperparameter tuning process"

## Control of steer angle and throttle
The steer angle is controlled using a calibrated PID controller which takes as input the CTE and outputs a steer value which is adjusted to the range [-1,1].

Speed is controlled using an on-off controller. A goal speed is set linearly depending on the CTE:
- For CTE=0 the speed goal is 50.
- For 0 < CTE < 2 the speed goes linearly from 50 to 10.
- For CTE > 2 the speed goal stays constant at 10, as means to keep the car going even with big errors.

![alt text][image1]

## Calibrating the PID controller
Following process has been used to tune the parameters:

![alt text][image2]

1. Set slow goal speed for starters. Find some starting stable set of parameters. The general process is to increase the P-gain a bit, then use the D-gain to correct oscillations. After few cycles use the I-gain to correct any bias the car is showing.
2. Select a faster speed function. Apply automated twiddle fine tuning to the parameters. Repeat until fast enough or the optimization doesn't work anymore.

## Twiddle Algorithm in the simulator
To implement the twiddle algorithm automatically, it is necessary to used the *reset* funcion.
The code runs the simulation for a fixed number of steps, after which the simulator is sent a reset message and changes are applied to the parameters depending on the error.
The twiddle algorithm was implemented as explained in class.

## Results
The proposed control manages to drive smoothly around the circuit at speeds between 45 and 50.

During the tests speeds as fast as 70 were used, but they induced excesive oscillation which would have made any passenger sick. The twiddle method was not able to ease these oscillations.

