/* === 产生 V-Blank 中断 === */
CyclesVBK -= instrCycles;
if( CyclesVBK < 0 ) {
    CyclesVBK = VBK_MAX;
    SetVBlankInt();
    
    CyclesLY = LY_MAX;                              // 强制使 LY 与 V-Blank 同步。
    _gbMem[0xFF44] = 144;
    
    CyclesSTATmode0 = STAT_MODE_0_MAX;              // 强制使 STAT 变成 Mode 1, 与 V-Blank 同步。
    _gbMem[0xFF41] &= MSK_2_7;
    (_gbMem[0xFF41])++;
    //fprintf(stderr, "V-Blank INT flag seted \n"); // DEBUG.
}