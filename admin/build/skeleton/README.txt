
LiteBSD is variant of 4.4BSD operating system for microcontrollers.
Techically it can run on any processor with paged memory and at least
512 kbytes of RAM. Currently, only Microchip PIC32MZ family is supported.

Files in this package:

    vmunix.hex        - Unix kernel image
    vmunix.dis        - Disassembly of the kernel image, for debugging
    sdcard.img        - Root filesystem image
    pic32prog.exe     - PIC32 programmer utility for Windows
    linux32/pic32prog - PIC32 programmer utility for 32-bit Linux
    linux64/pic32prog - PIC32 programmer utility for 64-bit Linux
    macos/pic32prog   - PIC32 programmer utility for Mac OS X
    pic32prog.txt     - Brief description of pic32prog utility
    README.txt        - This file

The installation of Lite to your board consists of
three steps:

(1) Transfer the Unix kernel to the board
(2) Put the filesystem image on to a SD card
(3) Connect to the console port and start LiteBSD


Transfer the Unix kernel on to the board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For boards, which use a virtual serial port for bootloader
(chipKIT WiFire, Majenko SDZL and Whitecat) you need to know
the exact device name to specify for pic32prog. Connect the board
via microUSB cable to your computer. To enter a bootloader
mode, press the PRG key first, hold it, then press the RESET
key. The LED will now flash and your board is ready to accept
new code. Use pic32prog utility to program the flash code.
The bootloader can appear as a HID device, or as a virtual serial
port (COM port) on your computer. Typically, this virtual port
has a name /dev/ttyUSB0 on Linux, something like COM12 on
Windows, or /dev/tty.usbmodemfa131 on Mac OS X. Some boards
use special names, like /dev/tty.SLAB_USBtoUART on Whitecat.

For boards which don't have bootloader (Microchip MEB-II,
Olimex HMZ144 and EMZ64) you need to use an external programmer like
PICkit2 or one of it's clones (like iCP02). For these boards
you do not need to specify the -d option and device name.

Use proper pic32prog binary for your operating system:

    pic32prog.exe     - for Windows
    linux32/pic32prog - for 32-bit Linux
    linux64/pic32prog - for 64-bit Linux
    macos/pic32prog   - for Mac OS X

Unpack the package and run command (say, for 64-bit Linux):

    linux64/pic32prog -d /dev/ttyUSB0 unix.hex

For Windows, it should be like:

    pic32prog -d COM12 unix.hex

On Mac OS X:

    macosx/pic32prog -d /dev/tty.usbmodemfa131 unix.hex

You should see:
    Programmer for Microchip PIC32 microcontrollers, Version 2.0.151
        Copyright: (C) 2011-2015 Serge Vakulenko
          Adapter: PICkit2 Version 2.32.0
        Processor: MZ2048ECM064
     Flash memory: 2048 kbytes
      Boot memory: 80 kbytes
             Data: 513800 bytes
            Erase: done
    Program flash: ############################################################### done
     Program boot: ### done
     Verify flash: ############################################################### done
      Verify boot: ## done
     Program rate: 3286 bytes per second


Put the filesystem image on to a SD card
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use any USB-to-SD card reader to attach the SD card to your PC.
On Windows, use Win32DiskImager utility. On Linux or Mac OS X,
run:

    sudo dd bs=32k if=sdcard.img of=/dev/XYZ

where XYZ is a name of SD card on your computer (use lsblk or
"diskutil list" to obtain).

Once that is done remove the SD card from card reader and plug
it into SD slot on your board. The LiteBSD system is ready
to run.


Connect to the console port and start LiteBSD
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Connect USB cable to the board and to your computer. The LiteBSD
console will appear as a virtual COM port on your computer. Use
any terminal emulation program (like putty on Windows or
minicom on Linux) to connect to this virtual COM port at baud
rate 115200. Press <Enter> to start LiteBSD. On Login prompt,
enter "root":

    Copyright (c) 1982, 1986, 1989, 1991, 1993
        The Regents of the University of California.  All rights reserved.

    4.4BSD-Lite build 1 compiled 2015-08-27
        sergev@ubuntu-sergev:LiteBSD/sys/compile/Whitecat.pic32
    cpu: PIC32MZ2048ECM064 rev A5, 200 MHz
    oscillator: system PLL div 1:6 mult x50
    cache: 16/4 kbytes
    real mem = 512 kbytes
    avail mem = 344 kbytes
    using 18 buffers containing 73728 bytes of memory
    spi2 at pins sdi=G7/sdo=G8/sck=G6
    en0 at interrupt 153, MAC address d8:80:39:13:d3:67
    en0: <SMSC LAN8720A> at address 1
    uart1 at pins rx=F4/tx=F5, interrupts 112/113/114, console
    uart2 at pins rx=B7/tx=B6, interrupts 145/146/147
    uart3 at pins rx=C13/tx=C14, interrupts 157/158/159
    uart4 at pins rx=D5/tx=D4, interrupts 170/171/172
    sd0 at port spi2, pin cs=G9
    gpio1 at portB, pins ------------iiii
    sd0: type II, size 1982464 kbytes, speed 16 Mbit/sec
    sd0a: partition type b7, sector 2, size 204800 kbytes
    sd0b: partition type b8, sector 409602, size 32768 kbytes
    sd0c: partition type b7, sector 475138, size 102400 kbytes
    WARNING: preposterous clock chip time -- CHECK AND RESET THE DATE!

    starting file system checks.
    /dev/rsd0a: file system is clean; not checking
    starting network
    clearing /tmp
    standard daemons: update inetd.
    Thu Apr 23 18:46:28 PDT 2015


    4.4BSD-Lite (bsd.net) (console)

    login: root
    Last login: Thu Apr 23 18:46:18 on console
    Copyright (c) 1980, 1983, 1986, 1988, 1990, 1991, 1993, 1994
            The Regents of the University of California.   All rights reserved.

    4.4BSD-Lite UNIX #1: Fri Apr 01 00:00:00 PDT 1994

    Welcome to 4.4BSD-Lite!

    erase ^H, kill ^U, intr ^C status ^T
    Don't login as root, use the su command.
    # _
