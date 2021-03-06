/* === 0xFF41 STAT 状态转换 === */
switch( _gbMem[0xFF41] & MSK_0_1 ) {
    case 0x00:{ CyclesSTATmode0 -= instrCycles;
                if( CyclesSTATmode0 < 0 ) {
                    CyclesSTATmode0 = STAT_MODE_0_MAX;      // 离开当前状态前先把自身计数器加满，等下一次一进入本状态就可以从最满开始计时。
                    if( _gbMem[0xFF44] > 143 )              // 根据 LY 来判断是否该进入 Mode 1，所以本 switch 应该放在更新 LY 的代码后面。    
                        ( _gbMem[0xFF41] )++;               // 变成 Mode 1, 因为现在是 0x00 ，自增后肯定会变成 0x01.
                }
                break; }

    case 0x01:{ //CyclesSTATmode1 -= instrCycles;           // 不要这句。放弃采用独立计数器，做到尽量跟 LY 走。
                if( _gbMem[0xFF44] < 144 )                  // 同样根据 LY 来判断是否进入 Mode 2。
                    ( _gbMem[0xFF41] )++;                   // 变成 Mode 2.
                break; }

    case 0x02:{ CyclesSTATmode2 -= instrCycles;
                if( CyclesSTATmode2 < 0 ) {
                    CyclesSTATmode2 = STAT_MODE_2_MAX;
                    ( _gbMem[0xFF41] )++;                   // 变成 Mode 3.
                }
                break; }

    case 0x03:{ CyclesSTATmode3 -= instrCycles;
                if( CyclesSTATmode3 < 0 ) {
                    CyclesSTATmode3 = STAT_MODE_3_MAX;
                    _gbMem[0xFF41] &= MSK_2_7;              // 变成 Mode 0.
                }
                break; }

    default:    break;
}