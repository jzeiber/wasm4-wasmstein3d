#pragma once

#include "wasm4.h"
#include "palette.h"
#include "Platform.h"

#include "wasmstring.h"

extern int y_lut[];

inline uint8_t* GetScreenBuffer() { return FRAMEBUFFER; }

[[gnu::always_inline]]
inline void drawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    uint16_t savecolor=*DRAW_COLORS;
    *DRAW_COLORS=color;
    line(x,y,x,y);
    *DRAW_COLORS=savecolor;
}

[[gnu::always_inline]]
inline void setPixel(uint8_t x, uint8_t y)
{
	 drawPixel(x, y, PALETTE_BLACK);
}

[[gnu::always_inline]]
inline void clearPixel(uint8_t x, uint8_t y)
{
	 drawPixel(x, y, PALETTE_WHITE);
}


inline void clearDisplay(uint8_t colour)
{
    const uint16_t savecolor=*DRAW_COLORS;
    *DRAW_COLORS=colour << 4 | colour;
    rect(0,0,SCREEN_SIZE,SCREEN_SIZE);
    *DRAW_COLORS=savecolor;
}

class WASM4Platform : public PlatformBase
{
public:
	void playSound(uint8_t id);

	void update();
};

void ERROR(const char* msg);

extern WASM4Platform Platform;

inline void diskRead2BPP(uint8_t *buffer, const uint8_t *sheet, int32_t bppoffset, int length)
{
    int32_t bytepos=(bppoffset/4);
    int32_t bitpos=(bppoffset%4)*2;
    int32_t bufferpos=0;

    memset(buffer,0,length);

    for(int i=0; i<length; i++)
    {
        buffer[bufferpos]|=((sheet[bytepos] & (0b00000011 << (6-bitpos))) >> (6-bitpos));
        bufferpos++;
        bitpos+=2;
        if(bitpos==8)
        {
            bytepos++;
            bitpos=0;
        }
    }
}

inline void writeSaveFile(uint8_t* buffer, int length)
{
    diskw(buffer,length);
}

inline bool readSaveFile(uint8_t* buffer, int length)
{
    uint32_t r=diskr(buffer,length);
    return r==length;
}
