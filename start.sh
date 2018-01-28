#!/bin/bash
# data folder check
if [ ! -a ./emu/data ]; then
    mkdir ./emu/data
fi
# db file check
if [ ! -a ./ui/mz80rpi.db ]; then
    cd ui; python3 makedb.py; cd ..
fi
# launch the emulator
sudo ./emu/mz80rpi &
sudo python3 ./ui/uiserver.py &
