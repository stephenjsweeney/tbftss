/*
Copyright (C) 2015 Parallel Realities

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

#ifndef REVISION
#define REVISION 0
#endif

#ifndef DATA_DIR
#define DATA_DIR ""
#endif

#define PI 3.14159265358979323846
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define STRNCPY(dest, src, n) strncpy(dest, src, n); dest[n - 1] = '\0'
#define TO_RAIDANS(angleDegrees) (angleDegrees * PI / 180.0)
#define TO_DEGREES(angleRadians) (angleRadians * 180.0 / PI)

#define SCREEN_WIDTH 	1280
#define SCREEN_HEIGHT 	720

#define MAX_KEYBOARD_KEYS	350
#define MAX_MOUSE_BUTTONS	8

#define FPS				60
#define LOGIC_RATE		(1000 / FPS)

#define MAX_NAME_LENGTH				32
#define MAX_DESCRIPTION_LENGTH		512
#define MAX_FILENAME_LENGTH			1024

#define NUM_TEXTURE_BUCKETS		32

#define MAX_STARS		500

#define MAX_FONTS				32
#define NUM_TEXT_BUCKETS		64

#define MAX_FIGHTER_GUNS		12
#define MAX_TARGET_RANGE		65536

#define GRID_CELL_WIDTH			640
#define GRID_CELL_HEIGHT		360
#define GRID_SIZE				50
#define MAX_GRID_CANDIDATES 	256
#define GRID_RESTRICTION_SIZE	250

#define BF_NONE				0
#define BF_ENGINE			(2 << 0)
#define BF_SYSTEM_DAMAGE	(2 << 1)
#define BF_SHIELD_DAMAGE	(2 << 2)
#define BF_EXPLODES			(2 << 3)

#define EF_NONE				0
#define EF_NO_KILL			(2 << 0)
#define EF_DISABLED			(2 << 1)
#define EF_IMMORTAL			(2 << 2)
#define EF_MISSION_TARGET	(2 << 3)
#define EF_NO_MT_BOX		(2 << 4)
#define EF_HAS_ROPE			(2 << 5)
#define EF_COLLECTS_ITEMS	(2 << 6)
#define EF_MUST_DISABLE		(2 << 7)
#define EF_RETREATING		(2 << 8)
#define EF_NO_EPIC			(2 << 9)
#define EF_STATIC			(2 << 10)
#define EF_TAKES_DAMAGE		(2 << 11)
#define EF_SECONDARY_TARGET	(2 << 12)

#define AIF_NONE				0
#define AIF_FOLLOWS_PLAYER		(2 << 0)
#define AIF_MOVES_TO_PLAYER		(2 << 1)
#define AIF_UNLIMITED_RANGE		(2 << 2)
#define AIF_COLLECTS_ITEMS		(2 << 3)
#define AIF_TOWS				(2 << 4)
#define AIF_RETREATS			(2 << 5)
#define AIF_GOAL_EXTRACTION		(2 << 6)
#define AIF_AVOIDS_COMBAT		(2 << 7)
#define AIF_DEFENSIVE			(2 << 8)
#define AIF_MISSILE_BOAT		(2 << 9)
#define AIF_AGGRESSIVE			(2 << 10)
#define AIF_INSTANT_DIE			(2 << 11)
#define AIF_LONG_RANGE_FIRE		(2 << 12)

/* player abilities */
#define BOOST_RECHARGE_TIME 	(FPS * 7)
#define BOOST_FINISHED_TIME 	(FPS * 0.75)
#define ECM_RECHARGE_TIME		(FPS * 7)

enum
{
	ET_FIGHTER,
	ET_ITEM,
	ET_WAYPOINT,
	ET_EXTRACTION_POINT,
	ET_CAPITAL_SHIP_COMPONENT,
	ET_CAPITAL_SHIP_GUN,
	ET_CAPITAL_SHIP_ENGINE,
	ET_CAPITAL_SHIP
};

enum
{
	TA_LEFT,
	TA_RIGHT,
	TA_CENTER
};

enum
{
	ALIVE_ALIVE,
	ALIVE_DYING,
	ALIVE_DEAD,
	ALIVE_ESCAPED
};

enum
{
	SIDE_NONE,
	SIDE_ALLIES,
	SIDE_REBEL,
	SIDE_PANDORAN,
	SIDE_PIRATE,
	SIDE_CSN,
	SIDE_INF,
	SIDE_UNF,
	SIDE_MAX
};

enum
{
	BT_NONE,
	BT_PARTICLE,
	BT_PLASMA,
	BT_LASER,
	BT_MAG,
	BT_ROCKET,
	BT_MISSILE,
	BT_FF,
	BT_MAX
};

enum
{
	EFFECT_LINE,
	EFFECT_TEXTURE,
	EFFECT_HALO,
	EFFECT_ECM
};

enum
{
	SND_PARTICLE,
	SND_PLASMA,
	SND_LASER,
	SND_MAG,
	SND_ARMOUR_HIT,
	SND_SHIELD_HIT,
	SND_SHIELD_BREAK,
	SND_EXPLOSION_1,
	SND_EXPLOSION_2,
	SND_EXPLOSION_3,
	SND_EXPLOSION_4,
	SND_GET_ITEM,
	SND_MISSILE,
	SND_INCOMING,
	SND_BOOST,
	SND_RADIO,
	SND_GUI_CLICK,
	SND_GUI_SELECT,
	SND_GUI_CLOSE,
	SND_GUI_DENIED,
	SND_MAX
};

enum
{
	OS_INCOMPLETE,
	OS_COMPLETE,
	OS_FAILED,
	OS_CONDITION
};

enum
{
	TT_DESTROY,
	TT_DISABLE,
	TT_WAYPOINT,
	TT_ESCAPED,
	TT_PLAYER_ESCAPED,
	TT_ITEM
};

enum
{
	MS_START,
	MS_IN_PROGRESS,
	MS_PAUSED,
	MS_COMPLETE,
	MS_FAILED
};

enum
{
	WT_BUTTON,
	WT_IMG_BUTTON,
	WT_SELECT,
	WT_SELECT_BUTTON
};

enum
{
	CHALLENGE_ARMOUR,
	CHALLENGE_TIME,
	CHALLENGE_ACCURACY,
	CHALLENGE_NO_LOSSES,
	CHALLENGE_1_LOSS,
	CHALLENGE_LOSSES,
	CHALLENGE_PLAYER_KILLS,
	CHALLENGE_DISABLE,
	CHALLENGE_TIME_MINS
};

enum
{
	STAT_MISSIONS_STARTED,
	STAT_MISSIONS_COMPLETED,
	STAT_SHOTS_FIRED,
	STAT_SHOTS_HIT,
	STAT_MISSILES_FIRED,
	STAT_MISSILES_HIT,
	STAT_ENEMIES_KILLED,
	STAT_ENEMIES_KILLED_PLAYER,
	STAT_ALLIES_KILLED,
	STAT_PLAYER_KILLED,
	STAT_ENEMIES_DISABLED,
	STAT_ENEMIES_ESCAPED,
	STAT_ECM,
	STAT_BOOST,
	STAT_MISSILES_EVADED,
	STAT_MISSILES_STRUCK,
	STAT_CIVILIANS_RESCUED,
	STAT_CIVILIANS_KILLED,
	STAT_TUG,
	STAT_SHUTTLE,
	STAT_NUM_TOWED,
	STAT_ITEMS_COLLECTED,
	STAT_EPIC_KILL_STREAK,
	/* add stats before here, so as not to mess up the stats screen */
	STAT_TIME,
	STAT_MAX
};
