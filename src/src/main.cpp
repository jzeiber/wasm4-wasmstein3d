#include "wasm4.h"

#include "Engine.h"
#include "Generated/fxdata.h"
#include "WASM4Platform.h"
#include "Generated/Data_Audio.h"
#include "Sounds.h"

//uint16_t audioBuffer[32];
//bool truefunc() { return true; }

//unsigned long lastTimingSample;

void WASM4Platform::playSound(uint8_t id)
{

  if(!Platform.isMuted())
  {
    if(id==SHOOTSND || id==ATKPISTOLSND || id==ATKMACHINEGUNSND || id==ATKGATLINGSND)
    {
      //void tone (uint32_t frequency, uint32_t duration, uint32_t volume, uint32_t flags);
      // adsr - 0,2,2,8 ( in adrs bit order )
      tone(220, (1 << 24) | (5 << 16) | (10 << 8) | 1, (100 << 8) | 30, TONE_NOISE);
    }
    if(id==NAZIFIRESND || id==BOSSFIRESND || id==SSFIRESND)
    {
      tone(240, (1 << 24) | (5 << 16) | (10 << 8) | 1, (50 << 8) | 15, TONE_NOISE);
    }
    if(id==DOGATTACKSND)
    {
      tone(100, (6 << 24) | (0 << 16) | (12 << 8) << 0, 50, TONE_PULSE1);
    }
    if(id==DOGBARKSND)
    {
      tone((200 << 16) | 120, (2 << 24) | (0 << 16) | (4 << 8) | 0, 50, TONE_PULSE2 | TONE_MODE1);
    }
    if(id==OPENDOORSND || id==CLOSEDOORSND)
    {
      tone(240, (12 << 24) | (0 << 16) | (12 << 8) | 15, 50, TONE_NOISE);
    }
    if(id==PUSHWALLSND)
    {
      tone(200, (12 << 24) | (0 << 16) | (12 << 8) | 130, 50, TONE_NOISE);
    }
    if(id==GETAMMOSND || id==GETKEYSND || id==GETMACHINESND || id==GETGATLINGSND || id==HEALTH1SND || id==HEALTH2SND || id==BONUS1SND || id==BONUS2SND || id==BONUS3SND || id==BONUS4SND || id==BONUS1UPSND)
    {
      tone((720 << 16) | 560, (16 << 24) | (4 << 16) | (4 << 8) << 0, 50, TONE_PULSE1 | TONE_MODE2);
    }

  }
}

//debug
/*
#include "TileTypes.h"
void setmaptile(const int32_t x, const int32_t y, const uint8_t tiletype, const uint8_t flags=0)
{
  MapData_data[(y*MAP_SIZE*2)+(x*2)]=tiletype;
  MapData_data[(y*MAP_SIZE*2)+(x*2)+1]=flags;
}
*/

void start() {

  PALETTE[0]=0x000000;
  PALETTE[1]=0x555555;
  PALETTE[2]=0xaaaaaa;
  PALETTE[3]=0xffffff;

  engine.init();

  /*
  memset(MapData_data,0,MAP_SIZE*MAP_SIZE*2);

  for(int y=0; y<MAP_SIZE; y++)
  {
    for(int x=0; x<MAP_SIZE; x++)
    {
      setmaptile(x,y,Tile_Wall01,0);
    }
  }

  for(int y=2; y<10; y++)
  {
    for(int x=2; x<10; x++)
    {
      setmaptile(x,y,Tile_Empty);
    }
  }
  int itemidx=0;

  setmaptile(5,5,Tile_PlayerStart_East);
  setmaptile(7,3,Tile_Item_Key1,itemidx++);
  setmaptile(10,8,Tile_Door_Locked1_Vertical);
  setmaptile(11,8,Tile_Empty);
  setmaptile(12,8,Tile_Door_Locked2_Vertical);
  setmaptile(8,5,Tile_Item_Key2,itemidx++);
  setmaptile(9,9,Tile_Item_Clip,itemidx++);
  setmaptile(8,9,Tile_Item_Clip,itemidx++);
  setmaptile(3,10,Tile_SecretPushWall,Tile_Wall04);
  setmaptile(3,11,Tile_Empty);
  setmaptile(3,12,Tile_Empty);
  setmaptile(3,13,Tile_Empty);
  setmaptile(3,14,Tile_Empty);

  for(int i=13; i<60; i++)
  {
    setmaptile(i,8,Tile_Empty);
  }

  */

}

void update() {

  static float tickAccum = 0;
 
  Platform.update();

  tickAccum += 1000. / 60.;
  constexpr float frameDuration = 1000 / TARGET_FRAMERATE;
  while(tickAccum > frameDuration)
  {
    engine.update();
	  tickAccum -= frameDuration;
  }
	
  engine.draw();

}
