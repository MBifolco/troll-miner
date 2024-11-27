
# Troll Miner
We Mine Trolls
```

          .      .
         /(.-""-.)\
     |\  \/      \/  /|
     | \ / =.  .= \ / |
     \( \   o\/o   / )/
      \_, '-/  \-' ,_/
        /   \__/   \
     __/\\\__|__|__/\\___
    /`   \     /   '    `\
   |       '----'          |
   |                       |
    \        ______        /
     '._    /      \    _.'
        `'-._____.-'    
           /   |   \
          |    |    \
           \   |   /
            `--|--`
```

# Thanks & Credit To
[Kano](https://github.com/kanoi), [Skot](https://github.com/skot) and othes for their work on [cgminer](https://github.com/kanoi/cgminer) and [esp-miner](https://github.com/skot/ESP-Miner) from which we borrow very very heavily.

# Why Troll Miner
Troll miner is a project to build esp32 firmware that allows for transparent exploration of how to manage asics as efficiently as possible specifically using the esp32 chip.

# Getting Started
You should be familiar with compiling and flashing esp32 firmware.

First you need to know some key information:
- Microcontroller model (esp32s3 suggested)
- Microcontroller flash memory size

## 1. Setup Firmware Project
 
 - Clone this repo locally
 - Follow steps 1 - 4 from here: https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32/get-started/linux-macos-setup.html
 - If you don't know your chip's flash size look up the specs or try getting info on it using esptool (see below in ESP Device Help).

## 2. Configure Firmware
From inside this project's directory:
- Run `idf.py set-target esp32s3` (replace esp32s3 with your board if necessary - get the list of options by running `idf.py --list-targets`)
- Run `idf.py menuconfig`
  - Choose `Wifi Configuration` from the menu
    - Set your ssid
    - Set you pw
    - Go back to main menu
  - Choose `Pool Configuration` from the menu
    - Set your primary pool address (e.g.  stratum+tcp://nya.kano.is)
    - Set your primary pool port (e.g.  3333)
    - Set your backup pool address (e.g.  stratum+tcp://nya.kano.is)
    - Set your backup pool port (e.g.  80)
  - Hit 's' to save
  - Leave menuconfig
 
## 3. Build Firmware
From inside this project's directory:
 - If you've attempted to build this firmware before run `idf.py fullclean` (or any time you change the configs and in generalas a first troubshooting effort)
 - Build the firmware run `idf.py build`
 - Flash the firmware run `idf.py flash`
 - Connect to your board to see how you did `idf.py monitor`
 - In the future you can run `idf.py build flash monitor`

# ESP Device Help
Depending on your familiarity with esp devices you might have trouble connecting with it and/or be unaware of the specifications you need to know.  Here is how to connect and get some data.

 1. Plug in your device
 2. Find the port of your device run `ls /dev/tty*` and look for something like `/dev/ttyUSB0` or `/dev/ttyACM0` (if you're not sure - unplug > run list > plug back in > run list > find diff)
 3. See if you can detect the board with esptool by running `esptool.py chip_id`
 4. If you get an error about permisions try adding yourself to the dialout group `sudo usermod -aG dialout $USER` and/or changing the permissions of the drive temporarily `sudo chmod 666 /dev/ttyACM0` (replace ttyAMCO with the correct port on your system)
 5. Try #3 again