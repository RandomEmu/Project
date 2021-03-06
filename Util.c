#include <stdio.h>
#include <malloc.h>
#include "BaseType.h"
#include "MainUtil.h"
#include "MemInterfaceUtil.h"
#include "Graphic.h"

extern byte_t
    _A,
    _F,
    _BC[2],
    _DE[2],
    _HL[2],
    _WZ[2];

extern word_t 
    _SP,
    _PC;

extern byte_t *instrPtr;

extern word_t
    *pW_BC,
    *pW_DE,
    *pW_HL,
    *pW_WZ,
    *pW;

extern byte_t   _gbMem[0xFFFF + 1];
extern MemInterFuncPtr_t memFuncArr[MAX_RW_FUNC_PAIR];
extern byte_t addrFlag[0xFFFF + 1];
extern byte_t *_gbRomBuf;
extern byte_t *_gbExtRam;
extern unsigned int INFO_BatterySize;
extern boolean _IME;
extern boolean isHalt;

extern int CyclesLY;
extern int CyclesVBK;

static FILE
    *fIn    = NULL,
    *fOut   = NULL;

static int  BufFile(byte_t **, FILE *, unsigned long);
public int  LoadRom_TEST();
public void ResetGB();
public void CleanUp();

static int BufFile(byte_t **buf, FILE *f, unsigned long limit)
{
    unsigned long size;

    fseek(f, 0L, SEEK_END);
    size = ftell(f);
    if(size > limit) {
        fprintf(stderr, "Error:bufFile() - File must be less than %llu Bytes.\n", limit);
        return ERROR;
    }
    *buf = (byte_t *)malloc(sizeof(byte_t) * size);
    fseek(f, 0L, SEEK_SET);
    if ( fread(*buf, size, 1, f) != 1) {
        fprintf(stderr, "Error:bufFile() - fread() failed.\n");
        return ERROR;
    }
    fclose(f);
    return SUCCESS;
}

/*==============================================
    读与 exe 同目录下的 1.gb 和 1.batt 文件。
    调试时用这个函数可以不用在命令行输入参数，
    直接读取 ROM 文件。
===============================================*/
public int LoadRom_TEST()
{
    if( ! (fIn = fopen("1.gb", "rb")) ) {
        fprintf(stderr, "Error: fopen()\n");
        //getchar();
        return ERROR;
    }
    if( ! BufFile(&_gbRomBuf, fIn, SIZE_2MB) ) {
        fprintf(stderr, "Error: bufFile(ROM)\n");
        return ERROR;
    }
    
    if( ! (fIn = fopen("1.batt", "rb")) ) {
        fprintf(stderr, "No .batt file. New one will be created when necessary.\n");    // DEBUG.
        _gbExtRam = (byte_t *)malloc(sizeof(byte_t) * SIZE_32KB);
    } else {
        if( ! BufFile(&_gbExtRam, fIn, SIZE_32KB) ) {
            fprintf(stderr, "Error: bufFile(Battery)\n");
            return ERROR;
        }
        
    }

    // switch(_gbRomBuf[0x0149]) {
    //  case 0x00: size = 0;        break;
    //  case 0x01: size = SIZE_2KB; break;
    //  case 0x02: size = SIZE_8KB; break;
    //  case 0x03: size = SIZE_32KB;break;
    //  default  : size = 0;
    // }
    fprintf(stderr, "LoadRom_TEST() success\n");
    return SUCCESS;
}

/*==============================================
    由传入的 path 读取 ROM 文件。
===============================================*/
public int LoadRomFromCmdArgement(const char *path)
{
    if( ! (fIn = fopen(path, "rb")) ) {
        fprintf(stderr, "Error: fopen()\n");
        return ERROR;
    }
    if( ! BufFile(&_gbRomBuf, fIn, SIZE_2MB) ) {
        fprintf(stderr, "Error: bufFile(ROM)\n");
        return ERROR;
    }
    
    if( ! (fIn = fopen("1.batt", "rb")) ) {
        fprintf(stderr, "No .batt file. New one will be created when necessary.\n");    // DEBUG.
        _gbExtRam = (byte_t *)malloc(sizeof(byte_t) * SIZE_32KB);
    } else {
        if( ! BufFile(&_gbExtRam, fIn, SIZE_32KB) ) {
            fprintf(stderr, "Error: bufFile(Battery)\n");
            return ERROR;
        }
        
    }

    // switch(_gbRomBuf[0x0149]) {
    //  case 0x00: size = 0;        break;
    //  case 0x01: size = SIZE_2KB; break;
    //  case 0x02: size = SIZE_8KB; break;
    //  case 0x03: size = SIZE_32KB;break;
    //  default  : size = 0;
    // }
    fprintf(stderr, "LoadRomFromCmdArgement() success\n");
    return SUCCESS;
}

/*==============================================
    初始化 GameBoy。
===============================================*/
public void ResetGB()
{
    ResetMain();
    ResetMemInterface();
}

public void DebugDisp()
{
    fprintf(stderr, "=============================================== \n");
    fprintf(stderr, "_PC:Opc = %04X:%02X \n", _PC, Rbyte(_PC, NULL));
    fprintf(stderr, "Z:%1X ", ( _F & MSK_7) >> 7);  fprintf(stderr, " _A:%02X \n", _A);
    fprintf(stderr, "N:%1X ", ( _F & MSK_6) >> 6);  fprintf(stderr, "_BC:%04X \n", W(_BC));
    fprintf(stderr, "H:%1X ", ( _F & MSK_5) >> 5);  fprintf(stderr, "_DE:%04X \n", W(_DE));
    fprintf(stderr, "C:%1X ", ( _F & MSK_4) >> 4);  fprintf(stderr, "_HL:%04X \n", W(_HL));
    fprintf(stderr, "_SP:%04X \n", _SP);
    fprintf(stderr, "_IME:%d \n", _IME);
    fprintf(stderr, "isHalt:%d \n", isHalt);
    fprintf(stderr, "IE (0xFFFF) : 0x%02X\n", _gbMem[0xFFFF]);
    fprintf(stderr, "IF (0xFF0F) : 0x%02X\n", _gbMem[0xFF0F]);  
    fprintf(stderr, "STAT   (0xFF41) : 0x%02X\n", _gbMem[0xFF41]);
    fprintf(stderr, "TIMA   (0xFF05) : 0x%02X\n", _gbMem[0xFF05]);
    fprintf(stderr, "TMA    (0xFF06) : 0x%02X\n", _gbMem[0xFF06]);
    fprintf(stderr, "TAC    (0xFF07) : 0x%02X\n", _gbMem[0xFF07]);
    fprintf(stderr, "Input  (0xFF00) : 0x%02X\n", _gbMem[0xFF00]);
    fprintf(stderr, "       (0xFF80) : 0x%02X\n", _gbMem[0xFF80]);      // GB俄罗斯方块保存按键状态的字节。
    fprintf(stderr, "LY (0xFF44) : %u\n", _gbMem[0xFF44]);
    //fprintf(stderr, "CyclesVBK = %d\n", CyclesVBK);
    //fprintf(stderr, "CyclesLY = %d\n", CyclesLY);
    fprintf(stderr, "=============================================== \n");
}

/*=============================
    释放所有资源（离开程序用） 
==============================*/
void CleanUp()
{
    if( _gbRomBuf != NULL ) {
        free(_gbRomBuf);
        _gbRomBuf = NULL;
    }

    if( _gbExtRam != NULL ) {
        free(_gbExtRam);
        _gbExtRam = NULL;
    }

    CleanUpGraphic();

    fprintf(stderr, "CleanUp()\n");
}