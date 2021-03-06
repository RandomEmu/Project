if( debugIsDebug ) {
    if( _PC == debugBreakPoint )            // DEBUG.
        debugIsStep = TRUE;
    
    if( debugIsStep ) {                 // DEBUG.
        fprintf(stderr, "_PC : %04X \n", _PC);
        DebugDisp();
        _getch();
        debugInput = _getch();
        debugEditMode = TRUE;
        if( debugInput == 'e' ) {
            fprintf(stderr, "^^^^^^^ Debug Info ^^^^^^^\n");
            while( debugEditMode ) {
                    fprintf(stderr, "Input an command:");
                    scanf("%X", &debugInput);
                    switch (debugInput) {
                        case 0x00:{ debugIsStep = FALSE;
                                    debugEditMode = FALSE;
                                    break; }        // 退出单步执行模式，直至到达下一个断点 debugBreakPoint 才继续启用单步执行模式。
                        case 0x01:{ fprintf(stderr, "Input an address:");
                                    scanf("%X", &debugInput);
                                    fprintf(stderr, "_gbMem[%04X] = %02X \n", debugInput, _gbMem[debugInput]);
                                    break; }        // 查看 _gbMem 内存值。
                        
                        case 0x02:{ fprintf(stderr, "Address Value:");
                                    scanf("%X", &debugInput);
                                    scanf("%X", &v8Before);
                                    _gbMem[debugInput] = v8Before;
                                    break; }        // 修改 _gbMem 内存值。
                        
                        case 0x55:{ debugIsDebug = FALSE;
                                    debugEditMode = FALSE;
                                    break; }
                
                        default : debugEditMode = FALSE;
                    }
            }
            fprintf(stderr, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
        }
    }
}