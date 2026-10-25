An Arduino-controlled dart turret that calculates the launch angle needed to hit a target. The operator aims the laser at the target using a joystick, an ultrasonic sensor measures the distance, and the Arduino solves the projectile equation and sets the barrel to the calculated angle. A servo-driven linear actuator feeds darts into two DC-powered flywheels to launch them.

Demo video: [add link here, or write "Video 1 in my MIT SlideRoom portfolio"]

Turret photo

How it works
Aim. The joystick moves the turret: one axis rotates the base with a stepper motor (through a custom 3:1 planetary gearbox), and the other tilts the barrel with a servo. A laser mounted alongside the barrel shows where the turret is pointing.
Measure. Typing target in the Serial Monitor makes the ultrasonic sensor measure the distance to the target.
Calculate geometry. The sensor, laser and barrel are mounted at known offsets from each other. Using the current barrel angle, the code converts the measured distance into the horizontal and vertical displacement of the target relative to the barrel.
Solve the projectile equation. Using SUVAT equations and a measured launch velocity, the code solves for the launch angle. There are two solutions (low and high). If no real solution exists for that target position, the code reports "NO REAL ANGLES" and does not move.
Set the angle. The servo moves to the low-angle solution, corrected for the laser offset.
Fire. Joystick button presses control launching (see Controls).
The maths

With horizontal displacement s_h, vertical displacement s_v, launch velocity v, and g = 9.81 m/s², the launch angle θ satisfies:

A·tan²θ − s_h·tanθ + (A + s_v) = 0,   where A = 4.905·s_h² / v²

The discriminant s_h² − 4A(A + s_v) must be non-negative for a real solution to exist. The two roots give the low and high launch angles.

The launch velocity (16.92 m/s) is the mean of three Tracker video measurements of dart speed. Two further videos gave much lower speeds (about 10 to 11 m/s) and were excluded as outliers. Full workings are in the docs/ folder and in my portfolio.

Controls
Input	Action
Joystick X	Tilt barrel (servo)
Joystick Y	Rotate turret (stepper)
Joystick button, press 1	Flywheels on
Joystick button, press 2	Servo 1 feeds a dart into the flywheels
Joystick button, press 3	Flywheels off
Serial: target	Measure distance, calculate angle, move barrel servo
Serial: a number from 0 to 180	Set barrel servo to that angle

Serial Monitor baud rate: 9600.

A servo angle of 90° means the barrel is horizontal.

Hardware
Arduino [add model]
2× DC motors (flywheels), driven by a TB6612FNG motor driver
Stepper motor, driven by an A4988 driver
2× servos (barrel tilt and dart feed)
HC-SR04 ultrasonic sensor
Laser module
Analogue joystick with push button
3D-printed parts (PLA) from my own CAD designs

The full schematic is in docs/schematic.pdf.

Pin map
Component	Pin
Flywheel motor A (PWM / IN1 / IN2)	5 / 7 / 8
Flywheel motor B (PWM / IN1 / IN2)	6 / 9 / 10
Servo 1 (dart feed)	11
Servo 2 (barrel tilt)	12
Laser	2
Ultrasonic TRIG / ECHO	3 / 4
Joystick X / Y / button	A0 / A1 / A2
Stepper STEP / DIR / EN	A5 / A3 / A4
Constants
Name	Value	Meaning
ultraToBarrel	22.589 mm	Offset between ultrasonic sensor and barrel centre
barrelToLaser	12.0 mm	Offset between barrel centre and laser
v	16.92 m/s	Measured dart launch velocity
Setup
Open turret.ino in the Arduino IDE.
The code uses the built-in Servo library.
Wire the hardware following the pin map above.
Upload, then open the Serial Monitor at 9600 baud.
Results

Tested at horizontal distances of 1 to 4 m with the vertical displacement held at 0.5 m, with five trials per distance.

Distance	Mean vertical offset	Standard deviation
1 m	+1.3 cm	1.20 cm
2 m	−2.5 cm	1.87 cm
3 m	+0.48 cm	2.92 cm
4 m	−3.2 cm	3.52 cm

The mean offset was closest to zero at 3 m. Repeatability decreased as distance increased, which I put down to variation in launch velocity, dart position and mechanical tolerances.

Limitations
The model assumes ideal projectile motion with no air resistance.
It assumes a constant launch velocity of 16.92 m/s, but dart position and flywheel contact cause small variation between launches.
The barrel angle is assumed to equal the commanded servo angle. The servo connection to the barrel has some play, so the real angle may differ.
The turret is aimed manually with the laser, so aiming error also contributes to the variation between trials.
The ultrasonic sensor's effective range is about 4 m, which limits the operating range.
Possible improvements
Include aerodynamic drag and solve the trajectory numerically.
Redesign the launch mechanism for a more consistent initial velocity.
Add feedback on the barrel angle and automate the horizontal aiming.
Test across more distances and heights.
Repository contents
turret.ino        Arduino firmware
docs/             Schematic, photos, handwritten derivations, data
cad/              STL and CAD files for the printed parts
Credits
The linear actuator was modelled using Fusion 360 Tutorial: Gear and Slider Modeling by RELUvance (YouTube), and the planetary gearbox using Fusion 360 Tutorial: Planetary Gear Train Modeling and Animation by RELUvance (YouTube). I used these as a guide to modelling moving parts and deviated from them where needed to suit my own design.
All code, the mathematical model, the mechanical design and the testing are my own work.

Part of my MIT Maker Portfolio. Built by Rodryk Carvalho.
