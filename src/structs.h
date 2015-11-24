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

struct SDL_Texture;
typedef struct Texture Texture;
typedef struct Lookup Lookup;
typedef struct Weapon Weapon;
typedef struct Entity Entity;
typedef struct Bullet Bullet;
typedef struct Effect Effect;
typedef struct Objective Objective;
typedef struct StarSystem StarSystem;
typedef struct Challenge Challenge;
typedef struct Mission Mission;
typedef struct Pulse Pulse;
typedef struct Widget Widget;
typedef struct HudMessage HudMessage;
typedef struct Trigger Trigger;
typedef struct GridCell GridCell;

typedef struct {
	float x;
	float y;
} PointF;

struct Texture {
	char name[MAX_DESCRIPTION_LENGTH];
	long hash;
	struct SDL_Texture *texture;
	Texture *next;
};

typedef struct {
	void (*logic)(void);
	void (*draw)(void);
	void (*handleClick)(int x, int y, int btn);
	void (*handleDrag)(int x, int y, int dx, int dy, int cx, int cy);
	void (*handleMouseUp)(int x, int y, int btn);
} Delegate;

struct Lookup {
	char name[MAX_NAME_LENGTH];
	long value;
	Lookup *next;
};

struct Weapon {
	int type;
	int ammo;
	int maxAmmo;
	int x;
	int y;
};

struct Entity {
	int type;
	char name[MAX_NAME_LENGTH];
	char defName[MAX_NAME_LENGTH];
	char groupName[MAX_NAME_LENGTH];
	int active;
	int id;
	int side;
	float x;
	float y;
	int w;
	int h;
	float dx;
	float dy;
	float thrust;
	float speed;
	float angle;
	int alive;
	int health;
	int maxHealth;
	int shield;
	int maxShield;
	int reload;
	int reloadTime;
	int selectedGunType;
	int combinedGuns;
	int shieldRecharge;
	int shieldRechargeRate;
	int systemPower;
	int armourHit;
	int shieldHit;
	int systemHit;
	int thinkTime;
	int aiActionTime;
	int aiAggression;
	int separationRadius;
	Weapon guns[MAX_FIGHTER_GUNS];
	int missiles;
	long flags;
	long aiFlags;
	SDL_Point targetLocation;
	Entity *towing;
	Entity *target;
	Entity *owner;
	void (*action)(void);
	void (*die)(void);
	struct SDL_Texture *texture;
	Entity *next;
};

struct Bullet {
	int type;
	float x;
	float y;
	int w;
	int h;
	float dx;
	float dy;
	int sound;
	int life;
	int damage;
	int angle;
	long flags;
	SDL_Texture *texture;
	Entity *owner;
	Entity *target;
	Bullet *next;
};

typedef struct {
	float x;
	float y;
	float speed;
} Star;

struct Pulse {
	int x;
	int y;
	float size;
	int life;
	int r, g, b;
	Pulse *next;
};

struct Effect {
	int type;
	float x;
	float y;
	float dx;
	float dy;
	float health;
	int size;
	int r;
	int g;
	int b;
	int a;
	SDL_Texture *texture;
	Effect *next;
};

struct Objective {
	int active;
	char description[MAX_DESCRIPTION_LENGTH];
	char targetName[MAX_NAME_LENGTH];
	int targetType;
	int currentValue;
	int targetValue;
	int status;
	int isCondition;
	int isOptional;
	Objective *next;
};

struct Challenge {
	int type;
	int targetValue;
	int passed;
	Challenge *next;
};

struct Trigger {
	int type;
	char targetName[MAX_NAME_LENGTH];
	int targetValue;
	int action;
	char actionValue[MAX_NAME_LENGTH];
	Trigger *next;
};

struct Mission {
	char name[MAX_NAME_LENGTH];
	char description[MAX_DESCRIPTION_LENGTH];
	char filename[MAX_DESCRIPTION_LENGTH];
	char requires[MAX_DESCRIPTION_LENGTH];
	char pilot[MAX_NAME_LENGTH];
	char squadron[MAX_NAME_LENGTH];
	char craft[MAX_NAME_LENGTH];
	int available;
	int completed;
	int epic;
	SDL_Rect rect;
	Challenge challengeHead;
	Mission *next;
};

struct StarSystem {
	char name[MAX_NAME_LENGTH];
	char description[MAX_DESCRIPTION_LENGTH];
	int side;
	int x;
	int y;
	int completedMissions;
	int totalMissions;
	int completedChallenges;
	int totalChallenges;
	Mission missionHead, *missionTail;
	StarSystem *next;
};

struct GridCell {
	Entity *entity;
	GridCell *next;
};

typedef struct {
	int entId;
	SDL_Point camera;
	int numAllies;
	int numEnemies;
	int numInitialEnemies;
	int status;
	int epic;
	int epicFighterLimit;
	int playerSelect;
	int missionFinishedTimer;
	int boostTimer;
	int ecmTimer;
	int numObjectivesComplete, numObjectivesTotal;
	Entity *missionTarget;
	Entity *extractionPoint;
	SDL_Texture *background, *planetTexture;
	PointF planet;
	Entity entityHead, *entityTail;
	Bullet bulletHead, *bulletTail;
	Effect effectHead, *effectTail;
	Objective objectiveHead, *objectiveTail;
	Trigger triggerHead, *triggerTail;
	unsigned int stats[STAT_MAX];
	GridCell grid[GRID_SIZE][GRID_SIZE];
} Battle;

typedef struct {
	StarSystem starSystemHead, *starSystemTail;
	Mission *currentMission;
	char selectedStarSystem[MAX_NAME_LENGTH];
	unsigned int stats[STAT_MAX];
} Game;

struct Widget {
	char name[MAX_NAME_LENGTH];
	char group[MAX_NAME_LENGTH];
	int type;
	int value;
	char text[MAX_NAME_LENGTH];
	char **options;
	int numOptions;
	int currentOption;
	int visible;
	int enabled;
	SDL_Rect rect;
	void (*action)(void);
	void (*onChange)(char *value);
	Widget *next;
};

struct HudMessage {
	char message[MAX_DESCRIPTION_LENGTH];
	SDL_Color color;
	int life;
	HudMessage *next;
};

typedef struct {
	int x;
	int y;
	int w;
	int h;
	int button[MAX_MOUSE_BUTTONS];
} Mouse;

typedef struct {
	char saveDir[MAX_FILENAME_LENGTH];
	int winWidth;
	int winHeight;
	float scaleX;
	float scaleY;
	int fullscreen;
	int musicVolume;
	int soundVolume;
	int vSync;
	int fps;
	Mouse mouse;
	int keyboard[MAX_KEYBOARD_KEYS];
	SDL_Texture *backBuffer;
	SDL_Renderer *renderer;
	SDL_Window *window;
	Delegate delegate;
} App;

typedef struct {
	SDL_Color red;
	SDL_Color orange;
	SDL_Color yellow;
	SDL_Color green;
	SDL_Color blue;
	SDL_Color cyan;
	SDL_Color purple;
	SDL_Color white;
	SDL_Color black;
	SDL_Color lightGrey;
	SDL_Color darkGrey;
} Colors;
