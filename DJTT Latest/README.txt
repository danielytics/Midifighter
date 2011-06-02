
Contents:

1. What's new ...
2. Features of the Midifighter Firmware
3. Entering and Using Menu Mode.
4. Using the Midifighter Expansion ports.
5. How To Flash the Midifighter with new Firmware
6. How To set up the Development Environment

What's New in Version 1.4
=========================

   * The delay before checking to see if menu mode or bootloader has been
     requested as been increased making this process more reliable.

   * The mapping for the analog expansion inputs has been updated, introducing
     a 3 bit dead zone at either end.

     // New mapping style:
                //
                //   0  3             64           124 127
                //   |--|-------------|-------------|--|   - full range
                //
                //      |0=======================127|      - CC A
                //                    |0=========105|      - CC B
                //
                //   |__|on____________________________|   - note A
                //   |off___________________________|on|   - note B
                //      3                          124


   * Cool stuff - Super Combos are built in. Ive allready said too much!

-------------------------------------------------------------------------------

What's New in Version 1.3
=========================

   * The concept of a "MIDI base note" has been removed. All MIDI layouts
     now start at note 36, and additionally all expansion port Notes and CCs
     are unchanged across all layouts. This will make future mappings easier
     to write and install for all users.

   * Fourbanks Mode comes in Internal and External modes, allowing you to
     select banks using the top four keys as the bank switches ("Fourbanks
     Internal") or to use the four Digital Expansion Inputs as the bank
     switches ("Fourbanks External").

   * BONUS FEATURE: Newer PCBs have a additional "Ground Effects" LED that
     flashes in time with the MIDI clock.


-------------------------------------------------------------------------------

What's New in Version 1.3
=========================

   * The concept of a "MIDI base note" has been removed. All MIDI layouts
     now start at note 36, and additionally all expansion port Notes and CCs
     are unchanged across all layouts. This will make future mappings easier
     to write and install for all users.

   * Fourbanks Mode comes in Internal and External modes, allowing you to
     select banks using the top four keys as the bank switches ("Fourbanks
     Internal") or to use the four Digital Expansion Inputs as the bank
     switches ("Fourbanks External").

   * BONUS FEATURE: Newer PCBs have a additional "Ground Effects" LED that
     flashes in time with the MIDI clock.

-------------------------------------------------------------------------------


1. Features of the Midifighter Firmware v1.3
============================================

The Midifighter is a Class Compliant USB device requiring no additional
device drivers to deliver MIDI over USB on Mac, PC and Linux.

It provides high performance key scanning, with keys read at 1000Hz giving 1
millisecond response to keydowns and 10ms response to keyups (due to the
10-entry debounce buffer). We send the MIDI events to the host as fast as we
capture them.

The Midifighter provides an internal Menu system available at boot-time,
that allows you to change device settings. These settings are written to
persistent storage and reused at every subsequent reboot. Hold down the
top-left key (nearest the USB socket) while powering up to enter Menu Mode.

BONUS FEATURE: The current revision of the Midifigher Circuit Board has
space for an additional LED in the center of the board. This LED will flash
in time with MIDI Clock events if they are sent to the Midifigher.


MIDI Implementation
-------------------

The MIDI implementation has three modes of operation: "Fourbanks Off",
"Fourbanks Internal" and "Fourbanks External" modes.


Fourbanks Off
-------------

In Normal operation each of the 16 keys generates a different MIDI
NoteOn/NoteOff pair when pressed and released. Additionally, the LED for
each key can be activated by sending a NoteOn to the device on the same MIDI
channel and note that the key generates. The default MIDI channel is channel
3, and each NoteOn and NoteOff produces the same velocity (default 127), a
setting which can also be altered using the Menu system.

The keys generate notes from C2 to D#3 (assuming the standard MIDI
interpretation that "Middle C" = C4 is note 60), and this is designed to line
up the keypad with the default window of an Ableton Live 8 drum map:

    C3  C#3 D3  D#3
    G#2 A2  A#2 B2
    E2  F2  F#2 G2
    C2  C#2 D2  D#2

or as MIDI note numbers:

    48  49  50  51
    44  45  46  47
    40  41  42  43
    36  37  38  39

Adding in the expansion port notes (described later), the Fourbanks Off MIDI
map looks like this:

    .  .  .  .   <- 124 .. 127
    .  .  .  .   <- 120 .. 123
    .  .  .  .   <- 116 .. 119
    .  .  .  .   <- 108 .. 115
   A3 A3 A4 A4   <- 104 .. 107 = analog 3,4
   A1 A1 A2 A2   <- 100 .. 103 = analog 1,2 
    .  .  .  .   <- 96 .. 99
    .  .  .  .   <- 92 .. 95
    .  .  .  .   <- 88 .. 91
    .  .  .  .   <- 84 .. 87
    .  .  .  .   <- 80 .. 83
    .  .  .  .   <- 76 .. 79
    .  .  .  .   <- 72 .. 75
    .  .  .  .   <- 68 .. 71
    .  .  .  .   <- 64 .. 67
    .  .  .  .   <- 60 .. 63
    .  .  .  .   <- 56 .. 59
    .  .  .  .   <- 52 .. 55
    1  1  1  1   <- 48 .. 51 = bank 1
    1  1  1  1   <- 44 .. 47 = bank 1
    1  1  1  1   <- 40 .. 43 = bank 1
    1  1  1  1   <- 36 .. 39 = bank 1
    .  .  .  .   <- 32 .. 35
    .  .  .  .   <- 28 .. 31
    .  .  .  .   <- 24 .. 27
    .  .  .  .   <- 20 .. 23
    .  .  .  .   <- 16 .. 19
    .  .  .  .   <- 12 .. 15
    .  .  .  .   <- 08 .. 11
   D1 D2 D3 D4   <- 04 .. 07 = Digital Input
    .  .  .  .   <- 00 .. 03

    
Fourbanks Internal
------------------

Fourbanks Internal mode sets aside the top row of four keys as Bank Select
keys, selecting between four different banks of keys where each bank is
accessed using the lower 12 keys. The currently selected bank is highlighted
with an LED on the top row. When a Bank Select key is pressed, the LED on
that key is illuminated and the bottom three rows of buttons switch their
Note values and LED state to reflect the newly selected bank.

The bank is selected by reacting to the most recently pressed key, and if
two keys are pressed simultaneously the leftmost key is selected. When a
bank key is pressed it emits a "Bank Select" NoteOn and the unselected bank
(even if it is currently active) sends a NoteOff. Bank select notes are in
the range 0 to 3. After the bank is selected, only the currently active bank
can emit NoteOn/NoteOff events as a normal key. This ensures that only one
Bank is selected at any time.

Key LEDs can be controlled by sending the Midifighter a NoteOn event with
the same channel and Note that the key generates. Note states are tracked
across all four banks and LEDs will correctly reflect the NoteOn/NoteOff
state of the keys as you switch banks.
3
Adding in the Bank Select notes and expansion port notes (described later),
the Fourbanks Internal MIDI map looks like this:

    .  .  .  .   <- 124 .. 127
    .  .  .  .   <- 120 .. 123
    .  .  .  .   <- 116 .. 119
    .  .  .  .   <- 108 .. 115
   A2 A2 A3 A3   <- 104 .. 107 = Smart Fader 2,3
   A0 A0 A1 A1   <- 100 .. 103 = Smart Fader 0,1
    .  .  .  .   <- 96 .. 99
    .  .  .  .   <- 92 .. 95
    .  .  .  .   <- 88 .. 91
    .  .  .  .   <- 84 .. 87
    4  4  4  4   <- 80 .. 83 = bank 4 
    4  4  4  4   <- 76 .. 79 = bank 4 
    4  4  4  4   <- 72 .. 75 = bank 4 
    3  3  3  3   <- 68 .. 71 = bank 3 
    3  3  3  3   <- 64 .. 67 = bank 3 
    3  3  3  3   <- 60 .. 63 = bank 3 
    2  2  2  2   <- 56 .. 59 = bank 2 
    2  2  2  2   <- 52 .. 55 = bank 2 
    2  2  2  2   <- 48 .. 51 = bank 2 
    1  1  1  1   <- 44 .. 47 = bank 1 
    1  1  1  1   <- 40 .. 43 = bank 1 
    1  1  1  1   <- 36 .. 39 = bank 1 
    .  .  .  .   <- 32 .. 35
    .  .  .  .   <- 28 .. 31
    .  .  .  .   <- 24 .. 27
    .  .  .  .   <- 20 .. 23
    .  .  .  .   <- 16 .. 19
    .  .  .  .   <- 12 .. 15
    .  .  .  .   <- 08 .. 11
   D1 D2 D3 D4   <- 04 .. 07 = Digital Input
   S1 S2 S3 S4   <- 00 .. 03 = Bank Select


Fourbanks External
------------------

Fourbanks External mode uses keys attached to the four Digital expansion
ports as Bank Select switches, allowing four banks of 16 notes. When a bank
is selected the entire grid of 16 keys switch their note values and LED
state to reflect the newly selected bank.

As in Fourbanks Internal, the bank is selected by the most recently pressed
Bank Select key, with deselected banks generating a NoteOff even if the key
is currently activated.

Key LEDs can be controlled by sending the Midifighter a NoteOn event with
the same channel and Note that the key generates. Note states are tracked
across all four banks and LEDs will correctly reflect the NoteOn/NoteOff
state of the keys as you switch banks.

Adding in the expansion port notes (described later), the Fourbanks External
MIDI map looks like this:

    .  .  .  .   <- 124 .. 127                    
    .  .  .  .   <- 120 .. 123                    
    .  .  .  .   <- 116 .. 119                    
    .  .  .  .   <- 108 .. 115                    
   A2 A2 A3 A3   <- 104 .. 107 = Smart Fader 2,3  
   A0 A0 A1 A1   <- 100 .. 103 = Smart Fader 0,1  
    4  4  4  4   <- 96 .. 99 = bank 4                     
    4  4  4  4   <- 92 .. 95 = bank 4                     
    4  4  4  4   <- 88 .. 91 = bank 4                     
    4  4  4  4   <- 84 .. 87 = bank 4                     
    3  3  3  3   <- 80 .. 83 = bank 3             
    3  3  3  3   <- 76 .. 79 = bank 3             
    3  3  3  3   <- 72 .. 75 = bank 3             
    3  3  3  3   <- 68 .. 71 = bank 3             
    2  2  2  2   <- 64 .. 67 = bank 2             
    2  2  2  2   <- 60 .. 63 = bank 2            
    2  2  2  2   <- 56 .. 59 = bank 2             
    2  2  2  2   <- 52 .. 55 = bank 2             
    1  1  1  1   <- 48 .. 51 = bank 1
    1  1  1  1   <- 44 .. 47 = bank 1             
    1  1  1  1   <- 40 .. 43 = bank 1             
    1  1  1  1   <- 36 .. 39 = bank 1             
    .  .  .  .   <- 32 .. 35                      
    .  .  .  .   <- 28 .. 31                      
    .  .  .  .   <- 24 .. 27                      
    .  .  .  .   <- 20 .. 23                      
    .  .  .  .   <- 16 .. 19                      
    .  .  .  .   <- 12 .. 15                      
    .  .  .  .   <- 08 .. 11                      
   D1 D2 D3 D4   <- 04 .. 07 = Digital Input    
   S1 S2 S3 S4   <- 00 .. 03 = Bank Select      

 
-------------------------------------------------------------------------------


3. Entering and Using Menu Mode
===============================

To enter Menu Mode, hold down the top-left key while resetting the
Midifighter. You can power on the Midifigher by either by plugging in the
USB cable, or by pressing an optional 6mm tactile "reset" switch that can be
soldered to the top left of the motherboard.

Once in Menu Mode, the LED will display 7 option keys and a flashing "exit"
key:

    * * * *   <- Menu items
    * * * .
    . . . .
    . . . #   <- Flashing exit key

To leave any Menu page or the Menu system itself, just hit the flashing Exit key.
    
Any changes made to the options are only written to the Midifighter if you
exit the Menu Mode through the top level "exit" key - this allows you to
play with the options and be sure your settings are correct before
committing the changes, without having to remember previous values.
Only when you are sure the Midifighter is set up to your specifications
should you use the top level "exit" button to commit your changes.

The seven menu items are:

    1. MIDI channel
    2. MIDI velocity
    3. empty
    4. Enable Keypress LED
    5. Select Fourbanks mode
    6. Select Expansion port Digital inputs
    7. Select Expansion Port Analog inputs

    
1. MIDI Channel
---------------

    # . . .   <- Flashing exit key
    . . . .
    * * * *   <- MIDI Channel in binary
    o . . o   <- increment/decrement keys
    
Each note generated by the Midifighter is generated on a single MIDI
channel. By default this is Channel 3, but the first menu item allows you to
change this to any of the 16 MIDI channels. The 4-bit binary value can be
altered by incrementing and decrementing or you can directly toggle bits in
the value itself.

NOTE: MIDI channels are written down as "1..16" but their binary number is
written "0..15", therefore all leds off will represent the binary value
b0000, which is interpreted as "MIDI Channel 1", b0001 is "MIDI Channel 2",
b0100 is "MIDI Channel 5", etc.


2. MIDI Velocity
----------------

    . # . .   <- Flashing exit key
    . * * *   <- MIDI Channel in binary (high bits)
    * * * *   <- MIDI Channel in binary (low bits)
    o . . o   <- increment/decrement keys
    
Each NoteOn event has an accompanying velocity. As the Midifighter keys are
not velocity sensitive we use a single fixed value for all events. This menu
item allows you to alter that velocity value. The 7-bit binary value can be
altered by incrementing and decrementing or you can directly toggle bits in
the value itself.


3. empty
-----------------

    . . # .   <- Flashing exit key
    . . . .
    . . . .
    . . . .

This menu slot is currently unused. Press the flashing Exit key to leave
this page..


4. Enable Keypress LED
----------------------

    . . . #   <- Flashing exit key
    . . . .
    * * * *   <- All on or all off, click to toggle
    . . . .
    
The LEDs react to MIDI inputs on the same channel and note numbers that they
generate when pressed. This allows advanced users to directly control the
display of information on the 4x4 grid using their software mappings. The
keypad also automatically lights the LED associated with a keypress. This
feature can be disabled if the user requires complete external control over
the 16 LEDs using this menu option.

The four LEDs of the "boolean bar" are either all on or all off. Press the
bar to swap between enabled and disabled.


5. Select Fourbanks Mode
------------------------

    . . . .
    # . . .   <- Flashing exit key
    * * * *   <- Selection bar graph (three states)
    o . . o   <- increment/decrement
    
This menu page allows you to select one of the three bank modes by using the
increment/decrement keys to move through the different options.

   . . . .  = Fourbanks Off
   
   . . * *  = Fourbanks Internal
   
   * * * *  = Fourbanks External

NOTE: If Fourbanks External is selected, the Digital Inputs menu page will
be disabled.


6. Select Expansion Port Digital Inputs
---------------------------------------

    . . . .
    . # . .   <- Flashing exit key
    * * * *   <- one LED for each digital input, click to toggle
    . . . . 
    
Enabling this option causes the Midifighter to read the values of the
external digital input pins and track their state using a debounce
buffer. Each LED can be individually toggled to enable MIDI events to be
read from that digital input.

NOTE: If Fourbanks External mode has been selected, this menu page is
disabled as the digital inputs are being used as Bank Select keys.


7. Enable Expansion Port Analog Inputs
--------------------------------------

    . . . .
    . . # .   <- Flashing exit key
    * * * *   <- one LED for each Analog input, click to toggle
    . . . . 
    
Enabling this option causes the Midifighter to read the analog values of the
external Analog Inputs and generate MIDI events. Each analog input channel
acts as a "Smart Fader", outputting two CC values and two KeyDown events
along the range. See the section "Expansion Ports" for more information on
the Smart Faders.

Each LED can be individually toggled to enable MIDI events to be
read from that digital input.


-------------------------------------------------------------------------------


4. Using the Midifighter Expansion ports
========================================

The Expansion Port has 4 debounced digital inputs that transmit on MIDI
Notes 4 to 7.

These are inputs are disabled by default to prevent random values from
unconnected hardware generating MIDI events but can be turned on using the
Menu system. When connecting switches to the digital inputs no pull-up
resistor is required as this is now handled inside the CPU itself. Simply
connect the switch between the Digital Input Pin and Ground. Activating the
switch will pull the pin low which the CPU will intepret as a keypress.

The Expansion Port also has 4 Analog Input Pins which can be enabled
independently from the Menu system. These analog inputs generate "Smart
Fader" output, a combination of interleaved CC and NoteOn/NoteOff messages
spread out along the fader's length. Four signals are sent in this precise
order:

a) A Control Change (CC) that varies from 0 to 127 along the entire
   range of the fader.
b) A CC that transmots nothing for the first half of the range, then
   transmits 0 to 127 along the second half of the range.
c) A Note that transmits a NoteOff when the fader enters the range 0 to 1, 
   and a NoteOn when it enters the range 1..127.
d) A Note that transmits a NoteOff when the fader enters the 126-127 range
   and a NoteOff when it leaves that range.

This diagram shows the messages transmitted for Analog Input "A1":

    |0------------------------------127| CC16
                      |0------------127| CC20
    |off|on----------------------------| Note 52
    |off----------------------------|on| Note 53
 
This order is chosen so that it is possible to use "MIDI Learn" to assign
the four messages in your favorite Audio Workstation software.

-------------------------------------------------------------------------------


How To Flash the Midifighter with new Firmware
==============================================

The Midifighter uses a "DFU" style bootloader that allows the chip, once put
into "bootloader mode", to receive new programs and settings over the USB
connection. Any tool that supports the DFU command set can be used to flash
a new program to the Midifighter over USB.

To drop your Midifighter into Bootloader Mode, plug in the USB cable while
holding down the four corner keys:

   # . . #
   . . . .
   . . . .
   # . . #

The Midifighter show a checkerboard pattern to indicate that it is ready to
accept DFU commands:

   * . * .
   . * . *
   * . * .
   . * . *

Putting the Midifighter into bootloader mode causes it to disconnect from
USB and reconnect as a different kind of USB device (i.e. not a MIDI class
device), and on some systems this device type requires the installation of a
driver before the reflashing software will recognize the bootloader
device. Setting up this driver is a one-time installation that is only
required of you wish to reflash your Midifighter.

The EEPROMs inside the chip are designed to be programmable for several
thousand cycles, a number of flahes that will take serious amounts of
development time. It's just worth knowing that there is a limit and it may
be a, very very improbable, cause of strange behaviors.


1. Using the MFUpdate application for Mac or PC
-----------------------------------------------

Download the application from DJTechTools.com at:

Mac:

   http://www.djtechtools.com/software/mac/MFUpdate.dmg

   NOTE: Download the .dmg file, open it then *drag the application to your
   desktop* before running it. This last step is important because the
   application downloads the firmware from our servers and executing the app
   from the .dmg container will fail the download.
   
PC:

   http://www.djtechtools.com/software/pc/MFUpdate.msi

   Download the .msi file, right click on it and select "Install" to run the
   Windows installer. After installation, an application shortcut to
   MFUpdate.exe will be added to the desktop. The first time you drop your
   Midifighter into Bootloader mode, Windows may ask to install a
   driver. Suitable drivers can found in the install directory:

      Program Files/DJTechTools/MidifighterUpdate/BootloaderDrivers

First make sure you have a working Internet connection then execute the
MFUpdate application, press "Update Midifighter" and follow the on-screen
instructions for how to get your Midifighter into Bootloader Mode.
Updating will take only a few seconds.



2. PC using Atmel Flip
----------------------

Using the program "Flip" from Atmel.

    http://atmel.com/dyn/products/tools_card_mcu.asp?tool_id=3886

At the end of installation, you *must* select the option to read the README file:

    file:///C:/Program%20Files/Atmel/Flip%203.4.1/info/Readme.html

as it contains instructions for installing the USB driver for the DFU
device. This driver allows connection to the DFU bootloader so that a
program can be uploaded. It's a normal driver install involving:

   - Go to the device manager.
   - Find the AT90USB162 device in the "Jungo" section.
   - Right-click and select "Update Driver...".
   - Select "Install from a specific location".
   - Select "Include this location in the search" and specify the driver
     location, which by default would be "C:/Program Files/Atmel/Flip 3.4.1/usb"
   - Select "Finish" to finally install the driver.
   - The device should appear in the "LibUSB-Win32 Devices" section.

Once installed, Flip is simple to use.

   1. Select the chip that is to be programmed. Use the "chip" icon,the menu
      item "Device/Select..." or press Ctrl-S. Select the option:
      
          AT90USB162

      This will tell the program what start address and size to use for
      program uploads.

   2. With the Midifighter connected and in Bootloader Mode, connect to the
      device by pressing the "usb cable" icon and selecting "USB", or by
      pressing Ctrl-U. Then select "Open" to connect to the Midifighter.

      (TIP: If you are going to be doing software development on the
       Midifighter, and uploading fresh firmware often, it's useful to
       select the option:
   
            "Settings/Preferences/Connecting-Closing/Auto-Connect"

       This will save you from having to hit Ctrl-U to connect over USB
       before each firmware upload.)

   3. Load a ".hex" firmware file using the "File/Load Hex File..." option,
      or by hitting Ctrl-L, and navigating to the hex file and hitting "OK".

   4. Select the programming steps "Erase", "Program" and "Verify". This
      will cause the old program data to be removed, the new data to be
      written and then read back to verify that it was successfully uploaded
      to the device. ("Blank Check" is used to verify that the erase
      operation completed successfully and is mainly used to verify that a
      chip is bad.)

   5. Hit the "Run" button to execute the reprogramming.

   6. Restart the Midifighter using the "Start Application" button, making
      sure the "reset" option is selected, or just unplug and replug the USB
      cable. The Midifighter should reset and act as if it has just been
      plugged in. Depending on the purpose of the new firmware you may
      notice no immediate difference, and if the EEPROM version has been
      incremented you may have lost your persistent settings or you may in
      rare situations encounter the "first boot hardware test". Read the
      firmware release notes for more information on the exact behavior to
      expect.

   
3. PC using the command line.
-----------------------------

If you are developing software for the Midifighter, it's sometimes useful to
be able to automate the reflashing process. Atmel Flip comes with a
command-line application for this called "batchisp.exe", and it can be found in
the same directory as the "flip.exe" binary.

The simplest way to access this program is to add the directory containing
the Flip binaries to the $PATH environment variable. For example, if you
have installed Flip version 3.4.1, the path can be appended using:

   > set PATH = %PATH%;"C:\Program Files\Atmel\Flip 3.4.1\bin"
 
Reprogramming and rebooting a chip takes three calls of the program. To
reflash the Midifighter with a firmware file called "myhexfile.hex" you
would issue these three commands: 

   > batchisp -hardware usb -device at90usb162 -operation memory EEPROM erase
   > batchisp -hardware usb -device at90usb162 \
              -operation memory EEPROM loadbuffer myhexfile.hex program
   > batchisp -hardware usb -device at90usb162 -operation start reset 0



4. Mac using the command line
-----------------------------

(Thanks to SarahEmm)

You will need to download the latest ".hex" firmware from the Midifighter
sourceforge site:

   http://sourceforge.net/projects/midifighter/files/

Then your steps are:

a) Ensure you have the Apple Developer Tools installed (they come on a
   CD/DVD with new macs/new OSes)
b) Install "MacPorts". An installer can be found at

      http://www.macports.org/install.php

c) Open a Terminal window, found under /Applications/Utilities/Terminal, to
   get a command line.
d) At the command line, type

      sudo /opt/local/bin/port install dfu-programmer
      
   to build and install dfu-programmer and dependencies.
e) Hold down the four corner buttons on your Midifighter and plug in the USB
   cable.
f) At the command line, type these three lines:

      dfu-programmer at90usb162 erase
      dfu-programmer at90usb162 flash --debug 1 midifighter.hex
      dfu-programmer at90usb162 reset
      
   where "midifighter.hex" is the name of the new firmware file to be
   programmed. 
g) Unplug and re-plug your Midifighter to start running the new firmware.


-------------------------------------------------------------------------------


How To set up the Development Environment
=========================================


Developing under Windows
------------------------

Create a project directory that will hold the libraries and tools, say,
"Midifighter" in an easy to find location. As many of the tools we will be
using are command-line based, keeping the path to this directory short will
help keep typing to a minimum. I keep my workspace at "C:\midifighter"

The source code for the Midifighter firmware can be downloaded from
Sourceforge at:

   http://sourceforge.net/projects/midifighter/

Unzip the source tree into your project directory and you should find the
source code (.c and .h files) as well as the project Makefile. The source
code is heavily documented and is broken into logical systems with the main
program loop residing in "midifighter.c".
   
The Midifighter zip archive contains the source code only, and so you will
need to download two additional sets of libraries that are required to
produce working binaries. WinAVR is a set of libraries, compilers, debuggers
and command line tools for the AVR series of processors that can be found
at:

   http://sourceforge.net/projects/winavr/

This version of the firmware was built against the "WinAVR 20100110"
version. WinAVR comes with an installer that installs all the tools and adds
the install path to the PATH environment variable. This means that all the
command line tools are instantly available at the command prompt. Part of
the WinAVR package is a set of Gnu Tools compiled for Win32 that provide the
Unix command line tools "make", "grep", "find" etc.
   
Next you will need to download the LUFA libraries for USB on AVR chips. This
library, written by Dean Camera can be found at:

   http://www.fourwalledcubicle.com/LUFA.php

The current codebase is built against the "LUFA 101122" version. Create a
"LUFA101122" subdirectory (without the space)) inside the project directory
and Unzip the LUFA source code into that location. The resulting layout
after installing these tools should be:

    drive (c:)
     |_ Program Files
     | |_ Atmel
     |    |_ Flip 3.4.1
     |      |_ bin
     |_ ...
     |_ WinAVR
     |  |_ bin
     |  |_ utils
     |_ ...
     |_ midifighter
        |_ LUFA101122
        | |_ Bootloaders
        | |_ Demos
        | |_ Documentation
        | |_ LUFA
        | |_ ...
        |_ COPYING.txt
        |_ Makefile
        |_ README.txt
        |_ adc.c
        |_ adc.h
        |_ constants.h
        |_ eeprom.c
        |_ eeprom.h
        |_ expansion.c
        |_ expansion.h
        |_ fourbanks.c
        |_ fourbanks.h
        |_ key.c
        |_ key.h
        |_ led.c
        |_ led.h
        |_ menu.c
        |_ menu.h
        |_ midi.c
        |_ midi.h
        |_ midifighter.c
        |_ selftest.c
        |_ selftest.h
        |_ spi.c
        |_ spi.h
        |_ usb_descriptors.c
        |_ usb_descriptors.h

        
To build the firmware, open a command line (from the Start button you can use "All
Programs/Accessories/Command Prompt", or use the "Run..." option and execute
"cmd"). Change the directory to your project dir and build the project using
the make command:

    > cd C:\midifighter
    > make -s

The "-s" on the make command silences printout of the actual commands
executed (which is optional), but the output should resemble:

    -------- begin --------
    avr-gcc (WinAVR 20100110) 4.3.3
    Copyright (C) 2008 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
     
    Compiling C: midifighter.c
    Compiling C: eeprom.c
    Compiling C: spi.c
    Compiling C: adc.c
    Compiling C: led.c
    Compiling C: key.c
    Compiling C: midi.c
    Compiling C: menu.c
    Compiling C: selftest.c
    Compiling C: expansion.c
    Compiling C: usb_descriptors.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/LowLevel/Device.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/LowLevel/Endpoint.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/LowLevel/Host.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/LowLevel/Pipe.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/LowLevel/USBController.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/LowLevel/USBInterrupt.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/HighLevel/ConfigDescriptor.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/HighLevel/DeviceStandardReq.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/HighLevel/Events.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/HighLevel/EndpointStream.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/HighLevel/HostStandardReq.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/HighLevel/PipeStream.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/HighLevel/USBTask.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Host/HIDParser.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Device/Audio.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Device/CDC.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Device/HID.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Device/MassStorage.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Device/MIDI.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Device/RNDIS.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Host/CDC.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Host/HID.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Host/MassStorage.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Host/MIDI.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Host/Printer.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Host/RNDIS.c
    Compiling C: LUFA101122/LUFA/Drivers/USB/Class/Host/StillImage.c
    Linking: midifighter.elf
    Creating load file for Flash: midifighter.hex
    Creating load file for EEPROM: midifighter.eep
    Creating Extended Listing: midifighter.lss
    Creating Symbol Table: midifighter.sym
     
    Size after:
    AVR Memory Usage
    ----------------
    Device: at90usb162
     
    Program:    7194 bytes (43.9% Full)
    (.text + .data + .bootloader)
     
    Data:        175 bytes (34.2% Full)
    (.data + .bss + .noinit)
     
    -------- end --------


At the end of this you will find a firmware image called "midifighter.hex"
in the project directory. This is the image that you "flash" to the
hardware.


Flashing from the Makefile
--------------------------

The Makefile that comes with the project contains a build target that will
reflash the Midifighter from the command line.

If you have already installed the Atmel Flip tool and added the "bin"
directory from that tool to your PATH environment variable:

    > set PATH= %PATH%;C:\Program Files\Atmel\Flip 3.4.1\bin

we can use the "batchisp" command to do the work. Make sure the Midifighter
is in Bootloader Mode and execute "make flip". This will execute a script
that will upload the firmware and reset the Midifighter to start the program
executing:


    > make flip

    batchisp -hardware usb -device at90usb162 -operation erase f
    Running batchisp 1.2.4 on Fri Oct 29 15:53:12 2010
     
    AT90USB162 - USB - USB/DFU
     
    Device selection....................... PASS 
    Hardware selection..................... PASS 
    Opening port........................... PASS 
    Reading Bootloader version............. PASS	1.0.5
    Erasing................................ PASS 
     
    Summary:  Total 5   Passed 5   Failed 0
    batchisp -hardware usb -device at90usb162 -operation loadbuffer midifighter.hex program
    Running batchisp 1.2.4 on Fri Oct 29 15:53:13 2010
     
    AT90USB162 - USB - USB/DFU
     
    Device selection....................... PASS 
    Hardware selection..................... PASS 
    Opening port........................... PASS 
    Reading Bootloader version............. PASS	1.0.5
    Parsing HEX file....................... PASS	midifighter.hex
    Programming memory..................... PASS	0x00000	0x01c19
     
    Summary:  Total 6   Passed 6   Failed 0
    batchisp -hardware usb -device at90usb162 -operation start reset 0
    Running batchisp 1.2.4 on Fri Oct 29 15:53:14 2010
     
    AT90USB162 - USB - USB/DFU
     
    Device selection....................... PASS 
    Hardware selection..................... PASS 
    Opening port........................... PASS 
    Reading Bootloader version............. PASS	1.0.5
    Starting Application................... PASS	RESET	0
     
    Summary:  Total 5   Passed 5   Failed 0
    
There are similar targets for programming using the "dfu=programmer" tool or
hardware programming debugging using Avrdude and a JTAG programmer. Read the
Makefile for more information.



-------------------------------------------------------------------------------

Robin Green
(C) Copyright DJ Tech Tools 2010-2011
February 4 2011

Michael Mitchell
(C) Copyright DJ Tech Tools 2010-2011
May 12 2011

