# ZX Raspberry Keyboard Scanner

## Introduction

Intended for a Raspberry Pi (any with the 40-pin GPIO) This python script uses the python-uinput and wiringpi libraries to interface a ZX Spectrum (original or +) keyboard membrane. Keypresses on the Spectrum keyboard are then injected into the kernel making it act like a regular keyboard. This can then be combined with RetroPie and/or FUSE to create a faithful emulation of the ZX Spectrum with nothing more than a Raspberry PI Zero W.

## Hardware

The ZX Spectrum keyboard matrix connects the the PCB via two ribbon connectors consisting of 5 data line (KB1) and 8 address lines (KB2). These need to be connected to the Raspberry Pi's GPIO.

Additionally a momentary push-to-make switch allows the switching of keyboard modes and resetting of the emulator.

Using two small pieces of stripboard, solder in two molex connectors. For the larger KB2 each address line requires a diode to prevent short circuits with the black band away from the connector. See photos on <http://mrpjevans.com> for more details. Solder a ribbon cable from each connector to the GPIO as follows:

(Broadcom GPIO Number) - (Connector / Line)

17 - KB1 / 1<br/>
27 - KB1 / 2<br/>
22 - KB1 / 3<br/>
18 - KB1 / 4<br/>
23 - KB1 / 5<br/>
<br/>
5 - KB2 / 1<br/>
6 - KB2 / 2<br/>
13 - KB2 / 3<br/>
19 - KB2 / 4<br/>
26 - KB2 / 5<br/>
16 - KB2 / 6<br/>
20 - KB2 / 7<br/>
21 - KB2 / 8<br/>
<br/>
12 - Switch<br/>
GND - Switch


## Software
These instructions are intended to be used on a Raspberry Pi (40-pin GPIO) with [RetroPie](https://retropie.org.uk/) v4.3 installed.

## Installation

Before using the script, several dependancies must be put in place.

### Install Python & Libraries

From the command line:

````
$ sudo apt-get install libudev-dev python-dev python-pip
````

Now install the wiringpi library for Python. We're using this in preference to the GPIO library as it's much faster.

````
sudo pip install wiringpi
````

### Install uinput libraries

Now download the uinput library from <https://github.com/tuomasjjrasanen/libsuinput>. This is the source and we're going to compile it ourselves. Unzip it into your home directory and change directory to its root.

Note: Instructions are for Raspian Jessie only. If on Wheezy or Stretch, edit the Makefile and change all instances of aclocal-1.14 to aclocal-1.15 before continuing.

````
$ ./configure
$ make
$ sudo make install
````

### Install python-uinput

Download the python-uinput package from <https://github.com/tuomasjjrasanen/python-uinput>

Again unzip in your home directory and change directory to its root.

````
$ sudo python setup.py build
$ sudo python setup.py install
````

If you're not using RetroPie as your base OS, you need to load the uinput kernel module. Add the following to /etc/modules-load.d/modules.conf

````
uinput
````

To load right away:

````
$ sudo modprobe uinput
````

(RetroPie users already have this done for them)

### Test

You can place this script anywhere. Mine is in my /home/pi/ directory. To test, carefully connect your ZX Spectrum membrane to the molex connectors and run the following from the project directory (ideally from an ssh session on another computer):

````
$ sudo python zxscanner.py
````

Each keypress should result in a letter on the screen. The SSH session will show debug output. Check the switch too.

### Running The Scanner In The Background

Complete these steps to always have the keyboard scanner running. Create a file called 'startzxscanner' as follows:

$ sudo nano /usr/local/bin/startzxscanner
 
Cut and paste the following:

````
#!/bin/sh
/usr/bin/python /home/pi/zxscanner.py
````

Now make that file an executable

````
$ sudo chmod +x /usr/local/bin/startzxscanner
````

Create this file:

```` 
$ sudo nano /usr/lib/systemd/zxscanner.service
````

Cut and paste the following:

```` 
[Unit]
Description=ZXScanner
 
[Service]
ExecStart=/usr/local/bin/startzxscanner
Restart=always
 
[Install]
WantedBy=multi-user.target
Alias=zxscanner.service
````

Enable it:

```` 
$ sudo systemctl enable /usr/lib/systemd/zxscanner.service
$ sudo systemctl start zxscanner.service
$ sudo systemctl daemon-reload
````
 
Now your keyboard scanner should be working all the time.

## Usage
When used with RetroPie and FUSE (note: not lr-fuse), a 3-second press on the button will cleanly close down FUSE (although you'll have to configure it not to prompt for configuration). A tap will switch the keyboard so keys 1-4 become F keys (so to get to FUSE's menu) and 5 6 7 8 act as cursor keys.

## Thanks
To Tuomas Räsänen for his python-uninput modules and Jools and his amazing RetroPie project.