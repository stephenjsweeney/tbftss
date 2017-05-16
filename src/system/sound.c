/*
Copyright (C) 2015-2016 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "sound.h"

static void loadSounds(void);

static Mix_Chunk *sounds[SND_MAX];
static Mix_Music *music;

static int lastPlayerX;
static int lastPlayerY;

void initSounds(void)
{
	memset(sounds, 0, sizeof(Mix_Chunk*) * SND_MAX);

	music = NULL;

	loadSounds();
}

void playMusic(char *filename, int loop)
{
	if (music != NULL)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(music);
		music = NULL;
	}

	music = Mix_LoadMUS(getFileLocation(filename));

    Mix_PlayMusic(music, (loop) ? -1 : 0);
}

void stopMusic(void)
{
	Mix_HaltMusic();
}

void musicSetPlaying(int playing)
{
	if (music != NULL)
	{
		if (playing)
		{
			Mix_ResumeMusic();
		}
		else
		{
			Mix_PauseMusic();
		}
	}
}

void playSound(int id)
{
	Mix_PlayChannel(-1, sounds[id], 0);
}

void playBattleSound(int id, int x, int y)
{
	float distance, bearing, vol;
	int channel;

	if (player->alive == ALIVE_ALIVE)
	{
		lastPlayerX = player->x;
		lastPlayerY = player->y;
	}

	distance = getDistance(lastPlayerX, lastPlayerY, x, y);

	if (distance <= MAX_BATTLE_SOUND_DISTANCE)
	{
		channel = Mix_PlayChannel(-1, sounds[id], 0);
		if (channel != -1)
		{
			vol = 255;
			vol /= MAX_BATTLE_SOUND_DISTANCE;
			vol *= distance;

			if (distance >= MIN_BATTLE_SOUND_DISTANCE)
			{
				bearing = 360 - getAngle(x, y, lastPlayerX, lastPlayerY);
				Mix_SetPosition(channel, (Sint16)bearing, (Uint8)distance);
			}
			else
			{
				Mix_SetDistance(channel, vol);
			}
		}
	}
}

static Mix_Chunk *loadSound(char *filename)
{
	return Mix_LoadWAV(getFileLocation(filename));
}

static void loadSounds(void)
{
	sounds[SND_ARMOUR_HIT] = loadSound("sound/275151__bird-man__gun-shot.ogg");
	sounds[SND_SHIELD_HIT] = loadSound("sound/49678__ejfortin__energy-short-sword-7.ogg");
	sounds[SND_PLASMA] = loadSound("sound/268344__julien-matthey__jm-noiz-laser-01.ogg");
	sounds[SND_LASER] = loadSound("sound/18382__inferno__hvylas.ogg");
	sounds[SND_MAG] = loadSound("sound/146725__fins__laser.ogg");
	sounds[SND_SHIELD_BREAK] = loadSound("sound/322603__clippysounds__glass-break.ogg");
	sounds[SND_PARTICLE] = loadSound("sound/77087__supraliminal__laser-short.ogg");
	sounds[SND_MISSILE] = loadSound("sound/65787__iwilldstroyu__laserrocket.ogg");
	sounds[SND_BOOST] = loadSound("sound/18380__inferno__hvrl.ogg");
	sounds[SND_RADIO] = loadSound("sound/321906__bruce965__walkie-talkie-roger-beep.ogg");
	sounds[SND_INCOMING] = loadSound("sound/242856__plasterbrain__nuclear-alarm.ogg");
	sounds[SND_GET_ITEM] = loadSound("sound/88275__s-dij__gbc-reload-06.ogg");
	sounds[SND_EXPLOSION_1] = loadSound("sound/162265__qubodup__explosive.ogg");
	sounds[SND_EXPLOSION_2] = loadSound("sound/207322__animationisaac__short-explosion.ogg");
	sounds[SND_EXPLOSION_3] = loadSound("sound/254071__tb0y298__firework-explosion.ogg");
	sounds[SND_EXPLOSION_4] = loadSound("sound/47252__nthompson__bad-explosion.ogg");
	sounds[SND_EXPLOSION_5] = loadSound("sound/172870__escortmarius__carbidexplosion.ogg");
	sounds[SND_JUMP] = loadSound("sound/276912__pauldihor__transform.ogg");
	sounds[SND_ECM] = loadSound("sound/251431__onlytheghosts__fusion-gun-flash0-by-onlytheghosts.ogg");
	sounds[SND_MAG_HIT] = loadSound("sound/172591__timbre__zapitydooda.ogg");
	sounds[SND_POWER_DOWN] = loadSound("sound/39030__wildweasel__d1clsstf.ogg");
	sounds[SND_SELECT_WEAPON] = loadSound("sound/329359__bassoonrckr__reed-guillotine.ogg");
	sounds[SND_TROPHY] = loadSound("sound/278142__ricemaster__effect-notify.ogg");
	sounds[SND_MINE_WARNING] = loadSound("sound/254174__kwahmah-02__s.ogg");
	sounds[SND_TIME_WARNING] = loadSound("sound/320181__dland__hint.ogg");
	sounds[SND_CAP_DEATH] = loadSound("sound/000000_large_explosion.ogg");
	sounds[SND_ZOOM] = loadSound("sound/62491__benboncan__dslr-click.ogg");
	sounds[SND_NO_MISSILES] = loadSound("sound/154934__klawykogut__empty-gun-shot.ogg");
	sounds[SND_RECHARGED] = loadSound("sound/33785__jobro__4-beep-b.ogg");
	sounds[SND_TOW_ROPE] = loadSound("sound/000000_tow-rope-attach.ogg");
	sounds[SND_NEW_OBJECTIVE] = loadSound("sound/246420__oceanictrancer__game-sound-effect-menu.ogg");
	sounds[SND_OBJECTIVE_COMPLETE] = loadSound("sound/107786__leviclaassen__beepbeep.ogg");
	sounds[SND_OBJECTIVE_FAILED] = loadSound("sound/255729__manholo__inception-stab-l.ogg");
	sounds[SND_WAYPOINT] = loadSound("sound/146311__jgeralyn__shortailenliketone2-lower.ogg");

	sounds[SND_GUI_CLICK] = loadSound("sound/257786__xtrgamr__mouse-click.ogg");
	sounds[SND_GUI_SELECT] = loadSound("sound/321104__nsstudios__blip2.ogg");
	sounds[SND_GUI_CLOSE] = loadSound("sound/178064__jorickhoofd__slam-door-shut.ogg");
	sounds[SND_GUI_DENIED] = loadSound("sound/249300__suntemple__access-denied.ogg");
}

void destroySounds(void)
{
	int i;

	for (i = 0 ; i < SND_MAX ; i++)
	{
		if (sounds[i])
		{
			Mix_FreeChunk(sounds[i]);
		}
	}

	if (music != NULL)
	{
		Mix_FreeMusic(music);
	}
}
