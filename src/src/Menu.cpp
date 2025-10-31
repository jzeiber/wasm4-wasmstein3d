#include "wasmstring.h"
#include "Engine.h"
#include "Menu.h"
#include "Sounds.h"
#include "Save.h"
#include "Generated/fxdata.h"

#include "wasm4.h"

#define MENU_ENTRY_END 0
#define MENU_STR(x) (const void*)(x)
#define MENU_CALLBACK(x) (const void*)(x)

typedef void (*MenuFn)(void);

// Main menu
const char Str_WASMStein3D[] PROGMEM = "WASMSTEIN 3D";
const char Str_Continue[] PROGMEM = "CONTINUE";
const char Str_NewGame[] PROGMEM = "NEW GAME";
const char Str_Sound[] PROGMEM = "SOUND:";
const char Str_ViewScores[] PROGMEM = "VIEW SCORES";
const char Str_LoadGame[] PROGMEM = "LOAD GAME";
const char Str_On[] PROGMEM = "ON";
const char Str_Off[] PROGMEM = "OFF";
const char Str_Help[] PROGMEM = "HELP";

const void* const Menu_Main[] PROGMEM = 
{
	Str_WASMStein3D,
	Str_NewGame,		MENU_CALLBACK(&Menu::chooseNewSlot),
	Str_LoadGame,		MENU_CALLBACK(&Menu::loadGame),
	Str_ViewScores,		MENU_CALLBACK(&Menu::viewScores),
	Str_Sound,			MENU_CALLBACK(&Menu::toggleSound),
	Str_Help,			MENU_CALLBACK(&Menu::showHelp),
	MENU_ENTRY_END
};

// Difficulty menu
const char Str_ChooseDifficulty[] PROGMEM = "HOW TOUGH ARE YOU?";
const char Str_SkillBaby[] PROGMEM = "CAN I PLAY, DADDY?";
const char Str_SkillEasy[] PROGMEM = "DON'T HURT ME.";
const char Str_SkillMedium[] PROGMEM = "BRING 'EM ON!";
const char Str_SkillHard[] PROGMEM = "I AM DEATH INCARNATE!";

const void* const Menu_ChooseDifficulty[] PROGMEM = 
{
	Str_ChooseDifficulty,
	Str_SkillBaby,		MENU_CALLBACK(&Menu::setDifficulty),
	Str_SkillEasy,		MENU_CALLBACK(&Menu::setDifficulty),
	Str_SkillMedium,	MENU_CALLBACK(&Menu::setDifficulty),
	Str_SkillHard,		MENU_CALLBACK(&Menu::setDifficulty),
	MENU_ENTRY_END
};

// High scores screen
const char Str_HighScores[] PROGMEM = "HIGH SCORES";
const void* const Menu_ViewScores[] PROGMEM =
{
	Str_HighScores,
	MENU_ENTRY_END
};

// Select slot menu
const char Str_SelectSlot[] PROGMEM = "CHOOSE A SLOT";
const char Str_SaveSlot[] PROGMEM = "";
const char Str_EmptySlot[] PROGMEM = "EMPTY SLOT";
const void* const Menu_SelectSlot[] PROGMEM =
{
	Str_SelectSlot,
	Str_SaveSlot,		MENU_CALLBACK(&Menu::chooseDifficulty),
	Str_SaveSlot,		MENU_CALLBACK(&Menu::chooseDifficulty),
	Str_SaveSlot,		MENU_CALLBACK(&Menu::chooseDifficulty),
	MENU_ENTRY_END
};


// Load game menu
const void* const Menu_LoadGame[] PROGMEM =
{
	Str_LoadGame,
	Str_SaveSlot,		MENU_CALLBACK(&Menu::loadSelectedSave),
	Str_SaveSlot,		MENU_CALLBACK(&Menu::loadSelectedSave),
	Str_SaveSlot,		MENU_CALLBACK(&Menu::loadSelectedSave),
	MENU_ENTRY_END
};

// Help screen
const void* const Menu_Help[] PROGMEM =
{
	Str_WASMStein3D,
	MENU_ENTRY_END
};


const char Str_Overwrite[] PROGMEM = "OVERWRITE THIS SAVE?";
const char Str_Yes[] PROGMEM = "YES";
const char Str_No[] PROGMEM = "NO";

// Overwrite dialog menu
const void* const Menu_OverwriteSlot[] PROGMEM =
{
	Str_Overwrite,
	Str_Yes,		MENU_CALLBACK(&Menu::chooseDifficulty),
	Str_No,			MENU_CALLBACK(&Menu::chooseNewSlot),
	MENU_ENTRY_END
};

const char Str_GameOver[] PROGMEM = "GAME OVER";
const char Str_FinalScore[] PROGMEM = "FINAL SCORE:";

// Game over menu
const void* const Menu_GameOver[] PROGMEM =
{
	Str_GameOver,
	MENU_ENTRY_END
};

// Win screen
const void* const Menu_YouWin[] PROGMEM =
{
	Str_GameOver,
	MENU_ENTRY_END
};

// Floor complete screen
const void* const Menu_FloorComplete[] PROGMEM =
{
	Str_GameOver,
	MENU_ENTRY_END
};

// New high score screen
const char Str_NewHighScore[] PROGMEM = "NEW HIGH SCORE!";
const void* const Menu_NewHighScore[] PROGMEM =
{
	Str_NewHighScore,
	MENU_ENTRY_END
};

void Menu::loadSelectedSave()
{
	engine.save.activeSlot = engine.menu.currentSelection;
	if (engine.save.saveFile.slots[engine.save.activeSlot].hp)
	{
		engine.loadGame();
	}
	else
	{
		Platform.playSound(NOWAYSND);
	}
}

void Menu::toggleSound()
{
	Platform.setMuted(!Platform.isMuted());
}

void Menu::chooseNewSlot()
{
	engine.menu.switchMenu(Menu_SelectSlot);
}

void Menu::loadGame()
{
	engine.menu.switchMenu(Menu_LoadGame);
}

void Menu::showHelp()
{
	engine.menu.switchMenu(Menu_Help);
}

void Menu::chooseDifficulty()
{
	if (engine.menu.currentMenu == Menu_SelectSlot)
	{
		engine.save.activeSlot = engine.menu.currentSelection;
		if (engine.save.saveFile.slots[engine.save.activeSlot].hp)
		{
			engine.menu.switchMenu(Menu_OverwriteSlot);
			return;
		}
	}
	engine.menu.switchMenu(Menu_ChooseDifficulty);
	engine.menu.currentSelection = 2;
}

void Menu::viewScores()
{
	engine.menu.switchMenu(Menu_ViewScores);
}

void Menu::setDifficulty()
{
	engine.difficulty = engine.menu.currentSelection;
	engine.startNewGame();
}

void Menu::init()
{
	switchMenu(Menu_Main);
	//	switchMenu(Menu_FloorComplete);
//	engine.map.enemyCount = 10;
//	engine.map.secretCount = 10;
//	engine.map.treasureCount = 10;
//	engine.player.enemiesKilled = 6;
//	engine.player.secretsFound = 10;
//	engine.player.treasureCollected = 0;
}

#define PERCENT100AMT	10000

void Menu::printStat(const char* name, uint8_t num, uint8_t count, const int offsetx, const int offsety, int x, int y, uint8_t startTime)
{
	if (currentSelection >= startTime)
	{
		const int statX = 110;
		int progress = (100 * (currentSelection - startTime)) / 16;
		int percent = (count > 0) ? (100 * num) / count : 100;
		int endTime = startTime + (16 * percent) / 100;
		if (percent > progress)
		{
			percent = progress;
		}
		engine.renderer.drawString(name, offsetx + x, offsety + y, PALETTE_DARKGREY);

		if (progress >= 0)
		{
			engine.renderer.drawInt(percent, offsetx + statX, offsety + y, PALETTE_DARKGREY);

			if (currentSelection < endTime)
			{
				Platform.playSound(ENDBONUS1SND);
			}
			else if (currentSelection == endTime)
			{
				if (percent == 100)
				{
					Platform.playSound(PERCENT100SND);
					engine.player.givePoints(PERCENT100AMT);
				}
				else if (percent == 0)
				{
					Platform.playSound(NOBONUSSND);
				}
				else
				{
					Platform.playSound(ENDBONUS2SND);
				}
			}
		}
	}
}


void Menu::draw()
{
	int startY = 20;
	const int itemSpacing = 8;
	const int offsetX = (DISPLAYWIDTH/2)-64;
	const int offsetY = (SCREEN_SIZE-64)/2;

	clearDisplay(PALETTE_WHITE);

//	clearDisplay(1);
//	engine.renderer.drawString(PSTR("YOU WIN!"), 0, 0, 0);
//	engine.renderer.drawString(PSTR("SHOOT"), 0, 6, 0);
//	engine.renderer.drawString(PSTR("HOLD TO STRAFE"), 0, 12, 0);
//	engine.renderer.drawString(PSTR("DOUBLE TAP"), 0, 18, 0);
//	engine.renderer.drawString(PSTR("TO SWAP WEAPON"), 0, 24, 0);
//	return;

	if (currentMenu == Menu_Main)
	{
		//engine.renderer.drawBackground(Data_titleBG);
		//engine.renderer.drawBackground((uint8_t *)&Data_titleBG_data[0]);
		*DRAW_COLORS=(PALETTE_WHITE << 4) | PALETTE_BLACK;
		blit(&Data_title_1bpp[0],0,0,160,80,BLIT_1BPP);
		startY = 10;
		engine.renderer.drawString(VERSION_STR,1,SCREEN_SIZE-(FONT_HEIGHT)-1,PALETTE_DARKGREY);
		engine.renderer.drawString(PSTR("WASM-4 PORT @JZEIBER"), DISPLAYWIDTH - (20*4), SCREEN_SIZE - (FONT_HEIGHT*2) - 2, PALETTE_DARKGREY);
		engine.renderer.drawString(PSTR("ORIGINAL @JAMESHHOWARD"), DISPLAYWIDTH - (22*4), SCREEN_SIZE - FONT_HEIGHT - 1, PALETTE_DARKGREY);
	}
	else if (currentMenu == Menu_Help)
	{
		engine.renderer.drawBackground((uint8_t *)&Data_helpBG_data[0], offsetY);
		return;
	}
	else if (currentMenu == Menu_YouWin)
	{
		//engine.renderer.drawBackground(Data_winBG);
		engine.renderer.drawBackground((uint8_t *)&Data_winBG_data[0], offsetY);
		engine.renderer.drawString(PSTR("YOU WIN!"), offsetX + 76, offsetY + startY - 2, PALETTE_DARKGREY);
		engine.renderer.drawString(Str_FinalScore, offsetX + 72, offsetY + startY + 8, PALETTE_DARKGREY);
		engine.renderer.drawLong(engine.player.score, offsetX + 72 + 44, offsetY + startY + 16, PALETTE_DARKGREY);
		return;
	}
	else if (currentMenu == Menu_FloorComplete)
	{
		engine.renderer.drawBackground(engine.frameCount & 16 ? (uint8_t *)&Data_floorComplete1BG_data[0] : (uint8_t *)&Data_floorComplete2BG_data[0], offsetY);
		engine.renderer.drawString(PSTR("FLOOR COMPLETED"), offsetX + 62, offsetY + 2, PALETTE_DARKGREY);
		int columnX = 62;

		printStat(PSTR("KILLS:    %"), engine.player.enemiesKilled, engine.map.enemyCount, offsetX, offsetY, columnX + 12, 10, 16);
		printStat(PSTR("SECRETS:    %"), engine.player.secretsFound, engine.map.secretCount, offsetX, offsetY, columnX + 4, 16, 50);
		printStat(PSTR("TREASURE:    %"), engine.player.treasureCollected, engine.map.treasureCount, offsetX, offsetY, columnX, 22, 92);
		if (engine.frameCount & 1 && currentSelection < 127)
		{
			currentSelection++;
		}
	}
	else
	{
		clearDisplay(PALETTE_WHITE);

		const char* title = (const char*)pgm_read_ptr(&currentMenu[0]);
		int titleWidth = strlen_P(title) * 4;

		engine.renderer.drawBox(0, 4, DISPLAYWIDTH, FONT_HEIGHT, PALETTE_BLACK);
		engine.renderer.drawBox(DISPLAYWIDTH / 2 - titleWidth / 2 - 2, 2, titleWidth + 4, FONT_HEIGHT + 4, PALETTE_BLACK);
		engine.renderer.drawString(title, DISPLAYWIDTH / 2 - titleWidth / 2, 4, PALETTE_DARKGREY);
	}

	int index = 1;
	int x = 14;
	int y = startY;
	int item = 0;

	while(1)
	{
		if(pgm_read_ptr(&currentMenu[index]) == 0)
			break;

		const char* text = (const char*)pgm_read_ptr(&currentMenu[index]);

		if (text == Str_SaveSlot)
		{
			SaveSlot* slot = &engine.save.saveFile.slots[item];
			if (slot->hp == 0)
			{
				engine.renderer.drawString(Str_EmptySlot, offsetX + x, offsetY + y, PALETTE_DARKGREY);
			}
			else
			{
				// FLOOR:XX SKILL: NORMAL
				engine.renderer.drawInt(slot->level + 1, offsetX + x + 28, offsetY + y, PALETTE_DARKGREY);
				engine.renderer.drawString(PSTR("FLOOR:"), offsetX + x, offsetY + y, PALETTE_DARKGREY);
				engine.renderer.drawString(PSTR("SKILL:"), offsetX + x + 38, offsetY + y, PALETTE_DARKGREY);
				switch (slot->difficulty)
				{
				case 0:
					engine.renderer.drawString(PSTR("BABY"), offsetX + x + 66, offsetY + y, PALETTE_DARKGREY);
					break;
				case 1:
					engine.renderer.drawString(PSTR("EASY"), offsetX + x + 66, offsetY + y, PALETTE_DARKGREY);
					break;
				case 2:
					engine.renderer.drawString(PSTR("NORMAL"), offsetX + x + 66, offsetY + y, PALETTE_DARKGREY);
					break;
				case 3:
					engine.renderer.drawString(PSTR("HARD"), offsetX + x + 66, offsetY + y, PALETTE_DARKGREY);
					break;
				}
			}
		}
		else
		{
			engine.renderer.drawString(text, offsetX + x, offsetY + y, PALETTE_DARKGREY);
		}

		if(text == Str_Sound)
		{
			if(Platform.isMuted())
				engine.renderer.drawString(Str_Off, offsetX + 40, offsetY + y, PALETTE_DARKGREY);
			else
				engine.renderer.drawString(Str_On, offsetX + 40, offsetY + y, PALETTE_DARKGREY);
		}
		index += 2;
		y += itemSpacing;
		item++;
	}

	if (numMenuItems())
	{
		int positionDelta = 0;
		if (selectionDelta < 0)
		{
			positionDelta = -itemSpacing / 2;
		}
		else if (selectionDelta > 0)
		{
			positionDelta = itemSpacing / 2;
		}
		engine.renderer.drawSprite2D(UI_Gun, offsetX + 2, offsetY + startY + currentSelection * itemSpacing + positionDelta);
	}

	if (currentMenu == Menu_ChooseDifficulty)
	{
		engine.renderer.drawSprite2D(UI_BJFace_Baby + currentSelection, DISPLAYWIDTH - 30, offsetY + startY);
	}
	else if (currentMenu == Menu_ViewScores)
	{
		for (int n = 0; n < 3; n++)
		{
			for (int j = 0; j < 3; j++)
			{
				engine.renderer.drawGlyph(engine.save.saveFile.scores[n].name[j] - FIRST_FONT_GLYPH, offsetX + x + 16 + j * 4, y, PALETTE_DARKGREY);
			}
			engine.renderer.drawLong(engine.save.saveFile.scores[n].score, offsetX + x + 80, y, PALETTE_DARKGREY);
			y += itemSpacing;
		}
	}
	else if (currentMenu == Menu_GameOver)
	{
		engine.renderer.drawSprite2D(UI_BJFace_Dead, offsetX + 20, offsetY + startY);
		engine.renderer.drawString(Str_FinalScore, offsetX + 64, offsetY + startY + 8, PALETTE_DARKGREY);
		engine.renderer.drawLong(engine.player.score, offsetX + 64 + 44, offsetY + startY + 16, PALETTE_DARKGREY);
	}
	else if (currentMenu == Menu_NewHighScore)
	{
		engine.renderer.drawString(Str_FinalScore, offsetX + 40, offsetY + 16, PALETTE_DARKGREY);
		engine.renderer.drawLong(engine.player.score, offsetX + 40 + 44, offsetY + 22, PALETTE_DARKGREY);

		engine.renderer.drawString(PSTR("ENTER YOUR NAME"), offsetX + 36, offsetY + 50, PALETTE_DARKGREY);

		HighScore* score = &engine.save.saveFile.scores[engine.save.activeSlot];
		x = 58;

		engine.renderer.drawGlyph('+' - FIRST_FONT_GLYPH, offsetX + x + 4 * currentSelection, offsetY + 32 + (selectionDelta < 0 ? -1 : 0), PALETTE_DARKGREY);
		engine.renderer.drawGlyph('*' - FIRST_FONT_GLYPH, offsetX + x + 4 * currentSelection, offsetY + 40 + (selectionDelta > 0 ? 1 : 0), PALETTE_DARKGREY);

		engine.renderer.drawGlyph(score->name[0] - FIRST_FONT_GLYPH, offsetX + x, offsetY + 36, PALETTE_DARKGREY);
		engine.renderer.drawGlyph(score->name[1] - FIRST_FONT_GLYPH, offsetX + x + 4, offsetY + 36, PALETTE_DARKGREY);
		engine.renderer.drawGlyph(score->name[2] - FIRST_FONT_GLYPH, offsetX + x + 8, offsetY + 36, PALETTE_DARKGREY);

	}
}

void Menu::update()
{
	if (selectionDelta != 0)
	{
		if (selectionDelta > 0)
			selectionDelta--;
		if (selectionDelta < 0)
			selectionDelta++;

		if (!selectionDelta && currentMenu != Menu_NewHighScore)
		{
			Platform.playSound(MOVEGUN2SND);
		}
	}

	if (!debounceInput)
	{
		if (currentMenu == Menu_NewHighScore)
		{
			if (Platform.readInput() & (Input_Dpad_Left | Input_Btn_A))
			{
				if (currentSelection > 0)
				{
					currentSelection--;
					Platform.playSound(MOVEGUN1SND);
				}
			}
			if (Platform.readInput() & (Input_Dpad_Right | Input_Btn_B))
			{
				if (currentSelection < 2)
				{
					currentSelection++;
					Platform.playSound(MOVEGUN1SND);
				}
				else if (Platform.readInput() & Input_Btn_B)
				{
					engine.save.save();
					switchMenu(Menu_ViewScores);
					Platform.playSound(SHOOTSND);
				}
			}
			if (Platform.readInput() & Input_Dpad_Down)
			{
				HighScore* score = &engine.save.saveFile.scores[engine.save.activeSlot];
				if (score->name[currentSelection] == ' ')
				{
					score->name[currentSelection] = 'Z';
				}
				else
				{
					score->name[currentSelection]--;
					if (score->name[currentSelection] < 'A')
					{
						score->name[currentSelection] = ' ';
					}
				}
				Platform.playSound(MOVEGUN1SND);
				selectionDelta = 2;
			}
			if (Platform.readInput() & Input_Dpad_Up)
			{
				HighScore* score = &engine.save.saveFile.scores[engine.save.activeSlot];
				if (score->name[currentSelection] == ' ')
				{
					score->name[currentSelection] = 'A';
				}
				else
				{
			
					score->name[currentSelection]++;
					if (score->name[currentSelection] > 'Z')
					{
						score->name[currentSelection] = ' ';
					}
				}
				Platform.playSound(MOVEGUN1SND);
				selectionDelta = -2;
			}
		}
		else if (numMenuItems())
		{
			if (Platform.readInput() & Input_Dpad_Up)
			{
				currentSelection--;
				selectionDelta = 2;
				if (currentSelection == -1)
				{
					currentSelection = numMenuItems() - 1;
					selectionDelta = 0;
				}
				Platform.playSound(MOVEGUN1SND);
			}
			if (Platform.readInput() & Input_Dpad_Down)
			{
				currentSelection++;
				selectionDelta = -2;
				if (currentSelection == numMenuItems())
				{
					currentSelection = 0;
					selectionDelta = 0;
				}
				Platform.playSound(MOVEGUN1SND);
			}
			if (Platform.readInput() & Input_Btn_B)
			{
				Platform.playSound(SHOOTSND);
				MenuFn fn = (MenuFn)pgm_read_ptr(&currentMenu[currentSelection * 2 + 2]);
				fn();
			}

			if (Platform.readInput() & Input_Btn_A)
			{
				if (currentMenu != Menu_Main)
				{
					switchMenu(Menu_Main);
					Platform.playSound(ESCPRESSEDSND);
				}
			}
		}
		else
		{
			if (Platform.readInput() & (Input_Btn_A | Input_Btn_B))
			{
				if (currentMenu == Menu_FloorComplete)
				{
					if (currentSelection == 127)
					{
						engine.enterNextLevel();
					}
					else
					{
						currentSelection = 127;
					}
					//Platform.playSound(SHOOTSND);
				}
				else if (currentMenu == Menu_GameOver || currentMenu == Menu_YouWin)
				{
					if(currentMenu == Menu_YouWin)
						engine.save.clearActiveSlot();

					if (engine.save.trySubmitHighScore(engine.player.score))
					{
						switchMenu(Menu_NewHighScore);
					}
					else
					{
						switchMenu(Menu_ViewScores);
					}
					Platform.playSound(SHOOTSND);
				}
				else
				{
					switchMenu(Menu_Main);
					Platform.playSound(ESCPRESSEDSND);
				}
			}
		}
	}
	debounceInput = Platform.readInput() != 0;
}

int8_t Menu::numMenuItems()
{
	int8_t index = 1;
	int8_t count = 0;

	while(1)
	{
		if(pgm_read_ptr(&currentMenu[index]) == 0)
			break;
		index += 2;
		count++;
	}
	return count;
}

void Menu::switchMenu(const MenuData* const newMenu)
{
	currentMenu = newMenu;
	currentSelection = 0;
	selectionDelta = 0;
	debounceInput = true;
	engine.fadeTransition();
}
