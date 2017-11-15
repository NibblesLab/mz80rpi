//----------------------------------------------------------------------------
// File:MZmain.c
// MZ-80 Emulator mz80rpi for Raspberry Pi
// mz80rpi:Main Program Module based on MZ700WIN
// (c) Nibbles Lab./Oh!Ishi 2017
//
// mz700win by Takeshi Maruyama, based on Russell Marks's 'mz700em'.
// MZ700 emulator 'mz700em' for VGA PCs running Linux (C) 1996 Russell Marks.
// Z80 emulation from 'Z80em' Copyright (C) Marcel de Kogel 1996,1997
//----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mz80rpi.h"

#include "z80.h"
#include "Z80Codes.h"

#include "defkey.h"

#include "mzmain.h"
#include "MZhw.h"
#include "mzscrn.h"
#include "mzbeep.h"

static bool intFlag = false;

static pthread_t scrn_thread_id;
void *scrn_thread(void *arg);
static pthread_t keyin_thread_id;
void *keyin_thread(void *arg);

#define	SOCK_PATH	"/tmp/cmdxfer"
int sockfd;
struct sockaddr_un s_addr;

#define SyncTime	16667000									/* 1/60 sec. */

int q_kbd;
typedef struct KBDMSG_t {
	long mtype;
	char len;
	unsigned char msg[80];
} KBDMSG;
const unsigned char ak_tbl[] =
{
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x91, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x73, 0x05, 0x64, 0x74,
	0x14, 0x00, 0x10, 0x01, 0x11, 0x02, 0x12, 0x03, 0x13, 0x04, 0x80, 0x54, 0x80, 0x25, 0x80, 0x80,
	0x80, 0x40, 0x62, 0x61, 0x41, 0x21, 0x51, 0x42, 0x52, 0x33, 0x43, 0x53, 0x44, 0x63, 0x72, 0x24,
	0x34, 0x20, 0x31, 0x50, 0x22, 0x23, 0x71, 0x30, 0x70, 0x32, 0x60, 0x80, 0x80, 0x80, 0xff, 0xff,
	0xff, 0x40, 0x62, 0x61, 0x41, 0x21, 0x51, 0x42, 0x52, 0x33, 0x43, 0x53, 0x44, 0x63, 0x72, 0x24,
	0x34, 0x20, 0x31, 0x50, 0x22, 0x23, 0x71, 0x30, 0x70, 0x32, 0x60, 0xff, 0xff, 0xff, 0xff, 0xff,
};
const unsigned char ak_tbl_s[] =
{
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0x00, 0x10, 0x01, 0x11, 0x02, 0x12, 0x03, 0x13, 0x04, 0x25, 0x05, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x24, 0xff, 0x20, 0xff, 0x30, 0x33,
	0x23, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x31, 0x32, 0x22, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

//------------------------------------------------
// Memory Allocation for MZ
//------------------------------------------------
int mz_alloc_mem(void)
{
	int result = 0;

	/* Main Memory */
	mem = malloc(64*1024);
	if(mem == NULL)
	{
		return -1;
	}

	/* Junk(Dummy) Memory */
	junk = malloc(4096);
	if(junk == NULL)
	{
		return -1;
	}

	/* MZT file Memory */
	mzt_buf = malloc(4*64*1024);
	if(mzt_buf == NULL)
	{
		return -1;
	}

	/* ROM FONT */
//	font = malloc(ROMFONT_SIZE);
//	if(font == NULL)
//	{
//		result = -1;
//	}

	/* PCG-8000 FONT */
//	pcg8000_font = malloc(PCG8000_SIZE);
//	if(pcg8000_font == NULL)
//	{
//		result = -1;
//	}

	return result;
}

//------------------------------------------------
// Release Memory for MZ
//------------------------------------------------
void mz_free_mem(void)
{
//	if(pcg8000_font)
//	{
//		free(pcg8000_font);
//	}

//	if(font)
//	{
//		free(font);
//	}

	if(mzt_buf)
	{
		free(mzt_buf);
	}

	if(junk)
	{
		free(junk);
	}

	if(mem)
	{
		free(mem);
	}
}

//--------------------------------------------------------------
// ＲＯＭモニタを読み込む
//--------------------------------------------------------------
int rom_load(void)
{
	FILE *in;
	int result = MON_EMUL;

	rom2_mode = MON_EMUL;

	if((in = fopen("/home/pi/mz80rpi/emu/fdif.rom", "r")) != NULL)
	{
		fread(mem + ROM2_START, sizeof(unsigned char), 1024, in);
		fclose(in);
		rom2_mode = MON_80FIO;
	}

	if((in = fopen("/home/pi/mz80rpi/emu/ipl.rom", "r")) != NULL)
	{
		fread(mem, sizeof(unsigned char), 4096, in);
		fclose(in);
		result = set_romtype();
	}

	/* フォントデータを読み込む */
	if(font_load("/home/pi/mz80rpi/emu/font.rom") < 0)
	{
		perror("Couldn't load font data.");
		mz_exit(1);
	}
	
	return result;
}

// モニタＲＯＭタイプを判別し、rom1_modeをセット
int set_romtype(void)
{
	if (!strncmp((char *)(mem+0x14D), "SP-1002", 7))
	{
		return MON_SP1002;
	}
	else
	if (!strncmp((char *)(mem+0x144), "MZ\x90MONITOR", 10))
	{
		return MON_NEWMON80K;
	}

	return MON_OTHERS;
}

//--------------------------------------------------------------
// ＭＺのモニタＲＯＭのセットアップ
//--------------------------------------------------------------
void mz_mon_setup(void)
{
	FILE *fd;

	memset(mem, 0xFF, 64*1024);
	memset(mem+RAM_START, 0, 48*1024);
	memset(mem+VID_START, 0, 1024);

	// ROMモニタの読み込み
	rom1_mode = rom_load();

	fd = fopen("/home/pi/mz80rpi/emu/Sp-5030.mzt", "r");
	fseek(fd, 0x80, SEEK_SET);
	fread(mem+0x1200, sizeof(unsigned char), 12361-128, fd);
	fclose(fd);
}

//--------------------------------------------------------------
// UNIXドメイン通信の準備
//--------------------------------------------------------------
static int setupXfer(void)
{
	// UNIXドメインソケットの生成
	sockfd = socket(AF_UNIX, SOCK_STREAM|SOCK_NONBLOCK, 0);
	if(sockfd == -1)
	{
		perror("socket");
		return -1;
	}

	// ソケット用のパスを消しておく
	if(remove(SOCK_PATH) == -1 && errno != ENOENT)
	{
		perror("remove SOCK_PATH");
		return -1;
	}

	// バインドと待ち受け
	memset(&s_addr, 0, sizeof(struct sockaddr_un));
	s_addr.sun_family = AF_UNIX;
	strncpy(s_addr.sun_path, SOCK_PATH, sizeof(s_addr.sun_path) - 1);
	if(bind(sockfd, (struct sockaddr *)&s_addr, sizeof(struct sockaddr_un)) == -1)
	{
		perror("bind");
		return -1;
	}
	if(listen(sockfd, 5) == -1)
	{
		perror("listen");
		return -1;
	}

	return 0;
}

//--------------------------------------------------------------
// 外部プログラムからのコマンド処理
//--------------------------------------------------------------
static void processXfer(void)
{
	int confd, num;
	unsigned char ch, cmd[80];
	KBDMSG kbdm;
	
	// 接続
	confd = accept(sockfd, NULL, NULL);
	if(confd == -1)
	{
		return;
	}

	// 1行受信
	ch = 0x01;
	num = 0;
	memset(cmd, 0, 80);
	while(ch != 0)
	{
		if(read(confd, &ch, 1) == 0)
		{
			usleep(1000);
			continue;
		}
		cmd[num++] = ch;
	}

	switch(cmd[0])
	{
	case 'R':	// Reset MZ
		mz_reset();
		break;
	case 'C':	// Casette Tape
		switch(cmd[1])
		{
		case 'T':	// Set Tape
			if(ts700.cmt_tstates == 0)
			{
				set_mztData((char *)&cmd[2]);
				ts700.cmt_play = 0;
			}
			break;
		case 'P':	// Play Tape
			if(ts700.mzt_settape != 0)
			{
				ts700.cmt_play = 1;
				ts700.mzt_start = ts700.cpu_tstates;
				ts700.cmt_tstates = 1;
				setup_cpuspeed(5);
			}
			break;
		case 'S':	// Stop Tape
			if(ts700.cmt_tstates != 0)
			{
				ts700.cmt_play = 0;
				ts700.cmt_tstates = 0;
				setup_cpuspeed(1);
				ts700.mzt_elapse += (ts700.cpu_tstates - ts700.mzt_start);
			}
			break;
		case 'E':	// Eject Tape
			if(ts700.cmt_tstates == 0)
			{
				ts700.mzt_settape = 0;
			}
			break;
		default:
			break;
		}
		break;
	case 'P':	// PCG
		if(cmd[1] == '0')	// OFF
		{
			hw700.pcg8000_mode = 0;
		}
		else if(cmd[1] == '1')	// ON
		{
			hw700.pcg8000_mode = 1;
		}
		break;
	case 'K':	// Key in
		kbdm.mtype = 1;
		kbdm.len = num;
		memcpy(kbdm.msg, &cmd[1], num);
		msgsnd(q_kbd, &kbdm, 81, IPC_NOWAIT);
		break;
	default:
		break;
	}

	close(confd);
}

//--------------------------------------------------------------
// シグナル関連
//--------------------------------------------------------------
void sighandler(int act)
{
	intFlag = true;
}

bool intByUser(void)
{
	return intFlag;
}

void mz_exit(int arg)
{
	sighandler(0);
}

//--------------------------------------------------------------
// メイン部
//--------------------------------------------------------------
int main(int argc, char *argv[])
{
	struct sigaction sa;

	sigaction(SIGINT, NULL, &sa);
	sa.sa_handler = sighandler;
	sa.sa_flags = SA_NODEFER;
	sigaction(SIGINT, &sa, NULL);

	mz_screen_init();
	mz_alloc_mem();
	init_defkey();
	read_defkey();
	makePWM();

	// キー1行入力用メッセージキューの準備
	q_kbd = msgget(QID_KBD, IPC_CREAT);

	// 外部プログラムとの通信の準備
	setupXfer();

	// ＭＺのモニタのセットアップ
	mz_mon_setup();

	// メインループ実行
	mainloop();

	// 終了
	mzbeep_clean();
	end_defkey();
	mz_free_mem();
	mz_screen_finish();

	return 0;

}


////-------------------------------------------------------------
////  mz700win MAINLOOP
////-------------------------------------------------------------
void mainloop(void)
{
	int _synctime = SyncTime;
	struct timespec timetmp, w, synctmp;

	mzbeep_init(44100);

	/* スレッド　開始 */
	start_thread();
	
	// Reset MZ
	mz_reset();

	setup_cpuspeed(1);
	Z80_IRQ = 0;
//	Z80_Trap = 0x0556;

	w.tv_sec = 0;

	// start the CPU emulation
	while(!intByUser())
	{
		processXfer();	// 外部プログラムからのコマンド処理

		clock_gettime(CLOCK_MONOTONIC_RAW, &timetmp);
		if (!Z80_Execute()) break;
		clock_gettime(CLOCK_MONOTONIC_RAW, &synctmp);
		if(synctmp.tv_nsec < timetmp.tv_nsec)
		{
			w.tv_nsec = _synctime - (1000000000 + synctmp.tv_nsec - timetmp.tv_nsec);
		}
		else
		{
			w.tv_nsec = _synctime - (synctmp.tv_nsec - timetmp.tv_nsec);
		}
		if(w.tv_nsec > 0)
		{
			nanosleep(&w, NULL);
		}

		if (Z80_Trace)
		{
			usleep(1000000);
		}
	}
//	
}

//------------------------------------------------------------
// CPU速度を設定 (10-100)
//------------------------------------------------------------
void setup_cpuspeed(int mul) {
	int _iperiod;
	//int a;

	_iperiod = (CPU_SPEED*CpuSpeed*mul)/(100*IFreq);

	//a = (per * 256) / 100;

	_iperiod *= 256;
	_iperiod >>= 8;

	Z80_IPeriod = _iperiod;
	Z80_ICount = _iperiod;

}

//--------------------------------------------------------------
// スレッドの準備
//--------------------------------------------------------------
int create_thread(void)
{
	return 0;
}

//--------------------------------------------------------------
// スレッドの開始
//--------------------------------------------------------------
void start_thread(void)
{
	int st;

	st = pthread_create(&scrn_thread_id, NULL, scrn_thread, NULL);
	if(st != 0)
	{
		perror("update_scrn_thread");
		return;
	}
	pthread_detach(scrn_thread_id);

	st = pthread_create(&keyin_thread_id, NULL, keyin_thread, NULL);
	if(st != 0)
	{
		perror("keyin_thread");
		return;
	}
	pthread_detach(keyin_thread_id);

}

//--------------------------------------------------------------
// スレッドの後始末
//--------------------------------------------------------------
int end_thread(void)
{
	return 0;
}

//--------------------------------------------------------------
// 画面描画スレッド 
//--------------------------------------------------------------
void * scrn_thread(void *arg)
{
	struct timespec vsynctmp, timetmp, vsyncwait;
	
	vsyncwait.tv_sec = 0;

	while(!intByUser())
	{
		// 画面更新処理
		hw700.retrace = 1;											/* retrace = 0 : in v-blnk */
		vblnk_start();

		clock_gettime(CLOCK_MONOTONIC_RAW, &timetmp);
		update_scrn();												/* 画面描画 */

		clock_gettime(CLOCK_MONOTONIC_RAW, &vsynctmp);
		if(vsynctmp.tv_nsec < timetmp.tv_nsec)
		{
			vsyncwait.tv_nsec = SyncTime - (1000000000 + vsynctmp.tv_nsec - timetmp.tv_nsec);
		}
		else
		{
			vsyncwait.tv_nsec = SyncTime - (vsynctmp.tv_nsec - timetmp.tv_nsec);
		}
		if(vsyncwait.tv_nsec > 0)
		{
			nanosleep(&vsyncwait, NULL);
		}

	}
	return NULL;
}

//--------------------------------------------------------------
// キーボード入力スレッド
//--------------------------------------------------------------
void * keyin_thread(void *arg)
{
	int fd = 0, fd2, key_active = 0, i;
	struct input_event event;
	KBDMSG kbdm;

	fd2 = open("/dev/input/mice", O_RDONLY);
	if(fd2 != -1)
	{
		ioctl(fd2, EVIOCGRAB, 1);
	}

	while(!intByUser())
	{
		// キー入力
		if(key_active == 0)
		{
			fd = open("/dev/input/event0", O_RDONLY);
			if(fd != -1)
			{
				ioctl(fd, EVIOCGRAB, 1);
				key_active = 1;	// キーデバイス確認、次回はキー入力
			}
		}
		else
		{
			if(read(fd, &event, sizeof(event)) == sizeof(event))
			{
				if(event.type == EV_KEY)
				{
					switch(event.value)
					{
					case 0:
						mz_keyup(event.code);
						break;
					case 1:
						mz_keydown(event.code);
						break;
					default:
						break;
					}
					//printf("code = %d, value = %d\n", event.code, event.value);
				}
			}
			else
			{
				key_active = 0;	// キーデバイス消失、次回はデバイスオープンを再試行
			}
		}

		// 外部プログラムからの1行入力
		if(msgrcv(q_kbd, &kbdm, 81, 0, IPC_NOWAIT) != -1)
		{
			for(i = 0; i < kbdm.len; i++)
			{
				if(ak_tbl[kbdm.msg[i]] == 0xff)
				{
					continue;
				}
				else if(ak_tbl[kbdm.msg[i]] == 0x80)
				{
					mz_keydown_sub(ak_tbl[kbdm.msg[i]]);
					usleep(60000);
					mz_keydown_sub(ak_tbl_s[kbdm.msg[i]]);
					usleep(60000);
					mz_keyup_sub(ak_tbl_s[kbdm.msg[i]]);
					usleep(60000);
					mz_keyup_sub(ak_tbl[kbdm.msg[i]]);
					usleep(60000);
				}
				else
				{
					mz_keydown_sub(ak_tbl[kbdm.msg[i]]);
					usleep(60000);
					mz_keyup_sub(ak_tbl[kbdm.msg[i]]);
					usleep(60000);
				}
			}
			mz_keydown_sub(0x84);	// CR
			usleep(60000);
			mz_keyup_sub(0x84);
			usleep(60000);
		}
		usleep(10000);
	}

	ioctl(fd, EVIOCGRAB, 0);
	close(fd);
	ioctl(fd2, EVIOCGRAB, 0);
	close(fd2);
	return NULL;
}
