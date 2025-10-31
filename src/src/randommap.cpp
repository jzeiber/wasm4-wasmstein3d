#include "randommap.h"
#include "Defines.h"
#include "TileTypes.h"
#include "wasmstring.h"
#include "randommt.h"
#include "cppfuncs.h"

enum Dir
{
    DIR_NORTH=1,
    DIR_EAST=2,
    DIR_SOUTH=3,
    DIR_WEST=4
};

struct RoomTheme
{
	uint8_t mainwall[2];
	uint8_t decorativewall[3];
	uint8_t secretwall[4];		// which walls can have secret behind (may be same as above)
	uint8_t roomdecoration[8];
    float decorationmult;

    int32_t MainWallCount() const
    {
        return Count(mainwall,countof(mainwall));
    }

    int32_t DecorativeWallCount() const
    {
        return Count(decorativewall,countof(decorativewall));
    }

    int32_t SecretWallCount() const
    {
        return Count(secretwall,countof(secretwall));
    }

    int32_t DecorationCount() const
    {
        return Count(roomdecoration,countof(roomdecoration));
    }

private:
    int32_t Count(const uint8_t* vals, int32_t maxcount) const
    {
        int32_t count=0;
        while(vals[count]!=Tile_Empty && count<maxcount)
        {
            count++;
        }
        return count;
    }
};

/*
	// Blocking decoration
	Tile_FirstBlockingDecoration,
	Tile_BlockingDecoration_Table = Tile_FirstBlockingDecoration,
	Tile_BlockingDecoration_TableChairs,
	Tile_BlockingDecoration_FloorLamp,
	Tile_BlockingDecoration_Barrel,
	Tile_BlockingDecoration_Pillar,
	Tile_BlockingDecoration_Vase,
	Tile_BlockingDecoration_Tree,
	Tile_BlockingDecoration_HangingSkeleton,
	Tile_BlockingDecoration_Plant,
	Tile_BlockingDecoration_Sink,
	//Tile_BlockingDecoration_Well,
	Tile_BlockingDecoration_SuitOfArmour,
	Tile_LastBlockingDecoration = Tile_BlockingDecoration_SuitOfArmour,

	// Non blocking decoration
	Tile_FirstDecoration,
	Tile_Decoration_OverheadLamp = Tile_FirstDecoration,
	Tile_Decoration_DeadGuard,
	Tile_Decoration_Skeleton,
	Tile_Decoration_KitchenStuff,
	Tile_Decoration_Chandelier,
	Tile_LastDecoration = Tile_Decoration_Chandelier,
*/

RoomTheme roomtheme[]={
//                              Main Wall Texture (2)           Decorative Wall Texture (3)                 Secret Wall Texture (4)                                 Decorations (8)
/* White Stone */               {{Tile_Wall01,Tile_Wall02},     {Tile_Wall03,Tile_Wall04,Tile_Wall06},      {Tile_Wall02,Tile_Wall03,Tile_Wall04,Tile_Wall06},      {Tile_Decoration_OverheadLamp,Tile_BlockingDecoration_Pillar,Tile_Empty},                                                               0.2},
/* Dark Rectangular Blocks */   {{Tile_Wall08,Tile_Wall09},     {Tile_Wall05,Tile_Wall07,Tile_Empty},       {Tile_Wall08,Tile_Wall09,Tile_Empty},                   {Tile_Decoration_DeadGuard,Tile_Decoration_Skeleton,Tile_Decoration_OverheadLamp,Tile_BlockingDecoration_Barrel,Tile_Empty},            0.1},
/* Wood Paneling */             {{Tile_Wall12,Tile_Empty},      {Tile_Wall10,Tile_Wall11,Tile_Empty},       {Tile_Wall10,Tile_Wall11,Tile_Empty},                   {Tile_Decoration_Chandelier,Tile_BlockingDecoration_SuitOfArmour,Tile_BlockingDecoration_Plant,Tile_BlockingDecoration_Vase,Tile_BlockingDecoration_FloorLamp,Tile_BlockingDecoration_TableChairs,Tile_BlockingDecoration_Table,Tile_Empty},     0.2}
};

enum BuildFlags
{
    BUILD_FLAG_STARTINGROOM=    0b00000001,
    BUILD_FLAG_BOSSROOM=        0b00000010,
    BUILD_FLAG_PLACEDKEY1=      0b00000100,
    BUILD_FLAG_PLACEDKEY2=      0b00001000,
    BUILD_FLAG_PLACEDSECRET1=   0b00010000,
    BUILD_FLAG_PLACEDSECRET2=   0b00100000,
    BUILD_FLAG_PLACEDBOSS=      0b01000000,
    BUILD_FLAG_PLACEDEXIT=      0b10000000,
};

struct BuildPosition
{
    uint8_t dir:4;
    uint8_t flags:4;
    uint8_t theme:4;
    uint8_t prevroomtheme:4;
    uint8_t x;
    uint8_t y;
    uint8_t prevroomx;
    uint8_t prevroomy;
    uint8_t startdistance;
};

void setMapTile(uint8_t *mapdata, const int32_t x, const int32_t y, const uint8_t tiletype, const uint8_t flags=0)
{
    mapdata[(y*MAP_SIZE*2)+(x*2)]=tiletype;
    mapdata[(y*MAP_SIZE*2)+(x*2)+1]=flags;
}

void removeFirstNode(BuildPosition nodes[], int maxnodes)
{
    for(int i=0; i<maxnodes-1; i++)
    {
        nodes[i]=nodes[i+1];
    }
    memset(&nodes[maxnodes-1],0,sizeof(BuildPosition));
}

bool checkRoomFits(uint8_t *mapdata, const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2)
{
    if(x1>x2 || y1>y2 || x1<0 || y1<0 || x2>=MAP_SIZE || y2>=MAP_SIZE)
    {
        return false;
    }
    for(int32_t y=y1; y<=y2; y++)
    {
        for(int32_t x=x1; x<=x2; x++)
        {
            if(mapdata[(y*MAP_SIZE*2)+(x*2)]!=255)
            {
                return false;
            }
        }
    }
    return true;
}

void replaceTile(uint8_t *mapdata, uint8_t tile, uint8_t newtile, uint8_t newmetadata)
{
    for(int32_t y=0; y<MAP_SIZE; y++)
    {
        for(int32_t x=0; x<MAP_SIZE; x++)
        {
            if(mapdata[(y*MAP_SIZE*2)+(x*2)]==tile)
            {
                setMapTile(mapdata,x,y,newtile,newmetadata);
            }
        }
    }
}

// makes sure doorway is surrounded by plain walls
void fixRoomDoorway(RandomMT &rnd, uint8_t *mapdata, const int32_t dx, const int32_t dy, const uint8_t theme, const uint8_t dir, const bool enlarge)
{
    int32_t x1=(dir==DIR_NORTH || dir==DIR_SOUTH) ? dx-(enlarge ? 2 : 1) : dx;
    int32_t x2=(dir==DIR_NORTH || dir==DIR_SOUTH) ? dx+(enlarge ? 2 : 1) : dx;
    int32_t y1=(dir==DIR_EAST || dir==DIR_WEST) ? dy-(enlarge ? 2 : 1) : dy;
    int32_t y2=(dir==DIR_EAST || dir==DIR_WEST) ? dy+(enlarge ? 2 : 1) : dy;

    for(int32_t y=y1; y<=y2; y++)
    {
        for(int32_t x=x1; x<=x2; x++)
        {
            if(x!=dx || y!=dy)
            {
                setMapTile(mapdata,x,y,roomtheme[theme].mainwall[rnd.Next()%roomtheme[theme].MainWallCount()],0);
                if(enlarge && ((x>x1 && x<x2) || (y>y1 && y<y2)))
                {
                    setMapTile(mapdata,x,y,Tile_Empty,0);
                }
            }
        }
    }
}

void placeWall(uint8_t *mapdata, RandomMT &rnd, const RoomTheme &theme, int32_t x1, int32_t y1, int32_t x2, int32_t y2, bool decorativewalls)
{
    bool prevdecor=false;
    for(int32_t y=y1; y<=y2; y++)
    {
        for(int32_t x=x1; x<=x2; x++)
        {
            if(mapdata[(y*MAP_SIZE*2)+(x*2)]==255)
            {
                setMapTile(mapdata,x,y,theme.mainwall[rnd.Next()%theme.MainWallCount()],0);
                if(decorativewalls==true && prevdecor==false && x>x1+1 && x<x2-1 && rnd.NextDouble()>0.8)
                {
                    setMapTile(mapdata,x,y,theme.decorativewall[rnd.Next()%theme.DecorativeWallCount()],0);
                    prevdecor=true;
                }
            }
        }
    }
}

void placeEnemies(uint8_t *mapdata, RandomMT &rnd, int32_t x1, int32_t y1, int32_t x2, int32_t y2, const float mult)
{
    int32_t tries=(((y2-y1)*(x2-x1))/20)*mult;
    for(int32_t i=0; i<tries; i++)
    {
        int32_t x=x1+1+(rnd.Next()%(x2-x1-1));
        int32_t y=y1+1+(rnd.Next()%(y2-y1-1));
        if(mapdata[(y*MAP_SIZE*2)+(x*2)]==Tile_Empty)
        {
            uint8_t diff=rnd.Next()%3;
            uint8_t type=Tile_Actor_Dog_Easy+diff;
            if(rnd.NextDouble()>=0.5)
            {
                type=Tile_Actor_Guard_Easy+diff;
            }
            else if(rnd.NextDouble()>=0.8)
            {
                type=Tile_Actor_SS_Easy+diff;
            }
            setMapTile(mapdata,x,y,type,0);
        }   
    }
}

void placeDecorations(uint8_t *mapdata, RandomMT &rnd, const RoomTheme &theme, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    int32_t tries=((y2-y1)*(x2-x1))*theme.decorationmult;
    for(int32_t i=0; i<tries; i++)
    {
        uint8_t type=theme.roomdecoration[rnd.Next()%theme.DecorationCount()];
        bool place=true;
        int32_t x=x1+1+(rnd.Next()%(x2-x1-1));
        int32_t y=y1+1+(rnd.Next()%(y2-y1-1));
        // if type is blocking, them must be next to wall and not next to another blocking or door
        if(type>=Tile_FirstBlockingDecoration && type<=Tile_LastBlockingDecoration)
        {
            bool foundwall=false;
            for(int32_t yy=y-1; yy<=y+1; yy++)
            {
                for(int32_t xx=x-1; xx<=x+1; xx++)
                {
                    if(xx>=0 && xx<MAP_SIZE && yy>=0 && yy<MAP_SIZE)
                    {
                        const uint8_t tile=mapdata[(yy*MAP_SIZE*2)+(xx*2)];
                        if((tile>=Tile_FirstBlockingDecoration && tile<=Tile_LastBlockingDecoration) || (tile>=Tile_FirstDoor && tile<=Tile_LastDoor) || tile==Tile_SecretPushWall)
                        {
                            place=false;
                        }
                        if(tile>=Tile_FirstWall && tile<=Tile_LastWall)
                        {
                            foundwall=true;
                        }
                    }
                }
            }
            if(foundwall==false)
            {
                place=false;
            }
        }
        if(place==true && mapdata[(y*MAP_SIZE*2)+(x*2)]==Tile_Empty)
        {
            setMapTile(mapdata,x,y,type,0);
        }
    }
}

bool tryPlaceRoom(uint8_t *mapdata, RandomMT &rnd, BuildPosition &pos, uint8_t &buildflags, int32_t &doorcount, BuildPosition *nodes, int32_t &nodecount, const int32_t maxnodes)
{
    const RoomTheme &theme=roomtheme[pos.theme];

    int32_t w=5+(rnd.Next()%(!(pos.flags & BUILD_FLAG_STARTINGROOM) ? 10 : 3));
    int32_t h=5+(rnd.Next()%(!(pos.flags & BUILD_FLAG_STARTINGROOM) ? 10 : 3));
    if(pos.flags & BUILD_FLAG_BOSSROOM)
    {
        w=15+(rnd.Next()%4);
        h=15+(rnd.Next()%4);
    }
    const uint8_t dir=pos.dir;

    int32_t x1=(dir==DIR_NORTH || dir==DIR_SOUTH ? pos.x-(w/4)-(rnd.Next()%(w/2))-1 : (dir==DIR_EAST ? pos.x : pos.x-w-1));
    int32_t x2=x1+w+1;
    int32_t y1=(dir==DIR_EAST || dir==DIR_WEST ? pos.y-(h/4)-(rnd.Next()%(h/2))-1 : (dir==DIR_SOUTH ? pos.y : pos.y-h-1));
    int32_t y2=y1+h+1;

    if(checkRoomFits(mapdata,x1,y1,x2,y2)==true)
    {

        // clear interior and place a few items
        for(int32_t y=y1+1; y<y2; y++)
        {
            for(int32_t x=x1+1; x<x2; x++)
            {
                uint8_t item=Tile_Empty;
                // don't place items on door axis
                if(x!=pos.prevroomx && y!=pos.prevroomy)
                {
                    if(rnd.NextDouble()>0.99)
                    {
                        item=Tile_Item_Clip;
                    }
                    else if(rnd.NextDouble()>0.99)
                    {
                        item=Tile_Item_Food;
                    }
                }
                setMapTile(mapdata,x,y,item,0);
            }
        }

        // place walls
        placeWall(mapdata,rnd,theme,x1,y1,x2,y1,true);
        placeWall(mapdata,rnd,theme,x1,y2,x2,y2,true);
        placeWall(mapdata,rnd,theme,x1,y1+1,x1,y2-1,true);
        placeWall(mapdata,rnd,theme,x2,y1+1,x2,y2-1,true);

        // place doors and doorways
        if(pos.prevroomx>0 && pos.prevroomx<MAP_SIZE-1 && pos.prevroomy>0 && pos.prevroomy<MAP_SIZE-1)
        {
            bool expandthis=false;
            bool expandprev=false;
            bool doorthis=true;
            bool doorprev=false;
            uint8_t door1metadata=0;
            uint8_t door2metadata=0;
            uint8_t doortype=(pos.dir==DIR_NORTH || pos.dir==DIR_SOUTH) ? Tile_Door_Generic_Horizontal : Tile_Door_Generic_Vertical;

            if(pos.theme==pos.prevroomtheme && rnd.NextDouble()>0.5)
            {
                expandthis=true;
                expandprev=true;
                doorthis=false;
            }
            else if(rnd.NextDouble()>0.5)
            {
                expandthis=false;
                expandprev=true;
            }
            else
            {
                expandthis=true;
                expandprev=false;
                doorthis=false;
                doorprev=true;
            }

            if(doorcount>MAX_DOORS)
            {
                doorthis=false;
                doorprev=false;
            }

            if(doorthis || doorprev)
            {
                doorcount++;
            }

            if((doorthis || doorprev) && ((buildflags & BUILD_FLAG_PLACEDBOSS) || ((buildflags & BUILD_FLAG_PLACEDKEY1) && !(buildflags & BUILD_FLAG_PLACEDSECRET1) && rnd.NextDouble()>0.75)))
            {
                doortype=(doortype==Tile_Door_Generic_Horizontal) ? Tile_Door_Locked1_Horizontal : Tile_Door_Locked1_Vertical;
                buildflags|=BUILD_FLAG_PLACEDSECRET1;
            }
            else if((doorthis || doorprev) && (buildflags & BUILD_FLAG_PLACEDKEY2) && !(buildflags & BUILD_FLAG_PLACEDSECRET2) && rnd.NextDouble()>0.75)
            {
                doortype=(doortype==Tile_Door_Generic_Horizontal) ? Tile_Door_Locked2_Horizontal : Tile_Door_Locked2_Vertical;
                buildflags|=BUILD_FLAG_PLACEDSECRET2;
            }

            // add doorway - (or just empty tile)
            setMapTile(mapdata,pos.x,pos.y,(doorthis==true ? doortype : Tile_Empty),door1metadata);
            setMapTile(mapdata,pos.prevroomx,pos.prevroomy,(doorprev==true ? doortype : Tile_Empty),door2metadata);

            fixRoomDoorway(rnd,mapdata,pos.x,pos.y,pos.theme,pos.dir,expandthis);
            fixRoomDoorway(rnd,mapdata,pos.prevroomx,pos.prevroomy,pos.prevroomtheme,pos.dir,expandprev);
        }

        if(pos.flags & BUILD_FLAG_STARTINGROOM)
        {
            setMapTile(mapdata,x1+w/2,y1+h/2,(rnd.Next()%4)+Tile_PlayerStart_North,0);
        }
        else if(pos.flags & BUILD_FLAG_BOSSROOM)
        {
            // TODO - place boss in center and add columns for cover
            setMapTile(mapdata,x1+w/2,y1+h/2,Tile_Actor_Boss,0);
            buildflags|=BUILD_FLAG_PLACEDBOSS;

            setMapTile(mapdata,x1+w/2+1,y1+h/2+1,Tile_Item_ChainGun,0);
            setMapTile(mapdata,x1+1,y1+1,Tile_Item_FirstAid);
            setMapTile(mapdata,x2-1,y2-1,Tile_Item_FirstAid);

            for(int32_t yy=0; yy<3; yy++)
            {
                for(int32_t xx=0; xx<3; xx++)
                {
                    setMapTile(mapdata,x1+3+xx,y1+3+yy,theme.mainwall[0],0);
                    setMapTile(mapdata,x1+3+xx,y2-5+yy,theme.mainwall[0],0);
                    setMapTile(mapdata,x2-5+xx,y1+3+yy,theme.mainwall[0],0);
                    setMapTile(mapdata,x2-5+xx,y2-5+yy,theme.mainwall[0],0);
                }
            }

        }
        else
        {
            placeEnemies(mapdata,rnd,x1,y1,x2,y2,(pos.startdistance<2) ? 0.5 : 1.0);
        }

        if(!(pos.flags & BUILD_FLAG_BOSSROOM))
        {
            placeDecorations(mapdata,rnd,theme,x1,y1,x2,y2);
        }

        // add new build nodes to try to place new rooms
        if(w>3)
        {
            if(nodecount<maxnodes && rnd.NextDouble()<0.75)
            {
                nodes[nodecount].dir=DIR_NORTH;
                nodes[nodecount].x=x1+(w/2)+(rnd.Next()%(w/2));
                nodes[nodecount].y=y1-1;
                nodes[nodecount].theme=(rnd.Next()%countof(roomtheme));
                nodes[nodecount].prevroomx=nodes[nodecount].x;
                nodes[nodecount].prevroomy=y1;
                nodes[nodecount].prevroomtheme=nodes[0].theme;
                nodes[nodecount].startdistance=nodes[0].startdistance+1;
                nodecount++;
            }
            if(nodecount<maxnodes && rnd.NextDouble()<0.75)
            {
                nodes[nodecount].dir=DIR_SOUTH;
                nodes[nodecount].x=x1+(w/2)+(rnd.Next()%(w/2));
                nodes[nodecount].y=y2+1;
                nodes[nodecount].theme=(rnd.Next()%countof(roomtheme));
                nodes[nodecount].prevroomx=nodes[nodecount].x;
                nodes[nodecount].prevroomy=y2;
                nodes[nodecount].prevroomtheme=nodes[0].theme;
                nodes[nodecount].startdistance=nodes[0].startdistance+1;
                nodecount++;
            }
        }
        if(h>3)
        {
            if(nodecount<maxnodes && rnd.NextDouble()<0.75)
            {
                nodes[nodecount].dir=DIR_EAST;
                nodes[nodecount].x=x2+1;
                nodes[nodecount].y=y1+(h/2)+(rnd.Next()%(h/2));
                nodes[nodecount].theme=(rnd.Next()%countof(roomtheme));
                nodes[nodecount].prevroomx=x2;
                nodes[nodecount].prevroomy=nodes[nodecount].y;
                nodes[nodecount].prevroomtheme=nodes[0].theme;
                nodes[nodecount].startdistance=nodes[0].startdistance+1;
                nodecount++;
            }
            if(nodecount<maxnodes && rnd.NextDouble()<0.75)
            {
                nodes[nodecount].dir=DIR_WEST;
                nodes[nodecount].x=x1-1;
                nodes[nodecount].y=y1+(h/2)+(rnd.Next()%(h/2));
                nodes[nodecount].theme=(rnd.Next()%countof(roomtheme));
                nodes[nodecount].prevroomx=x1;
                nodes[nodecount].prevroomy=nodes[nodecount].y;
                nodes[nodecount].prevroomtheme=nodes[0].theme;
                nodes[nodecount].startdistance=nodes[0].startdistance+1;
                nodecount++;
            }
        }

        return true;
    }

    return false;
}

bool tryPlaceSecretRoom(uint8_t *mapdata, RandomMT &rnd, BuildPosition &pos, uint8_t &buildflags, int32_t &doorcount)
{
    if(doorcount>=MAX_DOORS)
    {
        return false;
    }

    const RoomTheme &theme=roomtheme[pos.prevroomtheme];
    const uint8_t dir=pos.dir;
    int32_t w=3;
    int32_t h=3;

    if(dir==DIR_NORTH || dir==DIR_SOUTH)
    {
        w++;
    }
    else
    {
        h++;
    }

    int32_t x1=(dir==DIR_NORTH || dir==DIR_SOUTH ? pos.x+(rnd.NextDouble()>0.5 ? -1 : -w) : (dir==DIR_EAST ? pos.x : pos.x-w-1));
    int32_t x2=x1+w+1;
    int32_t y1=(dir==DIR_EAST || dir==DIR_WEST ? pos.y+(rnd.NextDouble()>0.5 ? -1 : -h) : (dir==DIR_SOUTH ? pos.y : pos.y-h-1));
    int32_t y2=y1+h+1;

    if(checkRoomFits(mapdata,x1,y1,x2,y2)==true)
    {
        bool placedkey=false;   // make sure only 1 key gets placed in each secret room
        int32_t bsx=pos.x;
        int32_t bsy=pos.y;

        switch(dir)
        {
            case DIR_NORTH:
                y1++;
                y2++;
                bsy-=(h-1);
                break;
            case DIR_EAST:
                x1--;
                x2--;
                bsx+=(w-1);
                break;
            case DIR_SOUTH:
                y1--;
                y2--;
                bsy+=(h-1);
                break;
            case DIR_WEST:
                x1++;
                x2++;
                bsx-=(w-1);
                break;
        }

        // clear interior and place items
        for(int32_t y=y1+1; y<y2; y++)
        {
            for(int32_t x=x1+1; x<x2; x++)
            {
                uint8_t item=Tile_Empty;
                // don't place items on door axis
                if(x!=pos.prevroomx && y!=pos.prevroomy)
                {
                    if(!(buildflags & BUILD_FLAG_PLACEDKEY1) && rnd.NextDouble()>0.8)
                    {
                        item=Tile_Item_Key1;
                        buildflags|=BUILD_FLAG_PLACEDKEY1;
                        placedkey=true;
                    }
                    else if(placedkey==false && (buildflags & BUILD_FLAG_PLACEDKEY1) && !(buildflags & BUILD_FLAG_PLACEDKEY2) && rnd.NextDouble()>0.8)
                    {
                        item=Tile_Item_Key2;
                        buildflags|=BUILD_FLAG_PLACEDKEY2;
                    }
                    else if(rnd.NextDouble()>0.7)
                    {
                        item=Tile_FirstTreasure+(rnd.Next()%(Tile_LastTreasure-Tile_FirstTreasure));
                    }
                    else if(rnd.NextDouble()>0.99)
                    {
                        item=Tile_Item_1UP;
                    }
                    else if(rnd.NextDouble()>0.8)
                    {
                        item=Tile_FirstItem+(rnd.Next()%(Tile_FirstTreasure-Tile_FirstItem));
                    }
                    else if(rnd.NextDouble()>0.5)
                    {
                        item=Tile_Item_Clip;
                    }
                }
                setMapTile(mapdata,x,y,item,0);
            }
        }

        // place walls
        placeWall(mapdata,rnd,theme,x1,y1,x2,y1,false);
        placeWall(mapdata,rnd,theme,x1,y2,x2,y2,false);
        placeWall(mapdata,rnd,theme,x1,y1+1,x1,y2-1,false);
        placeWall(mapdata,rnd,theme,x2,y1+1,x2,y2-1,false);

        // backstop for push wall
        setMapTile(mapdata,bsx,bsy,theme.mainwall[rnd.Next()%theme.MainWallCount()],0);

        // place secret push wall
        theme.secretwall[rnd.Next()%theme.SecretWallCount()];
        setMapTile(mapdata,pos.prevroomx,pos.prevroomy,Tile_SecretPushWall,theme.secretwall[rnd.Next()%theme.SecretWallCount()]);

        fixRoomDoorway(rnd,mapdata,pos.prevroomx,pos.prevroomy,pos.prevroomtheme,pos.dir,false);

        return true;
    }

    return false;
}

bool tryPlaceExitRoom(uint8_t *mapdata, RandomMT &rnd, BuildPosition &pos, uint8_t &buildflags, int32_t &doorcount)
{
    if(doorcount>=MAX_DOORS)
    {
        return false;
    }

    const uint8_t dir=pos.dir;
    int32_t w=1;
    int32_t h=1;

    int32_t x1=(dir==DIR_NORTH || dir==DIR_SOUTH ? pos.x-1 : (dir==DIR_EAST ? pos.x : pos.x-w-1));
    int32_t x2=x1+w+1;
    int32_t y1=(dir==DIR_EAST || dir==DIR_WEST ? pos.y-1 : (dir==DIR_SOUTH ? pos.y : pos.y-h-1));
    int32_t y2=y1+h+1;

    if(checkRoomFits(mapdata,x1,y1,x2,y2)==true)
    {

        int32_t exitx=pos.x;
        int32_t exity=pos.y;

        switch(dir)
        {
            case DIR_NORTH:
                y1++;
                y2++;
                exity=y1;
                break;
            case DIR_EAST:
                x1--;
                x2--;
                exitx=x2;
                break;
            case DIR_SOUTH:
                y1--;
                y2--;
                exity=y2;
                break;
            case DIR_WEST:
                x1++;
                x2++;
                exitx=x1;
                break;
        }

        // center of elevator
        setMapTile(mapdata,x1+1,y1+1,Tile_Empty,0);

        // exit
        setMapTile(mapdata,exitx,exity,Tile_ExitSwitchWall,0);

        // elevator walls
        if(dir==DIR_NORTH || dir==DIR_SOUTH)
        {
            setMapTile(mapdata,x1,y1+1,Tile_Wall22,0);
            setMapTile(mapdata,x2,y1+1,Tile_Wall22,0);
        }
        else
        {
            setMapTile(mapdata,x1+1,y1,Tile_Wall22,0);
            setMapTile(mapdata,x1+1,y2,Tile_Wall22,0);
        }

        // elevator door
        setMapTile(mapdata,pos.prevroomx,pos.prevroomy,(dir==DIR_NORTH || dir==DIR_SOUTH) ? Tile_Door_Elevator_Horizontal : Tile_Door_Elevator_Vertical,0);

        fixRoomDoorway(rnd,mapdata,pos.prevroomx,pos.prevroomy,pos.prevroomtheme,pos.dir,false);

        return true;
    }

    return false;
}

void generateRandomMap(uint8_t *mapdata, const int32_t seed, int8_t currentlevel)
{
    RandomMT rnd=RandomMT::Instance();
    rnd.Seed(seed);

    int32_t roomcount=0;
    uint8_t buildflags=0;

    do
    {

        for(int y=0; y<MAP_SIZE; y++)
        {
            for(int x=0; x<MAP_SIZE; x++)
            {
                setMapTile(mapdata,x,y,255,0);
            }
        }

        BuildPosition nodes[32];
        memset(nodes,0,sizeof(BuildPosition)*16);
        int32_t nodecount=1;

        nodes[0].dir=1+(rnd.Next()%4);
        nodes[0].flags=BUILD_FLAG_STARTINGROOM;
        nodes[0].theme=(rnd.Next()%countof(roomtheme));
        nodes[0].x=MAP_SIZE/4+(rnd.Next()%(MAP_SIZE/2));
        nodes[0].y=MAP_SIZE/4+(rnd.Next()%(MAP_SIZE/2));
        nodes[0].prevroomtheme=0;
        nodes[0].prevroomx=255;
        nodes[0].prevroomy=255;
        nodes[0].startdistance=0;

        bool placedkey1=false;
        bool placedkey2=false;
        bool placedboss=false;
        bool placedexit=false;

        roomcount=0;
        buildflags=0;
        int32_t doorcount=0;

        while(nodecount>0)
        {
            int32_t tries=0;
            bool placedroom=false;
            const bool placeboss=(currentlevel==9 && !(buildflags & BUILD_FLAG_PLACEDBOSS) && roomcount>3 && rnd.NextDouble()>0.5);
            const bool placeexit=((currentlevel<9 || (buildflags & BUILD_FLAG_PLACEDBOSS)) && placedexit==false && roomcount>12 && nodes[0].startdistance>4 && rnd.NextDouble()>0.75);
            const bool secretroom=(roomcount>2 && placeexit==false && rnd.NextDouble()>0.9);

            while(tries++<20 && placedroom==false)
            {
                if(placeboss==true)
                {
                    nodes[0].flags|=BUILD_FLAG_BOSSROOM;
                    if(tryPlaceRoom(mapdata,rnd,nodes[0],buildflags,doorcount,nodes,nodecount,countof(nodes)))
                    {
                        roomcount++;
                        placedroom=true;
                        buildflags|=BUILD_FLAG_PLACEDBOSS;
                    }
                }
                else if(placeexit==true)
                {
                    if(tryPlaceExitRoom(mapdata,rnd,nodes[0],buildflags,doorcount))
                    {
                        roomcount++;
                        placedexit=true;
                        buildflags|=BUILD_FLAG_PLACEDEXIT;
                        placedroom=true;
                    }
                }
                else if(secretroom==true)
                {
                    if(tryPlaceSecretRoom(mapdata,rnd,nodes[0],buildflags,doorcount))
                    {
                        roomcount++;
                        placedroom=true;
                    }
                }
                else if(tryPlaceRoom(mapdata,rnd,nodes[0],buildflags,doorcount,nodes,nodecount,countof(nodes)))
                {
                    roomcount++;
                    placedroom=true;
                }

            }

            removeFirstNode(nodes,countof(nodes));
            nodecount--;
        }

        // add index metadata for actors and items
        uint8_t actoridx=0;
        uint8_t itemidx=0;
        for(int32_t y=0; y<MAP_SIZE; y++)
        {
            for(int32_t x=0; x<MAP_SIZE; x++)
            {
                uint8_t tile=mapdata[(y*MAP_SIZE*2)+(x*2)];
                if(tile>=Tile_FirstItem && tile<=Tile_LastItem)
                {
                    mapdata[(y*MAP_SIZE*2)+(x*2)+1]=itemidx++;
                }
                if(tile>=Tile_FirstActor && tile<=Tile_LastActor)
                {
                    mapdata[(y*MAP_SIZE*2)+(x*2)+1]=actoridx++;
                }
            }
        }

        replaceTile(mapdata,255,Tile_Wall01,0);

        // replace keys if no corresponding door was placed
        if((buildflags & BUILD_FLAG_PLACEDKEY1) && !(buildflags & BUILD_FLAG_PLACEDSECRET1))
        {
            replaceTile(mapdata,Tile_Item_Key1,Tile_Empty,0);
        }
        if((buildflags & BUILD_FLAG_PLACEDKEY2) && !(buildflags & BUILD_FLAG_PLACEDSECRET2))
        {
            replaceTile(mapdata,Tile_Item_Key2,Tile_Empty,0);
        }

        // clear blocking decorations around doors
        for(int32_t y=1; y<MAP_SIZE-1; y++)
        {
            for(int32_t x=1; x<MAP_SIZE-1; x++)
            {
                int8_t wallcount=0;
                uint8_t tile=mapdata[(y*MAP_SIZE*2)+(x*2)];
                if((tile>=Tile_FirstDoor && tile<=Tile_LastDoor) || tile==Tile_SecretPushWall)
                {
                    for(int32_t yy=y-1; yy<=y+1; yy++)
                    {
                        for(int32_t xx=x-1; xx<=x+1; xx++)
                        {
                            uint8_t tile2=mapdata[(yy*MAP_SIZE*2)+(xx*2)];
                            if((xx!=x || yy!=y) && (tile2>=Tile_FirstBlockingDecoration && tile2<=Tile_LastBlockingDecoration))
                            {
                                setMapTile(mapdata,xx,yy,Tile_Empty,0);
                            }
                        }
                    }
                }
            }
        }

    }while(roomcount<15 || !(buildflags & BUILD_FLAG_PLACEDEXIT));

}
