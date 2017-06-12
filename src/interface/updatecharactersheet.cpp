/*-------------------------------------------------------------------------------

	BARONY
	File: updatecharactersheet.cpp
	Desc: contains updateCharacterSheet()

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../main.hpp"
#include "../game.hpp"
#include "../stat.hpp"
#include "../items.hpp"
#include "../player.hpp"
#include "../colors.hpp"
#include "interface.hpp"

void drawSkillsSheet();
void statsHoverText();

/*-------------------------------------------------------------------------------

	updateCharacterSheet

	Draws the character sheet and processes all interaction with it

-------------------------------------------------------------------------------*/

void updateCharacterSheet()
{
	int i = 0;
	int x = 0;
	SDL_Rect pos;
	bool b = false;
	node_t* node = NULL;
	Entity* entity = NULL;
	Item* item = NULL;
	int c;

	// draw window
	drawWindowFancy(0, 0, 224, 196);
	pos.x = 8;
	pos.y = 8;
	pos.w = 208;
	pos.h = 180;
	drawRect(&pos, 0, 255);
	//drawImage(character_bmp, NULL, &pos);
	//pos.x=0; pos.y=196;
	//pos.w=222; pos.h=392-196;
	//drawTooltip(&pos);
	drawWindowFancy(0, 196, 224, 392);

	// character sheet
	double ofov = fov;
	fov = 50;
	if (players[clientnum] != nullptr && players[clientnum]->entity != nullptr)
	{
		if (!softwaremode)
		{
			glClear(GL_DEPTH_BUFFER_BIT);
		}
		//TODO: These two NOT PLAYERSWAP
		//camera.x=players[clientnum]->x/16.0+.5*cos(players[clientnum]->yaw)-.4*sin(players[clientnum]->yaw);
		//camera.y=players[clientnum]->y/16.0+.5*sin(players[clientnum]->yaw)+.4*cos(players[clientnum]->yaw);
		camera_charsheet.x = players[clientnum]->entity->x / 16.0 + .65;
		camera_charsheet.y = players[clientnum]->entity->y / 16.0 + .65;
		camera_charsheet.z = players[clientnum]->entity->z * 2;
		//camera.ang=atan2(players[clientnum]->y/16.0-camera.y,players[clientnum]->x/16.0-camera.x); //TODO: _NOT_ PLAYERSWAP
		camera_charsheet.ang = 5 * PI / 4;
		camera_charsheet.vang = PI / 20;
		camera_charsheet.winx = 8;
		camera_charsheet.winy = 8;
		camera_charsheet.winw = 208;
		camera_charsheet.winh = 180;
		b = players[clientnum]->entity->flags[BRIGHT];
		players[clientnum]->entity->flags[BRIGHT] = true;
		if (!players[clientnum]->entity->flags[INVISIBLE])
		{
			glDrawVoxel(&camera_charsheet, players[clientnum]->entity, REALCOLORS);
		}
		players[clientnum]->entity->flags[BRIGHT] = b;
		c = 0;
		if (multiplayer != CLIENT)
		{
			for (node = players[clientnum]->entity->children.first; node != nullptr; node = node->next)
			{
				if (c == 0)
				{
					c++;
					continue;
				}
				entity = (Entity*) node->element;
				if ( !entity->flags[INVISIBLE] )
				{
					b = entity->flags[BRIGHT];
					entity->flags[BRIGHT] = true;
					glDrawVoxel(&camera_charsheet, entity, REALCOLORS);
					entity->flags[BRIGHT] = b;
				}
				c++;
			}
			for ( node = map.entities->first; node != NULL; node = node->next )
			{
				entity = (Entity*) node->element;
				if ( (Sint32)entity->getUID() == -4 )
				{
					glDrawSprite(&camera_charsheet, entity, REALCOLORS);
				}
			}
		}
		else
		{
			for ( node = map.entities->first; node != NULL; node = node->next )
			{
				entity = (Entity*) node->element;
				if ( (entity->behavior == &actPlayerLimb && entity->skill[2] == clientnum && !entity->flags[INVISIBLE]) || (Sint32)entity->getUID() == -4 )
				{
					b = entity->flags[BRIGHT];
					entity->flags[BRIGHT] = true;
					if ( (Sint32)entity->getUID() == -4 )
					{
						glDrawSprite(&camera_charsheet, entity, REALCOLORS);
					}
					else
					{
						glDrawVoxel(&camera_charsheet, entity, REALCOLORS);
					}
					entity->flags[BRIGHT] = b;
				}
			}
		}
	}
	fov = ofov;

	ttfPrintTextFormatted(ttf12, 8, 202, "%s", stats[clientnum]->name);
	ttfPrintTextFormatted(ttf12, 8, 214, language[359], stats[clientnum]->LVL, language[1900 + client_classes[clientnum]]);
	ttfPrintTextFormatted(ttf12, 8, 226, language[360], stats[clientnum]->EXP);
	ttfPrintTextFormatted(ttf12, 8, 238, language[361], currentlevel);

	// attributes
	ttfPrintTextFormatted(ttf12, 8, 262, language[1200], statGetSTR(stats[clientnum]), stats[clientnum]->STR);
	ttfPrintTextFormatted(ttf12, 8, 274, language[1201], statGetDEX(stats[clientnum]), stats[clientnum]->DEX);
	ttfPrintTextFormatted(ttf12, 8, 286, language[1202], statGetCON(stats[clientnum]), stats[clientnum]->CON);
	ttfPrintTextFormatted(ttf12, 8, 298, language[1203], statGetINT(stats[clientnum]), stats[clientnum]->INT);
	ttfPrintTextFormatted(ttf12, 8, 310, language[1204], statGetPER(stats[clientnum]), stats[clientnum]->PER);
	ttfPrintTextFormatted(ttf12, 8, 322, language[1205], statGetCHR(stats[clientnum]), stats[clientnum]->CHR);

	// armor, gold, and weight
	ttfPrintTextFormatted(ttf12, 8, 346, language[370], stats[clientnum]->GOLD);
	ttfPrintTextFormatted(ttf12, 8, 358, language[371], AC(stats[clientnum]));
	Uint32 weight = 0;
	for ( node = stats[clientnum]->inventory.first; node != NULL; node = node->next )
	{
		item = (Item*)node->element;
		weight += items[item->type].weight * item->count;
	}
	weight += stats[clientnum]->GOLD / 100;
	ttfPrintTextFormatted(ttf12, 8, 370, language[372], weight);

	drawSkillsSheet();
	statsHoverText();
}

void drawSkillsSheet()
{
	SDL_Rect pos;
	pos.x = xres - 208;
	pos.w = 208;
	pos.y = 32;
	pos.h = (NUMPROFICIENCIES * TTF12_HEIGHT) + (TTF12_HEIGHT * 3);

	drawWindowFancy(pos.x, pos.y, pos.x + pos.w, pos.y + pos.h);

	ttfPrintTextFormatted(ttf12, pos.x + 4, pos.y + 8, language[1883]);

	pos.y += TTF12_HEIGHT * 2 + 8;

	SDL_Rect initialSkillPos = pos;
	//Draw skill names.
	for ( int c = 0; c < (NUMPROFICIENCIES); ++c, pos.y += (TTF12_HEIGHT /** 2*/) )
	{
		ttfPrintTextFormatted(ttf12, pos.x + 4, pos.y, "%s:", language[236 + c]);
	}

	//Draw skill levels.
	pos = initialSkillPos;
	Uint32 color;
	for ( int i = 0; i < (NUMPROFICIENCIES); ++i, pos.y += (TTF12_HEIGHT /** 2*/) )
	{
		if ( skillCapstoneUnlocked(clientnum, i) )
		{
			color = uint32ColorGreen(*mainsurface);
		}
		else
		{
			color = uint32ColorWhite(*mainsurface);
		}

		if ( stats[clientnum]->PROFICIENCIES[i] == 0 )
		{
			ttfPrintTextFormattedColor(ttf12, pos.x + 4, pos.y, color, language[363]);
		}
		else if ( stats[clientnum]->PROFICIENCIES[i] < SKILL_LEVEL_BASIC )
		{
			ttfPrintTextFormattedColor(ttf12, pos.x + 4, pos.y, color, language[364]);
		}
		else if ( stats[clientnum]->PROFICIENCIES[i] >= SKILL_LEVEL_BASIC && stats[clientnum]->PROFICIENCIES[i] < SKILL_LEVEL_SKILLED )
		{
			ttfPrintTextFormattedColor(ttf12, pos.x + 4, pos.y, color, language[365]);
		}
		else if ( stats[clientnum]->PROFICIENCIES[i] >= SKILL_LEVEL_SKILLED && stats[clientnum]->PROFICIENCIES[i] < SKILL_LEVEL_EXPERT )
		{
			ttfPrintTextFormattedColor(ttf12, pos.x + 4, pos.y, color, language[366]);
		}
		else if ( stats[clientnum]->PROFICIENCIES[i] >= SKILL_LEVEL_EXPERT && stats[clientnum]->PROFICIENCIES[i] < SKILL_LEVEL_MASTER )
		{
			ttfPrintTextFormattedColor(ttf12, pos.x + 4, pos.y, color, language[367]);
		}
		else if ( stats[clientnum]->PROFICIENCIES[i] >= SKILL_LEVEL_MASTER && stats[clientnum]->PROFICIENCIES[i] < SKILL_LEVEL_LEGENDARY )
		{
			ttfPrintTextFormattedColor(ttf12, pos.x + 4, pos.y, color, language[368]);
		}
		else if ( stats[clientnum]->PROFICIENCIES[i] >= SKILL_LEVEL_LEGENDARY )
		{
			ttfPrintTextFormattedColor(ttf12, pos.x + 4, pos.y, color, language[369]);
		}
	}
}

void statsHoverText()
{
	int pad_y = 262; // 262 px.
	int pad_x = 8; // 8 px.
	int off_h = TTF12_HEIGHT - 4; // 12px. height of stat line.
	int off_w = 216; // 216px. width of stat line.
	int i = 0;
	int j = 0;
	SDL_Rect src;
	SDL_Rect pos;

	int tooltip_offset_x = 16; // 16px.
	int tooltip_offset_y = 16; // 16px.
	int tooltip_base_h = TTF12_HEIGHT;
	int tooltip_pad_h = 8;
	int tooltip_text_pad_x = 8;

	char tooltipHeader[6][128] =
	{
		"strength bonuses: ",
		"dexterity bonuses: ",
		"constitution bonuses: ",
		"intelligence bonuses: ",
		"perception bonuses: ",
		"charisma bonuses: "
	};

	int numInfoLines = 0;
	
	SDL_Surface *tmp_bmp = NULL;

	if ( attributespage == 0 )
	{
		for ( i = 0; i < 6; i++ ) // cycle through 6 stats.
		{
			switch ( i )
			{
				// prepare the stat image.
				case 0:
					numInfoLines = 3;
					tmp_bmp = str_bmp64u;
					break;
				case 1:
					numInfoLines = 1;
					tmp_bmp = dex_bmp64u;
					break;
				case 2:
					tmp_bmp = con_bmp64u;
					break;
				case 3:
					tmp_bmp = int_bmp64u;
					break;
				case 4:
					tmp_bmp = per_bmp64u;
					break;
				case 5:
					tmp_bmp = chr_bmp64u;
					break;
				default:
					numInfoLines = 0;
					break;
			}

			if ( mouseInBounds(pad_x, pad_x + off_w, pad_y, pad_y + off_h) )
			{
				src.x = mousex + tooltip_offset_x;
				src.y = mousey + tooltip_offset_y;
				src.h = tooltip_base_h * (numInfoLines + 1) + tooltip_pad_h;
				src.w = 256;
				drawTooltip(&src);

				pos.x = src.x + 6;
				pos.y = src.y + 4;
				pos.h = 32;
				pos.w = 32;

				drawImageScaled(tmp_bmp, NULL, &pos);
				
				src.x = pos.x + 16;
				src.y += 2;

				ttfPrintText(ttf12, src.x + 4, src.y + 4, tooltipHeader[i]);

				for ( j = numInfoLines; j > 0; j-- )
				{
					ttfPrintText(ttf12, src.x + 4 + tooltip_text_pad_x, src.y + 4 + (tooltip_base_h * (numInfoLines - j + 1)), "test");
				}
			}

			numInfoLines = 0;
			pad_y += 12;
		}
	}


	//if ( mouseInBounds(8, 224, 310, 310 + TTF12_HEIGHT - 4) && attributespage == 0 ) { //draw tooltip for PER
	//																				   //Tooltip
	//	SDL_Rect src;
	//	src.x = mousex + 16; src.y = mousey + 8;
	//	src.h = (TTF12_HEIGHT * 3 + 8);
	//	src.w = longestline("perception bonuses")*TTF12_WIDTH + 16 + 8;
	//	drawTooltip(&src);
	//	ttfPrintText(ttf12, src.x + 4, src.y + 4, "perception bonuses");
	//	ttfPrintTextFormatted(ttf12, src.x + 4 + 8, src.y + 4 + TTF12_HEIGHT, language[2066], mod_getCritChance(stats[clientnum]));
	//	ttfPrintTextFormatted(ttf12, src.x + 4 + 8, src.y + 4 + TTF12_HEIGHT * 2, language[2067], mod_getCritDamage(stats[clientnum]));
	//}
	//else if ( mouseInBounds(8, 224, 262, 262 + TTF12_HEIGHT - 4) && attributespage == 0 ) { //draw tooltip for STR
	//	SDL_Rect src;
	//	src.x = mousex + 16; src.y = mousey + 8;
	//	src.h = (TTF12_HEIGHT * 3 + 8);
	//	src.w = longestline("strength bonuses")*TTF12_WIDTH + 8;
	//	drawTooltip(&src);
	//	ttfPrintText(ttf12, src.x + 4, src.y + 4, "strength bonuses");
	//	if ( stats[clientnum]->weapon != NULL ) {
	//		ttfPrintTextFormatted(ttf12, src.x + 4 + 8, src.y + 4 + TTF12_HEIGHT, language[2068], stats[clientnum]->weapon->weaponGetAttack());
	//	}
	//	else {
	//		ttfPrintTextFormatted(ttf12, src.x + 4 + 8, src.y + 4 + TTF12_HEIGHT, language[2068], 0);
	//	}
	//	ttfPrintTextFormatted(ttf12, src.x + 4 + 8, src.y + 4 + TTF12_HEIGHT * 2, language[2069], statGetAttack(stats[clientnum]));
	//}
}
