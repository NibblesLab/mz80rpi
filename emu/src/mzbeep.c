//----------------------------------------------------------------------------
// File:mzbeep.c
// MZ-80 Emulator mz80rpi for Raspberry Pi
// mz80rpi:Sound Module
// (c) Nibbles Lab./Oh!Ishi 2017
//
// based on https://gist.github.com/rutles/6648946
//----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
//#include "dprintf.h"
#include "mzbeep.h"

//static	BOOL bPlaying;
//static 	BOOL bPulse;
//static	int hw_freq;
//static	int freq;
//static	int pulse_cou;
//static	int pulse_vec;

static uint32_t *pwm_regs;

// offset address
#define OFF_PRT 0x20200000 // port registers
#define OFF_PWM 0x2020C000 // PWM registers
#define OFF_CLK 0x20101000 // clock registers

// Create new mapping of registers
// offset: offset address of registers
// return: pointer to the mapped area, or MAP_FAILED on error
uint32_t *regs_map(off_t offset){
	int fd;
	void *map;
  
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if(fd < 0)
	  return MAP_FAILED;
  
	map = mmap(NULL,
	  getpagesize(),
	  PROT_READ | PROT_WRITE,
	  MAP_SHARED,
	  fd,
	  offset);
  
	close(fd);
	return map;
}

// set PWM space
// *regs: PWM mapping, val: space
void pwm_spc(uint32_t *regs, uint32_t val){
	regs[4] = val;
	//usleep(10);
}

// set PWM mark
// *regs: PWM mapping, value: mark
void pwm_mrk(uint32_t *regs, uint32_t value){
	regs[5] = value;
}

// PWM turn on/off
// *regs: PWM mapping, val: 0 off, 1 on
void pwm_sw(uint32_t *regs, uint32_t val){
	if(val)
	  regs[0] |= 1;
	else
	  regs[0] &= ~1;
}

// set PWM clock devide
// *regs: PWM mapping, val: divide
// NOTE: Frequency = 19200kHz / space / divide
void pwm_clk(uint32_t *regs, uint32_t val){
	uint32_t save;
	uint32_t *regs_clk;
  
	save = regs[0];
	regs[0] = 0;
  
	regs_clk = regs_map(OFF_CLK);
	regs_clk[40] = 0x5A000000 | 1;
	usleep(100);
	while(regs_clk[40] & 0x80)
	  usleep(1);
	regs_clk[41] = 0x5A000000 | (val << 12);
  
	regs_clk[40] = 0x5A000000 | 0x11;
	munmap((void *)regs_clk, getpagesize());
	regs[0] = save;
}

// set GPIO18 as PWM M/S mode
// return: pointer to the mapped area, or MAP_FAILED on error
uint32_t *fnc_pwm(){
	uint32_t *regs;
  
//	regs = regs_map(OFF_PRT);
//	if(regs == MAP_FAILED)
//	  return MAP_FAILED;
//	fnc_wr(regs, 18, 2);//set GPIO18 for PWM
//	munmap((void *)regs, getpagesize());
  
	regs = regs_map(OFF_PWM);
	if(regs == MAP_FAILED)
	  return MAP_FAILED;
	regs[0] |= 0x80;// PWM/MS mode
	return regs;
}

/*
 * コンストラクタ
 */
void mzbeep_init(int freq) {
//	dprintf("mzbeep()\n");

	//hw_freq = freq;
	//bPlaying = false;
	//bPulse = false;
	//pulse_cou = 0;
	//pulse_vec = 0;

	pwm_regs = fnc_pwm();
	if(pwm_regs == MAP_FAILED){
	  fprintf(stderr, "fail to get registers map. %s\n", strerror(errno));
	  return;
	}
	pwm_clk(pwm_regs, 10);	// divide 19 (1010 kHz)
}

/*
 * デストラクタ
 */
void mzbeep_clean() {
	int ret;

	//bPlaying = false;
	pwm_sw(pwm_regs, 0);
	ret = munmap((void *)pwm_regs, getpagesize());
	if(ret)
	  fprintf(stderr, "fail to unmap. %s\n", strerror(errno));
//	dprintf("~mzbeep()\n");
}

/*
 * 消音
 */
void mzbeep_stop() {
	//bPlaying = false;
	pwm_sw(pwm_regs, 0);
//	dprintf("mzbeep::stop()\n");
}

/*
 * 周波数の設定
 */
void mzbeep_setFreq(int arg) {
	//bPlaying = true;
	//freq = arg;
	//pulse_vec = (freq << 16) / hw_freq * 2;
	pwm_spc(pwm_regs, arg);
	pwm_mrk(pwm_regs, (arg + 1) / 2);

	pwm_sw(pwm_regs, 1);
//	dprintf("mzbeep::setFreq(%d)\n",arg);
}

/*
 * 更新
 */
/*void mzbeep_update(short* ptr, int cou) {
	int i;

	short dat;
	if (!bPlaying) {
		return;
	}
	
	for (i=0; i<cou*2; i+=2) {
		if ((pulse_cou += pulse_vec) & 0x10000) {
			bPulse ^= true;
			pulse_cou &= 0x0FFFF;
		}

		dat = bPulse ? -0x2000 : 0x2000;
		ptr[i] += dat;
	}
	
}*/
