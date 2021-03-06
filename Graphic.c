#include <SDL.h>
#include <stdio.h>
//#include <conio.h>
#include "BaseType.h"

public boolean isDebugGraphic = FALSE;

extern byte_t _gbMem[0xFFFF + 1];

static SDL_Window 
            *tileWindow = NULL,
            *bgWindow = NULL,
            *internalWarppedScreenWindow = NULL,
            *mainScreenWindow = NULL;

static SDL_Renderer 
            *tileWindowRenderer = NULL,
            *bgWindowRenderer = NULL,
            *internalWarppedScreenRenderer = NULL,
            *mainScreenRenderer = NULL;

static SDL_Texture 
            *tileTexture = NULL,
            *bgTexture = NULL,
            *internalWarppedScreenTexture = NULL,
            *mainScreenTexture = NULL;

static SDL_Surface
            *surOri8_8 = NULL,          // 需要水平翻转的一个 Tile 的原图像， 8*8 大小。
            *surOri8_16 = NULL,         // 同上，8*16 大小。
            *surRes8_8 = NULL,          // 根据原图像翻转后的结果图像， 8*8 大小。
            *surRes8_16 = NULL,         // 同上，8*16 大小。
            *surTile = NULL,
            *surTileScreen = NULL,
            *surBgScreen = NULL,
            *surGbWindow = NULL,
            *surInternalWarppedScreen = NULL,
            *surMainScreen = NULL;

static byte_t *vramTileData= & _gbMem[0x8000];
static byte_t grayValTile[8*8*384];
static byte_t surTilePixValArr[4*8*8*384];              // pixel data in Surface

static byte_t debugBgp0[] = {255, 170, 85, 0};          // Palette 0.
static byte_t debugBgp1[] = {0, 85, 170, 255};          // Palette 1.

int isX_Flip = 0,
    isY_Flip = 0;

public void InitSDLGraphic()
{
    SDL_Init( SDL_INIT_VIDEO ); 

    surTile = SDL_CreateRGBSurfaceFrom(surTilePixValArr, 8, 8*384, 8*4, 4*8, 0, 0, 0, 0);   
    surTileScreen = SDL_CreateRGBSurface(0, 8*16, 8*24, 8*4, 0, 0, 0, 0);
    surBgScreen = SDL_CreateRGBSurface(0, 8*32, 8*32, 8*4, 0, 0, 0, 0);
    surGbWindow = SDL_CreateRGBSurface(0, 8*32, 8*32, 8*4, 0, 0, 0, 0);
    surInternalWarppedScreen = SDL_CreateRGBSurface(0, 256+160+8, 256+144+16, 8*4, 0, 0, 0, 0);
    surMainScreen = SDL_CreateRGBSurface(0, 160, 144, 8*4, 0, 0, 0, 0);
    
    surOri8_8 = SDL_CreateRGBSurface(0, 8, 8, 8*4, 0, 0, 0, 0);
    surRes8_8 = SDL_CreateRGBSurface(0, 8, 8, 8*4, 0, 0, 0, 0);
    surOri8_16 = SDL_CreateRGBSurface(0, 8, 16, 8*4, 0, 0, 0, 0);
    surRes8_16 = SDL_CreateRGBSurface(0, 8, 16, 8*4, 0, 0, 0, 0);

    if( isDebugGraphic ) {
        tileWindow = SDL_CreateWindow("VRAM Tile Data", 3, 21, 8*16*3, 8*24*3, SDL_WINDOW_OPENGL);
        tileWindowRenderer = SDL_CreateRenderer(tileWindow, -1, SDL_RENDERER_ACCELERATED);
        SDL_SetRenderDrawBlendMode(tileWindowRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(tileWindowRenderer, 0, 178, 178, 0);
        tileTexture = SDL_CreateTexture(tileWindowRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 8*16, 8*24);
    
        bgWindow = SDL_CreateWindow("Background Map", 8*16*3+3, 21, 8*32*2, 8*32*2, SDL_WINDOW_OPENGL);
        bgWindowRenderer = SDL_CreateRenderer(bgWindow, -1, SDL_RENDERER_ACCELERATED);
        SDL_SetRenderDrawBlendMode(bgWindowRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(bgWindowRenderer, 0, 178, 178, 0);
        bgTexture = SDL_CreateTexture(bgWindowRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 8*32, 8*32);
    
        internalWarppedScreenWindow = SDL_CreateWindow("Inner Warpped Screen", 8*16*3+3+8*32*2, 21, (256+160+8)*1, (256+144+16)*1, SDL_WINDOW_OPENGL);
        internalWarppedScreenRenderer = SDL_CreateRenderer(internalWarppedScreenWindow, -1, SDL_RENDERER_ACCELERATED);
        SDL_SetRenderDrawBlendMode(internalWarppedScreenRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(internalWarppedScreenRenderer, 0, 178, 178, 0);
        internalWarppedScreenTexture = SDL_CreateTexture(internalWarppedScreenRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256+160+8, 256+144+16);
    }

    mainScreenWindow = SDL_CreateWindow("TiGB 1.01", 600, 300, 160*2, 144*2, SDL_WINDOW_OPENGL);
    mainScreenRenderer = SDL_CreateRenderer(mainScreenWindow, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(mainScreenRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(mainScreenRenderer, 0, 178, 178, 0);
    mainScreenTexture = SDL_CreateTexture(mainScreenRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
}


/* =======================================================================
    把从 gray 地址开始的一个 Tile 灰度值写入到 pixArr 像素数组的相应位置中
==========================================================================*/
static void UpdateOneTile(byte_t *gray, byte_t *pixArr)
{   
    const static int LENTH = 4*8*8;
    int i, j;
    
    j = 0;
    for(i = 0; i < LENTH; i += 4){
        pixArr[i] = pixArr[i + 1] = pixArr[i + 2] = debugBgp0[ gray[j] ];   // Blue,Green,Red.
        //pixArr[i + 3] = 255;                                              // Alpha.
        pixArr[i + 3] = 0;                                              // Alpha.
        j++;                                                                // Next tile.
    }

}

static void UpdateAllTile()
{
    int i;

    byte_t
        *gray = grayValTile,
        *pixArr = surTilePixValArr;

    for(i = 0; i < 384; i++) {
        UpdateOneTile(gray, pixArr);
        gray += 64;
        pixArr += 4*64;
    }

    //fprintf(stderr, "UpdateAllTile() -- gray = %d ,pixArr = %d \n", gray - grayValTile,  pixArr - surTilePixValArr);
}

/* =====================================================
    把从 vramTile 地址开始的 16 个字节解析成 {0,1,2,3} 的灰度值
    存放于从 gray 地址开始的 64 个元素中。
=======================================================*/
static void TranslateOneTile(byte_t *vramTile, byte_t *gray)
{
    byte_t msk;

    int i, j;

    int n = 0;

    for(i = 0; i < 8; i++) {
        msk = 0x80;
        for(j = 7; j > -1; j--) {
            gray[n++] = ( (vramTile[0] & msk) >> j )
                        + 
                        ( ( (vramTile[1] & msk) << 1 ) >> j );
            msk >>= 1;
        }

        vramTile += 2;
    }
}

static void TranslateAllTile()
{
    int i = 0;

    byte_t
        *vramTile = vramTileData,
        *gray = grayValTile;

    for(i = 0; i < 384; i++) {
        TranslateOneTile(vramTile, gray);
        vramTile += 16;
        gray += 64;
    }

    //fprintf(stderr, "TranslateAllTile() -- vramTile = %d ,gray = %d \n", vramTile - vramTileData,  gray - grayValTile);
}

static void ArrangeTilesInSurface()
{
    int i, j;
    static SDL_Rect srcR, desR;             // 设置 static 只是为了不用每次调用本函数时都要创建 SDL_Rect.
                                            // 本函数的 static 变量在每次调用本函数都会重新赋值，不用担心。
    
    srcR.x = srcR.y = desR.x = desR.y = 0;
    srcR.w = srcR.h = desR.w = desR.h = 8;

    for(i = 0; i < 24; i++) {               // 24 rows of Tile.
        desR.x = 0;
        for(j = 0; j < 16; j++) {           // 16 cols a row.
            SDL_BlitSurface(surTile, &srcR, surTileScreen, &desR);
            srcR.y += 8;
            desR.x +=8;
        }

        desR.y += 8;
    }

}

static void ConstructBackground()
{
    static SDL_Rect srcR, desR;
    byte_t *bgMap = NULL;
    //unsigned int tileNum = 0;
    unsigned int n = 0;
    int i, j;

    srcR.x = srcR.y = desR.x = desR.y = 0;
    srcR.w = srcR.h = desR.w = desR.h = 8;
    n = 0;


    if( _gbMem[0xFF40] & MSK_3 )        // Bit 3 - BG Tile Map Display Select     (0=9800-9BFF, 1=9C00-9FFF)
        bgMap = &(_gbMem[0x9C00]);
    else bgMap = &(_gbMem[0x9800]);

    if( _gbMem[0xFF40] & MSK_4 ) {
        for(i = 0; i < 32; i++) {
            desR.x = 0;
            for(j = 0; j < 32; j++) {
                if( (bgMap[n]) & MSK_7 )
                    srcR.y = ( (unsigned int)(bgMap[n]) ) << 3;
                //else srcR.y = 256 * 8 + ( ( (unsigned int)(bgMap[n]) ) << 3 );
                else srcR.y = ( ( (unsigned int)(bgMap[n]) ) << 3 );
                n++;
                SDL_BlitSurface(surTile, &srcR, surBgScreen, &desR);
                desR.x += 8;
            }
            desR.y += 8;
        }
    } else {
        for(i = 0; i < 32; i++) {
            desR.x = 0;
            for(j = 0; j < 32; j++) {
                srcR.y = ( 256 + ((signed char)(bgMap[n])) ) << 3;
                n++;
                SDL_BlitSurface(surTile, &srcR, surBgScreen, &desR);
                desR.x += 8;
            }
            desR.y += 8;
        }
    }
    
}

/* =========================================================
    构造 GB Window 并把它 blit 到 surInternalWarppedScreen
    注意，此函数因为要用到在 ConstructBackground() 中已经
    画好的 surBgScreen ，所以本函数“一定”放在 
    ConstructBackground() “之后”执行。
===========================================================*/
public void AddGbWindowToWarppedScreen()
{
    static SDL_Rect srcR, desR;
    byte_t *bgMap = NULL;
    unsigned int n = 0;
    int i, j;

    int posX = _gbMem[0xFF4B] - 7 + _gbMem[0xFF43] + 8,     // WX + SCX.
        posY = _gbMem[0xFF4A] + _gbMem[0xFF42] + 16;        // WY + SCY.
    
    if( (posX > 415) || (posY > 399) )                      // 超出 surInternalWarppedScreen 的范围就直接不用画 Window 了。
        return;

    if( (_gbMem[0xFF40] & MSK_3) == (_gbMem[0xFF40] & MSK_6) ) {    // 判断 Window 与 Background 是否使用同一 Map 表。
        /* === 若使用同一 Map 表则可直接用 ConstructBackground() 已经画好的 surBgScreen === */
        srcR.x = srcR.y = 0;                                // The Window is always displayed starting from its left upper corner.
        desR.x = posX;
        desR.y = posY;
        srcR.w = desR.w = 416 - posX;
        desR.h = srcR.h = 400 - posY;
        SDL_BlitSurface(surBgScreen, &srcR, surGbWindow, &desR);
        SDL_BlitSurface(surGbWindow, &desR, surInternalWarppedScreen, &desR);
    } else {
        /* === 若使用不同 Map 表则要重新画，选取 Tile 的代码直接拿 ConstructBackground() 里面的 === */
        srcR.x = srcR.y = desR.x = desR.y = 0;
        srcR.w = srcR.h = desR.w = desR.h = 8;
        n = 0;

        if( _gbMem[0xFF40] & MSK_6 )                        // Bit 6 - Window Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
            bgMap = &(_gbMem[0x9C00]);
        else bgMap = &(_gbMem[0x9800]);
    
        if( _gbMem[0xFF40] & MSK_4 ) {
            for(i = 0; i < 32; i++) {
                desR.x = 0;
                for(j = 0; j < 32; j++) {
                    if( (bgMap[n]) & MSK_7 )
                        srcR.y = ( (unsigned int)(bgMap[n]) ) << 3;
                    else srcR.y = ( ( (unsigned int)(bgMap[n]) ) << 3 );
                    n++;
                    SDL_BlitSurface(surTile, &srcR, surGbWindow, &desR);
                    desR.x += 8;
                }
                desR.y += 8;
            }
        } else {
            for(i = 0; i < 32; i++) {
                desR.x = 0;
                for(j = 0; j < 32; j++) {
                    srcR.y = ( 256 + ((signed char)(bgMap[n])) ) << 3;
                    n++;
                    SDL_BlitSurface(surTile, &srcR, surGbWindow, &desR);
                    desR.x += 8;
                }
                desR.y += 8;
            }
        }   // if(_gbMem[0xFF40])

        srcR.x = srcR.y = 0;                                    // The Window is always displayed starting from its left upper corner.
        desR.x = posX;
        desR.y = posY;
        srcR.w = desR.w = 416 - posX;
        desR.h = srcR.h = 400 - posY;

        SDL_BlitSurface(surGbWindow, &srcR, surInternalWarppedScreen, &desR);
    }
}

static void X_Flip_8_8(SDL_Surface *ori, SDL_Surface *res)
{
    static SDL_Rect srcR, desR;
    int i, j;

    srcR.x = 7;
    srcR.y = 0;
    desR.x = desR.y = 0;
    srcR.w = srcR.h = desR.w = desR.h = 1;

    for(i = 0; i < 8; i++) {
        srcR.x = 7;
        desR.x = 0;
        for(j = 0; j < 8; j++) {                        // 反序传输一行像素。
            SDL_BlitSurface(ori, &srcR, res, &desR);
            srcR.x--;
            desR.x++;
        }
        srcR.y++;
        desR.y++;
    }
}

static void Y_Flip_8_8(SDL_Surface *ori, SDL_Surface *res)
{
    static SDL_Rect srcR, desR;
    int i, j;

    srcR.x = 0;
    srcR.y = 7;
    desR.x = desR.y = 0;
    srcR.w = srcR.h = desR.w = desR.h = 1;

    for(i = 0; i < 8; i++) {
        srcR.x = 0;
        desR.x = 0;
        for(j = 0; j < 8; j++) {
            SDL_BlitSurface(ori, &srcR, res, &desR);
            srcR.x++;
            desR.x++;
        }
        srcR.y--;
        desR.y++;
    }
}


static void AddSpritesToWarppedScreen()
{
    static SDL_Rect srcR, desR;
    unsigned int oamNum;

    

    srcR.x = srcR.y = desR.x = desR.y = 0;
    
    srcR.x = srcR.y = 0;
    desR.x = 8; desR.y = 16;                                                // 留出空间给超出边界的 Sprite.
    srcR.w = srcR.h = desR.w = desR.h = 8*32;
    
    if( ! ( _gbMem[0xFF40] & MSK_1 ) )                  // If Sprite Display disabled.
        return;

    srcR.w = desR.w = srcR.h = desR.h = 8;

    /*if( _gbMem[0xFF40] & MSK_2 )          // 判断 Spire 是 8x8 还是 8x16 模式。
        srcR.h = desR.h = 16;
    else srcR.h = desR.h = 8;*/

    for(oamNum = 0xFE00; oamNum < 0xFE9F; oamNum += 4) {
        isX_Flip = 0;
        isY_Flip = 0;

        if( _gbMem[oamNum + 3] & MSK_5) {       // 是否需要水平翻转。
            isX_Flip = 1;
            srcR.x = 0;
            srcR.y = ( (unsigned int)(_gbMem[oamNum + 2]) ) << 3;
            desR.x = desR.y = 0;
            srcR.w = desR.w = srcR.h = desR.h = 8;
            SDL_BlitSurface(surTile, &srcR, surOri8_8, &desR);      // 构造一个 8*8 的原图像。
            X_Flip_8_8(surOri8_8, surRes8_8);                       // 水平翻转，结果保存在 surRes8_8 中。
        }

        if( _gbMem[oamNum + 3] & MSK_6) {
            isY_Flip = 1;
            if( isX_Flip ) {
                Y_Flip_8_8(surRes8_8, surOri8_8);                   // 若前面发生过水平翻转，则把上次结果进行垂直翻转后保存在 surOri8_8 中。
            } else {
                srcR.x = 0;
                srcR.y = ( (unsigned int)(_gbMem[oamNum + 2]) ) << 3;
                desR.x = desR.y = 0;
                srcR.w = desR.w = srcR.h = desR.h = 8;
                SDL_BlitSurface(surTile, &srcR, surOri8_8, &desR);      // 构造一个 8*8 的原图像。
                Y_Flip_8_8(surOri8_8, surRes8_8);
            }

        }

        srcR.w = desR.w = srcR.h = desR.h = 8;
        srcR.x = 0;
        //desR.y = _gbMem[oamNum + 0];      // Get OAM Y position.
        //desR.x = _gbMem[oamNum + 1];      // Get OAM X position.
        desR.y = _gbMem[oamNum + 0] + _gbMem[0xFF42];       // OAM-Y + SCY.
        desR.x = _gbMem[oamNum + 1] + _gbMem[0xFF43];       // OAM-X + SCX.
        
        SDL_SetColorKey(surTile, SDL_TRUE, SDL_MapRGB(surTile->format, debugBgp0[0], debugBgp0[0], debugBgp0[0]));
        SDL_SetColorKey(surOri8_8, SDL_TRUE, SDL_MapRGB(surOri8_8->format, debugBgp0[0], debugBgp0[0], debugBgp0[0]));
        SDL_SetColorKey(surRes8_8, SDL_TRUE, SDL_MapRGB(surRes8_8->format, debugBgp0[0], debugBgp0[0], debugBgp0[0]));

        switch( isX_Flip + isY_Flip ) {
            case 0: srcR.y = ( (unsigned int)(_gbMem[oamNum + 2]) ) << 3;
                    SDL_BlitSurface(surTile, &srcR, surInternalWarppedScreen, &desR);
                    break;      // 若没有发生任何翻转，则直接从 surTile 里传输 Tile.

            case 1: srcR.y = 0;
                    SDL_BlitSurface(surRes8_8, &srcR, surInternalWarppedScreen, &desR);
                    break;      // 若只发生过一次翻转（水平或垂直），则从 surRes8_8 里传输。
            
            case 2: srcR.y = 0;
                    SDL_BlitSurface(surOri8_8, &srcR, surInternalWarppedScreen, &desR);
                    break;      // 若都发生过水平和垂直翻转，则从 surOri8_8 里传输。
            
            default: break;
        }

        SDL_SetColorKey(surTile, SDL_FALSE, SDL_MapRGB(surTile->format, debugBgp0[0], debugBgp0[0], debugBgp0[0]));
        SDL_SetColorKey(surOri8_8, SDL_FALSE, SDL_MapRGB(surOri8_8->format, debugBgp0[0], debugBgp0[0], debugBgp0[0]));
        SDL_SetColorKey(surRes8_8, SDL_FALSE, SDL_MapRGB(surRes8_8->format, debugBgp0[0], debugBgp0[0], debugBgp0[0]));

    }       // for(oamNum = 0xFE00)


}

static void ConstructInternalWarppedScreen()
{
    static SDL_Rect srcR, desR;

    /* === Left upper 256*256 === */
    srcR.x = srcR.y = 0;
    desR.x = 8;
    desR.y = 16;
    srcR.w = srcR.h = desR.w = desR.h = 256;
    SDL_BlitSurface(surBgScreen, &srcR, surInternalWarppedScreen, &desR);

    /* === Right upper 160*256 === */
    srcR.x =  srcR.y = 0;
    desR.x = 256 + 8;
    desR.y = 16;
    srcR.w =  desR.w = 160;
    srcR.h = desR.h = 256;
    SDL_BlitSurface(surBgScreen, &srcR, surInternalWarppedScreen, &desR);

    /* === Left lower 256*144 === */
    srcR.x = srcR.y = 0;
    desR.x = 8;
    desR.y = 256 + 16;
    srcR.w = desR.w = 256;
    srcR.h = desR.h = 144;
    SDL_BlitSurface(surBgScreen, &srcR, surInternalWarppedScreen, &desR);

    /* === Right lower 160*144 === */
    srcR.x = srcR.y =0;
    desR.x = 256 + 8;
    desR.y = 256 + 16;
    srcR.w =  desR.w = 160;
    srcR.h = desR.h = 144;
    SDL_BlitSurface(surBgScreen, &srcR, surInternalWarppedScreen, &desR);
}

public void ConstructMainScreen()
{
    static SDL_Rect srcR, desR;

    srcR.x = _gbMem[0xFF43] + 8;        // Get SCX.
    srcR.y = _gbMem[0xFF42] + 16;       // Get SCY.
    srcR.w = desR.w = 160;
    srcR.h = desR.h = 144;
    desR.x = desR.y = 0;

    SDL_BlitSurface(surInternalWarppedScreen, &srcR, surMainScreen, &desR);
}

public void RefreshAllWindow()
{
    TranslateAllTile();                 // 必要步骤。
    UpdateAllTile();                    // 必要步骤。
    ArrangeTilesInSurface();            // 绘制 Tile 列表窗口，非必要步骤，可以不画。
    ConstructBackground();              // 必要步骤。        
    ConstructInternalWarppedScreen();   // 必要步骤。

    if( _gbMem[0xFF40] & MSK_5 )        // 必要步骤。
        AddGbWindowToWarppedScreen();
    
    if( _gbMem[0xFF40] & MSK_1 )        // 必要步骤。
        AddSpritesToWarppedScreen();
    
    ConstructMainScreen();              // 必要步骤。
    
    SDL_RenderClear(mainScreenRenderer);
    SDL_UpdateTexture(mainScreenTexture, NULL, surMainScreen -> pixels, 4*160);
    SDL_RenderCopy(mainScreenRenderer, mainScreenTexture, NULL, NULL);
    SDL_RenderPresent(mainScreenRenderer);

    if( isDebugGraphic ) {
        SDL_RenderClear(bgWindowRenderer);
        SDL_UpdateTexture(bgTexture, NULL, surBgScreen -> pixels, 4*8*32);
        SDL_RenderCopy(bgWindowRenderer, bgTexture, NULL, NULL);
        SDL_RenderPresent(bgWindowRenderer);
    
        SDL_RenderClear(tileWindowRenderer);
        SDL_UpdateTexture(tileTexture, NULL, surTileScreen -> pixels, 4*8*16);
        SDL_RenderCopy(tileWindowRenderer, tileTexture, NULL, NULL);
        SDL_RenderPresent(tileWindowRenderer);
    
        SDL_RenderClear(internalWarppedScreenRenderer);
        SDL_UpdateTexture(internalWarppedScreenTexture, NULL, surInternalWarppedScreen -> pixels, 4*(256+160+8));
        SDL_RenderCopy(internalWarppedScreenRenderer, internalWarppedScreenTexture, NULL, NULL);
        SDL_RenderPresent(internalWarppedScreenRenderer);
    }

    SDL_Delay(10);      // 测试时用较小值，加快速度。
}


static void FreeSurface(SDL_Surface **p)
{
    if( *p = NULL )
        return;
    
    SDL_FreeSurface(*p);
    *p = NULL;
}

static void FreeTexture(SDL_Texture **p)
{
    if( *p = NULL )
        return;
    
    SDL_DestroyTexture(*p);
    *p = NULL;
}

static void FreeRenderer(SDL_Renderer **p)
{
    if( *p = NULL )
        return;
    
    SDL_DestroyRenderer(*p);
    *p = NULL;
}

static void FreeWindow(SDL_Window **p)
{
    if( *p = NULL )
        return;
    
    SDL_DestroyWindow(*p);
    *p = NULL;
}

public void CleanUpGraphic()
{
    FreeSurface(&surOri8_8);
    FreeSurface(&surOri8_16);
    FreeSurface(&surRes8_8);
    FreeSurface(&surRes8_16);
    FreeSurface(&surTile);
    FreeSurface(&surTileScreen);
    FreeSurface(&surBgScreen);
    FreeSurface(&surGbWindow);
    FreeSurface(&surInternalWarppedScreen);
    FreeSurface(&surMainScreen);

    FreeTexture(&tileTexture);
    FreeTexture(&bgTexture);
    FreeTexture(&internalWarppedScreenTexture);
    FreeTexture(&mainScreenTexture);

    FreeRenderer(&tileWindowRenderer);
    FreeRenderer(&bgWindowRenderer);
    FreeRenderer(&internalWarppedScreenRenderer);
    FreeRenderer(&mainScreenRenderer);

    FreeWindow(&tileWindow);
    FreeWindow(&bgWindow);
    FreeWindow(&internalWarppedScreenWindow);
    FreeWindow(&mainScreenWindow);

    //fprintf(stderr, "CleanUpGraphic()\n");    // DEBUG.
}
