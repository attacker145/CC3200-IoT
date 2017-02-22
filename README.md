# Updated Code for the CC3200 Wi-Fi Project

Project design files, explanations, and other important information at: http://www.cnktechlabs.com/CC3200.html 


This repository contains code that demonstrates using the Exosite cloud platform
with TI's CC3200 WiFi launchpad.

# Using It

The first step is to flash all the needed files to the board. There is a `.ucf`
file in the "flash" folder, this is a config file for TI's uniflash tool. It
will flash the included mcu inage and all the supporting files to the sflash
on the launch pad.

If you haven't programmed an access point profile (your WiFi SSID and key) yet,
press SW2 push button on the development board. It will switch the board into AP mode. Connect to the
'mysimplelink-XXXXXX' AP with a phone or laptop and go to
http://mysimplelink.net and configure your network settings profile just like
in the out of box demo.

Once configured, remove the jumpper and reset the board. It will now connect to
your network and attempt to activate itself on the Exosite platform. That means
that you can now add it to your account on https://ti.exosite.com and it should
then be able to activate itself and begin writing data to the platform.

# Known Issues

* There may be an I2C issue, but as far as I can tell it's a hardware issue, or
  at least at a lower level than I can easily access.
* The "User Button" counting has not been implemented yet.
* Setting the LED on or off has not been implemented yet.
* Bloat; There is a lot of code and html/js files left over from the out of box
  demo that this is based on. This needs to be removed and replaced with a nice
  looking, mobile friendly, single page, the C code that makes the API will
  probably stay as-is.
* Make it go into AP mode if no profiles are saved or can't connect.



================================================================================
Exosite CC3200. Cloud Demo CC3200CloudDemo-master (another cloud example) https://github.com/exosite-garage/CC3200CloudDemo
================================================================================

This project is an embedded software project meant to run on the Texas
Instruments cc3200 launchpad to demonstrate interacting with the Exosite cloud
using the onboard WiFi connectivity.

The project functionality includes:

  * Write Temperature Sensor Value
  * Write Accelerometer Movment
  * Write Push Button Count (Planned)
  * Control LED On/Off from Web Dashboard (Planned)

Tested with:
  * Code Composer Studio 6 On Windows
  * CC3200 Compiler
  * CC3200 Launchpad (Rev 4.2)
  * CC3200 SDK v1.0.1

================================================================================
Developer Getting Started (Build Project & Program)
================================================================================
These directions assume that you have already successfully configured Code
Composer Studio to build example applications from the CC3200 SDK, if not,
follow the instructions in the Quick Start Guide included in the SDK.

1) Clone this repository or download the zip and extract.

2) Open Code Composer Studio v6.0 or greater.

3) Import the project files.
   * Select "Projects -> Import CCS Projects..."
   * Browse for and select the folder you created in step one.
   * Check the box for this project in "Discovered Projects"
   * Click Finish

4) Build The Project

5) You can now flash the device using uniflash or run the debugger, but note
   that you must do a full system flash using uniflash at least once before you
   will be able to run the application in the debugger to install necessary
   supporting files.

6) Open the ucf file using the uniflash tool, select the proper serial port and
   press the "Program" button, it may then ask you to press the reset button.
   Note that the board must be in flash more, do this by placing a jumper on
   the SOP2 header.

7) Open a serial terminal application such as CoolTerm or Putty or termite and connect to
   the debugging serial port.

8) Run the application.
   * Remove the SOP2 jumper.
   
9) Press and hold SW2 switch then reset the board (note: you can switch to AP mode by pressing SW2 button after
   the board is reset as well.)

10) Connect to the "mysimplelink-XXXXXX" Access Point on smartphone, tablet or
    laptop and open the webpage at "mysimplelink.net".

11) Take note of the MAC address of your board from the status page, you'll need
    it in a minute.

12) Click the profiles link to fill your Access Point's SSID, Security type
    and Security Key in Add Profile item, then click Add button

13) Remove the P58 jumper and reset the board. It will now connect to your
    network and begin communicating with the Exosite OnePlatform.

14) Add a matching client to your portal on https://ti.exosite.com.
   * In the Manage menu, click "Devices".
   * Click "+ Add Device"
   * Choose to add a CC3200 type device
   * Give your device a name and enter the device's MAC address. MAC address is on the original web page. 54:4A:16:29:C9:08
   * After clicking through the wizard you will be taken to the dashboard for
     your new CC3200 Launchpad.
   * (Note: If the board has been running for some time it may take up to 10
      minutes to activate. You can force an activation attempt by resetting the
      board again. )

================================================================================
Known Issues
================================================================================

* The "User Button" counting from v1 has not been implemented yet.
* Setting the LED on or off from v1 has not been implemented yet.
* Bloat: There is a lot of code and html/js files left over from the out of box
  demo that this is based on. This needs to be removed and replaced with a nice
  looking, mobile friendly, single page, the C code that makes the API will
  probably stay as-is.

================================================================================
Release Info
================================================================================
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Release 2014-10-11
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Initial Public Release

