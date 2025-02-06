# Range Finder

This repository should hold everything related to the range finders made by the ILG for the 6AL lab series. These range finders were made using off the shelf components from adafruit and amazon with a 3D printed case, to solve the issue of tedious data taking in the Energy Conservation lab in 6AL. For information on the components visit the datasheets folder, for information on the housing view the CAD folder, and to view the code view the code folder. 

## Assembly Instructions

#### Materials required

* 1 [Adafruit ItsyBitsy M0](/Documentation//Components/ItsyBitsyM0ExpressGuide.pdf)
* 1 [Adafruit VL53L1X Breakout Board](/Documentation/Components/VL53L1XGuide.pdf)
* 1 [128x32 I2C SSD1306 OLED Display](/Documentation/Components/SSD1306OLEDGuide.pdf)
* 4 [6x6mm Tall Buttons with Colored Caps](/Documentation/Components/6mmx6mm_Button_Tall.jpg)
* 1 [5cm x 7cm Perf Board](/Documentation/Components/5cmx7cm_PerfBoard.jpg)
* 3 M3x6mm Screws
* 3 [M3x6mm Standoffs](/Documentation/Components/M3x6mm_Standoff.jpg)
* 3 [M3x6mm Threaded Inserts](/Documentation/Components/M3x6mm_ThreadedInsert.jpg)
* 2 [M4 Threaded Banana Plugs](/Documentation/Components/M4_BananaPlug.jpg)
* 2 [M4x8mm Threaded Inserts](/Documentation/Components/M4x8mm_ThreadedInsert.jpg)

#### Assembling the Internals

The Perf board must be cut fairly exactly to size for it to fit in the case. 

* First [score and snap](/Documentation/PerfBoardInstructions/ScoredPerfBoard.jpg) the perf board along row 20. 
* Then [sand along the edge](/Documentation/PerfBoardInstructions/SandedPerfBoard.jpg) to make smooth. 
* Lastly to attach you need [M3 clearance holes drilled](/Documentation/PerfBoardInstructions/FinishedPerfBoard.jpg) into the two holes at the bottom and into the hole at J18. 

From here you can solder all the connections together on the perf board. The schematic used is [shown here](/Documentation/CircuitInstructions/RangeFinderSchematic.pdf). The connections are tight so here are pictures of the asiest way to solder all the connections together: [Top of Circuit](/Documentation/CircuitInstructions/CircuitTop.jpg), [Bottom of Circuit](/Documentation/CircuitInstructions/CircuitBottom.jpg), and a [Finished Circuit](/Documentation/CircuitInstructions/CircuitFinished.jpg). The solder joints must be carefully checked, the ItsyBitsy has a tendency to shut down the I2C lines if there is too much noise on SCL or SDA causing the system to no longer work. If the circuit is not working you should first check that all connectons made are correct according to the scematic, and then if it still is failing ensure that all the solder joints are clean and nothing is even close to touching the SCL or SDA lines. 

#### Assembling the Case

The files for the case can be found [here](/CAD/). This folder contains the raw [Fusion 360 file](/CAD/RangeFinderCase.f3d) if it needs to be edited in the future, as well as .stl files for the [lid](/CAD/RangeFinderLid.stl) and the [main body](/CAD/RangeFinderBody.stl), for easier printing. Once the files are printed you need to [install the threaded inserts](Documentation/CaseInstructions/InstallThreadedInserts.jpg) into the holes. The 3 M3x6mm threaded inserts go in the [main cavity of the body](Documentation/CaseInstructions/M3InsertLocation.jpg) and the 2 M4x8mm threaded inserts go in the [bottom channel](Documentation/CaseInstructions/M4InsertLocation.jpg). 

From here you just put everything together. First you [screw in the M4 Banana Plugs](Documentation/CaseInstructions/M4BananaPlugInsert.jpg) to interface with the air track. Next you [screw the main circuit in](Documentation/CaseInstructions/CircuitInCase.jpg) to the case with the M3x6mm Standoffs. The last step is to just [screw on the lid](Documentation/CaseInstructions/FinalCase.jpg) with the M3x6mm screw. 

#### Flashing the Code

The code for this system was made in Arduino IDE and requires the built in libraries to fuction properly, so to flash the code you must use this IDE. The raw .ino file can be found [here](/Code/RangeFinderCode/). To properly compile the code you will also need to install the core for the ItsyBitsy through Arduino IDE, to do this you will need to follow the steps in the ItsyBitsy datasheet. The code also requires a few libraries to work, you can find all of these [here](/Code/Libraries/). You will need copy these over into the libraries sub-folder of wherever the Aduino IDE stores its files, often in "~/documents/Arduino/". From here you should be able to compile and upload the code to the ItsyBitsy when it is plugged into your computer. 
