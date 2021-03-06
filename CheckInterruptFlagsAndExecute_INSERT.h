/* =========================================================
    开始检测各个中断位，并根据优先级执行中断。
    在这个 do-whle(FALSE) 体中，最多只有 1 个 if 被完整执行，
    因此各中断的优先级体现在中断所在 if 的先后顺序上。
===========================================================*/
if( _IME ) {    // Interrupt Master Enable flag.
    do {
        if( ( _gbMem[0xFFFF] & MSK_0) && ( _gbMem[0xFF0F] & MSK_0 ) ) {     // V-Blank Interrupt.
            _IME = FALSE;
            isHalt = FALSE;                                                 // Disable IME.
            ResetVBlankInt();
            RefreshAllWindow();
            _SP -= 2;
            pW = (word_t*)&( _gbMem[_SP] );
            *pW = _PC;
            _PC = 0x0040;                   // V-Blank interrupt vector.
            //fprintf(stderr, "V-Blank INT called \n"); // DEBUG.
            break;
        }
        
        // 实现: LCD STAT Interrupt
        
        if( ( _gbMem[0xFFFF] & MSK_2) && ( _gbMem[0xFF0F] & MSK_2 ) ) {     // Timer Interrupt.
            _IME = FALSE;                                                   // Disable IME.
            ResetTimerInt();
            isHalt = FALSE;
            _SP -= 2;
            pW = (word_t*)&( _gbMem[_SP] );
            *pW = _PC;
            _PC = 0x0050;                   // Timer interrupt vector.
            //fprintf(stderr, "Timer INT called \n");   // DEBUG.
            break;
        }
        if( ( _gbMem[0xFFFF] & MSK_4 ) && ( _gbMem[0xFF0F] & MSK_4 ) ) {    // Joypad Interrupt.
            _IME = FALSE;                                                   // Disable IME.
            ResetJoypadInt();
            isHalt = FALSE;
            _SP -= 2;
            pW = (word_t*)&( _gbMem[_SP] );
            *pW = _PC;
            _PC = 0x0060;                   // Joypad interrupt vector.
            //fprintf(stderr, "Joypad INT called \n");  // DEBUG.
            break;
        }
    } while ( FALSE );
}