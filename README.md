# FrankenFrisbeeBot
Project to document the electrical and software used on the Frisbee robot from the FRC 2013 season.
This robot has been retrofitted with mostly commercial products from the RC hobby space. This was done to reduce the cost of the control system components running on this machine, while still maintaining its functional capabilities for use as a demo-robot at outreach events.

## Robot Description
FrankenFrisbeeBot competed in the [2013 season](https://www.thebluealliance.com/team/2168/2013) in the [FIRST Robotics Competition](https://www.firstinspires.org/robotics/frc). It was designed to quickly shoot frisbees into large goals on the playing field. It was our teams first robot to attend world championships!

## Bill of Materials
| Item                      | Description                                     | Qty |Price (ea) | Link |
|---------------------------|-------------------------------------------------|:---:|:---------:|------|
|FlySky FS-I6               | 6 Channel RC Radio (TX) and Receiver (RX)       | 1   | $45       | ebay |
|Self Centering Throttle Mechanism | Only one joystick on the FS-I6 self-centers in the Y direction out of the box. We're driving tank, self centering on both sticks is needed. | 1 | $3 | [AliEx](https://www.aliexpress.com/item/Flysky-i6S-FS-i6S-Self-Centering-Throttle-Mode-Change-to-Mode-1-Mode-2-Part/32790481855.html)|
|G.T.Power Electronic Switch| Converts PWM signal into discrete on/off switch. Used to drive solenoids from RX| 2 | $6 | [AliEx](https://www.aliexpress.com/item/Original-G-T-POWER-Remote-Control-Electronic-Switch-for-RC-Airplane-Helicopter-Car/32698354167.html) |
|Arduino Nano               | Small form factor ATMega328 board.              | 1   | $6        | [Amazon](https://www.amazon.com/gp/product/B06XR46VGD)|
|Nano I/O Expansion Shield  | Breaks out Arduino pins into 1x3 headers for convenience and ease of wiring. | 1 | $7 | [Amazon](https://www.amazon.com/gp/product/B00UBEHJUO) |
|30/40A 12VDC Auto. Relay   | Drives the compressor on/off from nason switch. | 1   | $3        | [Amazon](https://www.amazon.com/gp/product/B072QXDZRD)|
|4V210-08 Solenoids & Manifold | Four 12vdc solenoids & manifold.             | 1   | $43       | [Amazon](https://www.amazon.com/gp/product/B01D9HTQCS)|
|PDB                        | Legacy FRC Power Distribution Board             | 1   | $0 (on hand) | |
|Victor 884 - 888           | Legacy FRC Motor Controllers for 12VDC motors   | 6   | $0 (on hand) | [Vex](https://www.vexrobotics.com/217-2769.html)| 


## Wiring
![](https://github.com/Team2168/FrankenFrisbeeBot/blob/master/wiring/Slide1.PNG "Wiring Schematic")
This wiring layout is pretty simple and largely follows how we would wire the robot in a normal season.
The areas where things are unique is in how the pneumatic compressor is controlled, and the presence of the Arduino & FS-IA6 receiver.

###Pneumatic Compressor Wiring
Normal FRC wiring would have the compressor plugged into a Spike Relay Module or a Pneumatics Control Module, both of which would get their control signals (to turn the compressor on/off) from the roboRIO.
In our case, we're reducing the complexity of the control system (and also saving $$$). In normal FRC plumbing a pressure sensitive switch is included, known as the nason pressure switch.
This switch opens or closes a circuit depending on the pressure sensed. A pressure above ~120psi causes the switch to open circuit. A pressure below ~90psi causes the switch to close.
In our FrankenBot, we will use this switch in a stand alone fassion, to directly turn the compressor on/off. There is no computer in the loop. The only problem is the nason switch is only rated for about 5A of current at 12VDC. The compressor we are using needs about 12A however.
To work around this, we're using the nason switch to open/close an automotive relay. In this configuration the nason switchdrives the coil of the relay, a very small load. The relay, which is rated for 30/40A @ 12VDC then drives the compressor on/off. Cheap and easy.
We still have a pressure relief valve plumbed in to the system. In the event the nason switch or relay fails in a manner that leaves the pump running, the relief valve will ensure the high-side pressure does not exceed ~120psi.

## Plumbing
![](https://github.com/Team2168/FrankenFrisbeeBot/blob/master/wiring/Slide2.PNG "Plumbing Schematic")
![](https://github.com/Team2168/FrankenFrisbeeBot/blob/master/wiring/Slide3.PNG "Valve and Manifold Details")
