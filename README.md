# 4B25 Final Project - Human Fall Detector
Name: Jinze Sha  
College: King's College  
CRS ID: js2294 
## Summary
According to the [World Health Organization](https://www.who.int/news-room/fact-sheets/detail/falls): Falls are the second leading cause of accidental or unintentional injury deaths worldwide. People may not be able to call for help after falling. So a device was developed to detect human fall and raise a warning after a fall is detected, which can be cleared by pressing the button, after which the device enters back into fall detecting mode.  

## Repository layout
This project is based on the [Warp firmware](https://github.com/physical-computation/Warp-hardware), the modifications on code can be viewed at the bottom of 'Files changed' section in the [diff link](https://github.com/physical-computation/Warp-firmware/compare/master...Fibonacci-Soup:master), apologies for including compiled files in git, which is because I've been using git to transfer files from the remote server in the lab to my personal computer. The relevant changes for the final project are in the following list of files:

	1. The core of the implementation:
		src/boot/ksdk1.1.0/warp-kl03-ksdk1.1-boot.c
		
	2. Driver for the accelerometer sensor (freefall detection function):
		src/boot/ksdk1.1.0/devMMA8451Q.c
		src/boot/ksdk1.1.0/devMMA8451Q.h
		
	3. Driver for OLED display (status and warning displaying function):
		src/boot/ksdk1.1.0/devSSD1331.c
		src/boot/ksdk1.1.0/devSSD1331.h
		
	4. Miscellaneous changes (compilation, pin configuration etc.)
		src/boot/ksdk1.1.0/CMakeLists.txt
		src/boot/ksdk1.1.0/gpio_pins.h
		tools/scripts/jlink.commands
		js2294-coursework-5-original-resolution.pdf (full resolution report uploaded because Moodle only allows files under 5MB)





# Warp firmware information

## Baseline firmware for the [Warp](https://github.com/physical-computation/Warp-hardware) family of hardware platforms
This is the firmware for the [Warp hardware](https://github.com/physical-computation/Warp-hardware) and its publicly available and unpublished derivatives. This firmware also runs on the Freescale/NXP FRDM KL03 evaluation board which we use for teaching at the University of Cambridge. When running on platforms other than Warp, only the sensors available in the corresponding hardware platform are accessible.

**Prerequisites:** You need an arm cross-compiler such as `arm-none-eabi-gcc` installed as well as a working `cmake` (installed, e.g., via `apt-get` on Linux or via [MacPorts](https://www.macports.org) on macOS). You will also need an installed copy of the SEGGER [JLink commander](https://www.segger.com/downloads/jlink/), `JlinkExe`, which is available for Linux, macOS, and Windows (here are direct links for downloading it for [macOS](https://www.segger.com/downloads/jlink/JLink_MacOSX.pkg), and [Linux tgz 64-bit](https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.tgz)).

### 1.  Compiling the Warp firmware
First, make sure the environment variable `ARMGCC_DIR` is set correctly (you can check whether this is set correctly, e.g., via `echo $ARMGCC_DIR`; if this is unfamiliar, see [here](http://homepages.uc.edu/~thomam/Intro_Unix_Text/Env_Vars.html) or [here](https://www2.cs.duke.edu/csl/docs/csh.html)). If your `arm-none-eabi-gcc` is in `/usr/local/bin/arm-none-eabi-gcc`, then you want to set  `ARMGCC_DIR` to `/usr/local`. If your shell is `tcsh`:

	setenv ARMGCC_DIR <full path to the directory containing bin/arm-none-eabi-gcc>

Alternatively, if your shell is `bash`

	export ARMGCC_DIR=<full path to the directory containing bin/arm-none-eabi-gcc>

(You can check what your shell is, e.g., via `echo $SHELL`.) Second, edit the jlink command file, `tools/scripts/jlink.commands` to include the correct path.

Third, you should be able to build the Warp firmware by

	cd build/ksdk1.1/
	./build.sh

This copies the files from `Warp/src/boot/ksdk1.1.0/` into the KSDK tree, builds, and converts the binary to SREC. See 	`Warp/src/boot/ksdk1.1.0/README.md` for more. _When editing source, edit the files in `Warp/src/boot/ksdk1.1.0/`, not the files in the build location, since the latter are overwritten during each build._

Fourth, you will need two terminal windows. In one shell window, run the firmware downloader:

	JLinkExe -device MKL03Z32XXX4 -if SWD -speed 100000 -CommanderScript ../../tools/scripts/jlink.commands

In the other shell window, launch the JLink RTT client<sup>&nbsp;<a href="#Notes">See note 1 below</a></sup>:

	JLinkRTTClient

### 2. Using the Warp firmware on the FRDM KL03
The SEGGER firmware allows you to use SEGGER’s JLink software to load your own firmware to the board, even without using their specialized JLink programming cables. You can find the SEGGER firmware at the SEGGER Page for [OpenSDA firmware](https://www.segger.com/products/debug-probes/j-link/models/other-j-links/opensda-sda-v2/).


### 3.  Editing the firmware
The firmware is currently all in `src/boot/ksdk1.1.0/`, in particular, see `src/boot/ksdk1.1.0/warp-kl03-ksdk1.1-boot.c` and the per-sensor drivers in `src/boot/ksdk1.1.0/dev*.[c,h]`.

The firmware builds on the Kinetis SDK. You can find more documentation on the Kinetis SDK in the document [doc/Kinetis SDK v.1.1 API Reference Manual.pdf](https://github.com/physical-computation/Warp-firmware/blob/master/doc/Kinetis%20SDK%20v.1.1%20API%20Reference%20Manual.pdf).

The firmware is designed for the Warp hardware platform, but will also run on the Freeacale FRDM KL03 development board. In that case, the only driver which is relevant is the one for the MMA8451Q. For more details about the structure of the firmware, see [src/boot/ksdk1.1.0/README.md](src/boot/ksdk1.1.0/README.md).


### If you use Warp in your research, please cite it as:
Phillip Stanley-Marbell and Martin Rinard. “A Hardware Platform for Efficient Multi-Modal Sensing with Adaptive Approximation”. ArXiv e-prints (2018). arXiv:1804.09241.

**BibTeX:**
```
@ARTICLE{1804.09241,
  author = {Stanley-Marbell, Phillip and Rinard, Martin},
  title = {A Hardware Platform for Efficient Multi-Modal Sensing with Adaptive Approximation},
  journal = {ArXiv e-prints},
  archivePrefix = {arXiv},
  eprint = {1804.09241},
  year = 2018,
}
```

### Acknowledgements
This research is supported by an Alan Turing Institute award TU/B/000096 under EPSRC grant EP/N510129/1, by Royal Society grant RG170136, and by EPSRC grants EP/P001246/1 and EP/R022534/1.

----
### Notes
<sup>1</sup>&nbsp; On some Unix platforms, the `JLinkRTTClient` has a double echo of characters you type in. You can prevent this by configuring your terminal program to not echo the characters you type. Alternatively, rather than using the `JLinkRTTClient`, you can use a `telnet` program: `telnet localhost 19021`. This avoids the JLink RTT Client's "double echo" behavior but you will then need a carriage return (&crarr;) for your input to be sent to the board. Also see [Python SEGGER RTT library from Square, Inc.](https://github.com/square/pylink/blob/master/examples/rtt.py) (thanks to [Thomas Garry](https://github.com/tidge27) for the pointer).
