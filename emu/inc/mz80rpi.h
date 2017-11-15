
/* Typedef */
typedef	unsigned int UINT32;
typedef	unsigned short UINT16;
typedef	unsigned char UINT8;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned       DWORD;

// FDC�L���H
//#define ENABLE_FDC

/* ���b�Z�[�W�œ��{����g���� */
#define JAPANESE		0

/* */
#define ROM_START	0			/* monitor rom */
#define RAM_START	(4*1024)	/* 48k normal ram */
#define VID_START	(52*1024)	/* video ram */
#define ROM2_START	(60*1024)	/* fd rom */

/* */
#define ROM1_ADDR 0x0000		/* ROM1 mz addr */
#define ROM2_ADDR 0xF000		/* ROM2 mz addr */

#define ROMFONT_SIZE 2048		/* ROMFONT SIZE  */
#define PCG8000_SIZE 2048		/* PCG-8000 SIZE  */

#define CPU_SPEED 2000000		/* 2.000 MHz */
#define CPU_SPEED_BASE 2000000	/* 2.000 MHz */
#define FRAMES_PER_SECOND 60

#define VRAM_ACCESS_WAIT 42
#define TEMPO_TSTATES ((CPU_SPEED*10)/381/2)
#define VBLANK_TSTATES 45547	

/* ���j�^�q�n�l�P�̃^�C�v�萔 */
enum TMON1_TYPE
{
	MON_EMUL = 0,														/*  0:EMUL ROM(NO ROM FILE) */
	MON_OTHERS,															/*  1:OTHERS */
	MON_SP1002,															/*  2:SP-1002 */
	MON_NEWMON80K,														/*  3:MZ-NEW MONITOR */

	MON_LAST,
};

/* ���j�^�q�n�l�Q�̃^�C�v�萔 */
enum TMON2_TYPE
{
	MON_80FIO = 1,														/*  1:FD BOOT ROM */
};

/* MZT�t�@�C���̃w�b�_ */
typedef struct
{
	BYTE mz_fmode;														/* �l�y�̃t�@�C�����[�h */
	BYTE mz_filename[17];												/* �l�y�̃t�@�C���� */
	WORD mz_filesize;													/* �t�@�C���T�C�Y */
	WORD mz_topaddr;													/* �擪�A�h���X */
	WORD mz_execaddr;													/* ���s�A�h���X */
	
	BYTE dummy[0x80-0x18];												/* �p�f�B���O */
} MZTHED;

/* �L�[�e�[�u�� Patch�p�\���� */
typedef struct
{
	WORD addr;															/* patch address */
	BYTE data;															/* patch data */
	
} TPATCH;

/* for MONITOR PATCH */	
typedef struct
{
	WORD addr;															/* patch addr */
	unsigned short code;												/* em code */
} TMONEMU;

// ���b�Z�[�W�L���[ID
enum MSGQID
{
	QID_KBD = 1001,
};