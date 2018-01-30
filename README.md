# mz80rpi - MZ-80 series Emulator for Raspberry Pi
This is the MZ-80 series emulator for Raspberry Pi based on [MZ700WIN](http://retropc.net/mz-memories/mz700/) by [Marukun](http://retropc.net/mz-memories/). I made for replace of "PasocomMini MZ-80C"'s system(Raspberry Pi A+) by HAL Laboratory, probably it can run on other Pi available at many shops. This is the console application.

Please refer to [PasocomMini MZ-80Cの改造](http://cwaweb.bai.ne.jp/~ohishi/zakki/pcmini.html).
## Structure
* emu/ … emulator
* ui/ … user interface for web browser
## Need to run
* Install Tornado (Python package)
* Modify config.txt
    * framebuffer_width=320
    * framebuffer_height=240
    * overscan_left=-26
    * overscan_right=-26
    * overscan_top=-16
    * overscan_bottom=-16
    * disable_overscan=1
    * famebuffer_depth=16
    * sdtv_mode=16 (if you need)
* Build main program of emulator
    * cd emu/z80
    * make
    * cd ..
    * make
## How to run
* Start emu/mz80rpi and ui/uiserver.py . You can launch by script start.sh, add the attribute for runnable to start.sh before to run.
* Open ``http://(Pi's address):8080`` by web browser.
* At first, regist monitor ROM and CG ROM to DB. Next, make configuration of environment for MZ-80C with ROMs.
* Apply configuration set.
## Notice
Monitor ROM or BASIC interpreter, Sharp or other company's products, are not included, you need to ready to use converted datas from casset tape or real MZ. If you don't have, it's not perfectly, but can use emulator by compatible data.
* [MZ-NEW MONITOR](http://retropc.net/mz-memories/mz700/kyodaku.html)
* [KM-BASIC for MZ-80K/MZ-700](http://www.vector.co.jp/soft/other/other/se499422.html)

Also CG-ROM is not included too, but you can use the font  made by "look-likes font" tool within MZ700WIN instead of ROM.