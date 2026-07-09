# Project Overview
## Problem Formulation
  This project is an extension of a pre-existing product, that being a Logitech Z313 Speaker System with Subwoofer [1]. This speaker system is wired as the output to a Sony TV, one that can be controlled with a wireless controller, but does not respond to volume up or volume down. This is okay because the Logitech speakers do not work with the TV volume. The Logitech speakers control their volume by manually controlling a potentiometer. I would like the ability to control the volume remotely. The TV remote not controlling the TV volume works with us in this case; we can read the IR signals coming from the remote with an IR sensor and adjust the potentiometers to control the volume.
  
  The pre-existing circuit can be seen in Circuit_Board_Scale, and the wires we will be working with can be seen in Circuit_Board_Connectors.
  
<img src="Circuit_Board_Scale.jpeg" alt="Circuit_Board_Scale: a picture of the circuit next to a quarter" width="500"/>
<img src="Circuit_Board_Connectors.jpeg" alt="Circuit_Board_Connectors: a picture of the circuit revealing the pin placements" width="500"/>

The pins go as follows: GND, Lout, Rout, GND, Lin, Rin, OUT, IN, LED. Looking at the back reveals that both grounds are connected to each other, creating a common ground.
- Lout and Rout are the left and right channels post-potentiometer
- Lin and Rin are the left and right channels pre-potentiometer
- OUT and IN seem to be used as a way to tell if the system is on or off. They are connected through the dbdt switch and nothing else.
- LED is the power that goes to the LED; we might be able to tap into this power for the integrated circuits. UNKNOWN VOLTAGE

## Proposed Solution
  In order to read IR signals, we need to include a microcontroller, but in order to choose one, we need to know what else is needed. 2 physical buttons will take the place of the current physical potentiometer, which changes the internal volume variable. And an RGB LED to determine when the system is powered, this can also be used as a debugging tool, changing colors when changing internal variables. 2 digital potentiometers are used to adjust the left and right channel, controlled through I2C. A DBDT switch will be used to control the "power" LED signifier, connect the standby OUT and IN, and introduce power to the rest of the system. The microcontroller in question will need 2 pins for I2C communication (and the ability to do that), 2 pins for manual control (2 buttons), 2 pins for the LED (Blue and Green), and 1 pin for the IR Sensor. In total, we need 7 usable pins, and the ATtiny 404 will fit our case [2]. We will include 2 digital potentiometers that are controlled with I2C 

##  Version 1:
  The first iteration was successful in communicating with the digital potentiometers through I2C. The programming for the experimental stage gave complete control to the RBG LED, the 2 Digital Potentiometers, the ability to read the IR signals from the remote, and the ability to compile the program within the 4kB flash memory limit. This came with the problem of not reading enough datasheets. After further examination of the digital potentiometer datasheet, it does not appreciate negative voltage through the two terminals (which audio covers a range of about [-2.5, 2.5]). This led to some complications and created a design that fundamentally did not work.

##  Version 2:
  The issue was tackled by using a coupling capacitor within a voltage divider to shift the range into a completely positive voltage [0, 5]. The side buttons were removed because they were virtually inaccessible when the casing was completely put on. A different voltage regulator was used that established a 5V rail, which is how the voltage divider was allowed to work.

  <img width="431" height="592" alt="image" src="https://github.com/user-attachments/assets/73f807d5-b101-412d-89a3-a9e3b96e5c28" />

## References
[1] https://www.logitech.com/en-us/shop/p/z313-speaker-system-subwoofer.980-000382 

[2] https://www.digikey.com/en/products/detail/microchip-technology/ATTINY404-SSNR/8594960
