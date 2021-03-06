#include <stdio.h>
#include <conio.h>
#include <SDL.h>
#include "BaseType.h"
#include "Util.h"
#include "MemInterface.h"
#include "Graphic.h"

public byte_t
    _A,
    _F,
    _BC[2],      // {C, B}
    _DE[2],      // {E, D}
    _HL[2],      // {L, H}
    _WZ[2];      // {Z, W}  仅用来做临时运算。与 Z80 的内部寄存器 WZ 无关，这里只是拿 WZ 这个名字来用而已。

public word_t 
    _SP,
    _PC;

public word_t
    *pW_BC = (word_t*)_BC,
    *pW_DE = (word_t*)_DE,
    *pW_HL = (word_t*)_HL,
    *pW_WZ = (word_t*)_WZ,
    *pW = NULL;

static byte_t
    *pB_SP = (byte_t*)&_SP,
    *pB_PC = (byte_t*)&_PC;

static byte_t
    v8Before,
    v8After;

static word_t
    v16Before,
    v16After;

static dword_t
    v32Carry;

public boolean _IME = TRUE;                     // Interrupt Master Enable.重要。

public byte_t   _gbMem[0xFFFF + 1];         // GameBoy 内存。
public byte_t addrFlag[0xFFFF + 1];         // 每个内存地址的标志（作为 memFuncArr[] 的元素序号）。用于 GB 系统的 R/W 函数对从数组头部开始放置， MBC 系列函数从数组尾部开始放置。
public MemInterFuncPtr_t memFuncArr[MAX_RW_FUNC_PAIR];  // 函数指针数组，用来存放指向 MemInterface.c 中函数的指针, [X].r 指向从某地址 read 的函数，[X].w 指向从某地址 write 的函数 。数组先定义这么多个，以后等发现更多地址后再增加长度。

public byte_t *_gbRomBuf = NULL;            // 整个 ROM 的缓存。运行时 malloc.
public byte_t *_gbExtRam = NULL;            // External Ram in cartridge. Up to 32K. 运行时 malloc.

public byte_t *instrPtr;                    // 存放当前指令的地址，当指令需要时，直接从该地址的下 1，2 个地址读操作数，
                                            // 不必再调用 MemInterface.c 函数，减少不必要的性能消耗。
                                            // Fetch 的时候以 &instrPtr 形式作为第二参数传入 MemInterFuncPtr_t::r()。

static int instrCycles = 0;                 // 当前指令的执行周期。重要。
public int CyclesLY = LY_MAX;
public int CyclesVBK = VBK_MAX;
public int CyclesSTATmode0 = STAT_MODE_0_MAX;
public int CyclesSTATmode2 = STAT_MODE_2_MAX;
public int CyclesSTATmode3 = STAT_MODE_3_MAX;

public int CyclesDIV        = DIV_MAX;
public int CyclesTIMA4096   = 0;
public int CyclesTIMA262144 = 0;
public int CyclesTIMA65536  = 0;
public int CyclesTIMA16384  = 0;

public boolean colButton_0_A        = FALSE;
public boolean colButton_1_B        = FALSE;
public boolean colButton_2_Select   = FALSE;
public boolean colButton_3_Start    = FALSE;

public boolean colDirection_0_Right = FALSE;
public boolean colDirection_1_Left  = FALSE;
public boolean colDirection_2_Up    = FALSE;
public boolean colDirection_3_Down  = FALSE;


public boolean isHalt       = FALSE;                // CPU 挂起状态标志。
public boolean isStop       = FALSE;                // CPU 停止状态标志，若为真，则退出运行循环。
static boolean isUserQuit   = FALSE;                // 模拟器运行标志，若为真，则退出运行循环。

static boolean  debugIsDebug    = FALSE;            // DEBUG 用，DEBUG 模式主标志。
static word_t   debugBreakPoint = 0x729D;           // DEBUG 用，断点地址。
static boolean  debugIsStep     = FALSE;            // DEBUG 用，是否单步执行。
static word_t   debugInput;                         // DEBUG 用，输入的调试命令。
static boolean  debugEditMode   = TRUE;             // DEBUG 用，修改模式状态标志。

SDL_Event ev;

static char userComm[100];

public unsigned int INFO_BatterySize;       // 电池记忆体大小。在读 ROM 时初始化。保存 .sav 文件时会用到。
public unsigned int INFO_MBC_Type;          // 卡带 MBC 类型。

public void ResetMain();

int main(int argc, char* argv[])
{
    do {
        //fprintf(stdout, "Command:");
        //fscanf(stdin, "%s", userComm);

        /*if(strcmp(userComm, "q") == 0)
            break;*/
        if( argc < 2 ) {
            fprintf(stderr, "Syntax error.\n");
            fprintf(stderr, "Syntax: AppName [driver:][path]filename.gb [-d]\n");
            fprintf(stderr, "Press any key to exit...... \n");
            _getch();
            break;
        }

        if( argc == 3 )
            isDebugGraphic = TRUE;
        else isDebugGraphic = FALSE;

        InitSDLGraphic();

        if( ! LoadRomFromCmdArgement(argv[1]) ) {
            fprintf(stderr, "LoadRomFromCmdArgement() ERROR \n");
            fprintf(stderr, "Press any key to exit...... \n");
            _getch();
            break;
        }
        
        fprintf(stderr, "ROM loaded.\n");   // DEBUG.

        if( ! InitMemInterface() ) {
            fprintf(stderr, "InitMemInterface() failed \n");
            break;
        }   
        fprintf(stderr, "Mem interface initialized\n"); // DEBUG.
        
        ResetGB();
        fprintf(stderr, "GB reseted \n");   // DEBUG.

        while( ! isUserQuit) {
            
            if(isStop){
                fprintf(stderr, "Stoped (present _PC = %04X Opcode = %02X) \n", _PC, Rbyte(_PC, NULL) );        // DEBUG.
                _getch();               // DEBUG.
                isUserQuit = TRUE;
                break;
            } else {
                if(isHalt) {
                    
                    // CPU halted. GameBoy Low Power Consumtion Mode. Do nothing.

                } else {

                    Rbyte(_PC, &instrPtr);              // Fetch.

                    /* === DEBUG. 断点，单步 === */
                    #include "Debug_BreakpointAndStep_INSERT.h"
                    
                    /* === Decode, Execute === */
                    switch( instrPtr[0] ) {
                        #include "CPU_Instr_INSERT.h"
                        
                        default : isStop = TRUE;
                    }                   

                }   // if(isHalt)
                
                //_gbMem[0xFF40] &= 0xF7;       // DEBUG.强制使用 BG Map table 0 ( 0x9800-0x9BFF ).
                //_gbMem[0xFF40] |= 0x08;       // DEBUG.强制使用 BG Map table 1 ( 0x9C00-0x9FFF ).
                //_gbMem[0xFFFF] |= 0x04;       // DEBUG.强制允许 Timer 中断。
                //_gbMem[0xFF07] |= 0x04;       // DEBUG.强制开始 TIMA 计时。

                if( isHalt )
                    instrCycles = 100;              // 暂时决定在 CPU 被 halt 的时候，以这个指令周期作为下面非 CPU 部件的更新速度。

                /* === 产生 V-Blank 中断 === */
                #include "InterruptGenerate_V-Blank_INSERT.h"

                /* === 0xFF44 LY 更新 === */
                #include "LY(0xFF44)Update_INSERT.h"

                /* === 0xFF41 STAT 状态转换 === */
                #include "STAT(0xFF41)Update_INSERT.h"

                /* === 0xFF04 DIV 更新 === */
                #include "DIV(0xFF04)Update_INSERT.h"

                /* === 产生 0xFF05 TIMA 中断 === */
                #include "InterruptGenerate_Timer_INSERT.h"

                /* === 产生 Joypad 中断 === */
                #include "InterruptGenerate_Joypad_INSERT.h"

                /* === 开始检测各个中断位，并根据优先级执行中断 === */
                #include "CheckInterruptFlagsAndExecute_INSERT.h"

            }   // if(isStop)
            
            /*if( debugIsDebug == FALSE ) {         // DEBUG. 正常运行时，可以随时按任意键来进入单步运行状态。
                if(_kbhit()) {
                    debugIsDebug = TRUE;
                    debugIsStep = TRUE;
                    DebugDisp();
                    _getch();
                    if( _getch() == 'q')
                        break;
                }
            }*/
        
        }   // while(isUserQuit)
                
    } while( FALSE );
    
    SDL_Quit();
    CleanUp();
    return 0;
}




/*=================================================
    初始化本文件内的 GameBoy 相关寄存器与相关数组
    本函数被 Util.c::ResetGB() 调用。
==================================================*/
public void ResetMain()
{
    _PC = 0x0100;
    _SP = 0xFFFE;
    _A = 0x01;
    _F = 0xB0;
    W(_BC) = 0x0013;
    W(_DE) = 0x00D8;
    W(_HL) = 0x014D;
    W(_WZ) = 0x0000;

    _gbMem[0xFF00] = 0xCF;  // P1/JOYP  数据来源 BGB 模拟器.
    _gbMem[0xFF05] = 0x00;  // TIMA.
    _gbMem[0xFF06] = 0x00;  // TMA.
    _gbMem[0xFF07] = 0x00;  // TAC.
    _gbMem[0xFF40] = 0x91;  // LCDC.
    _gbMem[0xFF42] = 0x00;  // SCY.
    _gbMem[0xFF43] = 0x00;  // SCX.
    _gbMem[0xFF45] = 0x00;  // LYC.
    _gbMem[0xFF47] = 0xFC;  // BGP.
    _gbMem[0xFF48] = 0xFF;  // OBP0.
    _gbMem[0xFF49] = 0xFF;  // OBP1.
    _gbMem[0xFF4A] = 0x00;  // WY.
    _gbMem[0xFF4B] = 0x00;  // WX.
    _gbMem[0xFFFF] = 0x00;  // IE.

    _IME = TRUE;
    instrCycles = 0;

    isStop      = FALSE;
    isHalt      = FALSE;
    isUserQuit  = FALSE;
    
}