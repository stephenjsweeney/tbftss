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

typedef struct Texture Texture;
typedef struct Lookup Lookup;
typedef struct Weapon Weapon;
typedef struct Quadtree Quadtree;
typedef struct Entity Entity;
typedef struct Bullet Bullet;
typedef struct Debris Debris;
typedef struct Effect Effect;
typedef struct Spawner Spawner;
typedef struct Objective Objective;
typedef struct StarSystem StarSystem;
typedef struct Challenge Challenge;
typedef struct Mission Mission;
typedef struct Pulse Pulse;
typedef struct Widget Widget;
typedef struct HudMessage HudMessage;
typedef struct MessageBox MessageBox;
typedef struct ScriptRunner ScriptRunner;
typedef struct Location Location;
typedef struct Bucket Bucket;
typedef struct Trophy Trophy;
typedef struct Tuple Tuple;
typedef struct Credit Credit;

typedef struct {
	int debug;
	int takeScreenshots;
	char *screenshotFolder;
	int noAIWeapons;
	int showFPS;
	int playerImmortal;
	int playerUnlimitedMissiles;
	int noEntityActions;
	int allImmortal;
	int fps;
} Dev;

typedef struct {
	float x;
	float y;
} PointF;

struct Texture {
	char name[MAX_DESCRIPTION_LENGTH];
	long hash;
	long ttl;
	SDL_Texture *texture;
	Texture *next;
};

typedef struct {
	void (*logic)(void);
	void (*draw)(void);
	void (*handleClick)(int x, int y, int btn);
	void (*handleDrag)(int x, int y, int dx, int dy, int cx, int cy);
	void (*handleMouseUp)(int x, int y, int btn);
} Delegate;

typedef struct {
	int type;
	char message[MAX_DESCRIPTION_LENGTH];
} ModalDialog;

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
	int spawned;
	int id;
	int side;
	float x;
	float y;
	int offsetX;
	int offsetY;
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
	int aiDamagePerSec;
	int aiDamageTimer;
	int aiEvadeTimer;
	int separationRadius;
	int deathType;
	Weapon guns[MAX_FIGHTER_GUNS];
	int missiles;
	long flags;
	long aiFlags;
	SDL_Point targetLocation;
	Entity *towing;
	Entity *target;
	Entity *leader;
	Entity *owner;
	Entity *killedBy;
	void (*action)(void);
	void (*draw)(void);
	void (*die)(void);
	SDL_Texture *texture;
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

struct Debris {
	float x;
	float y;
	float dx;
	float dy;
	int health;
	int thinkTime;
	float angle;
	SDL_Texture *texture;
	Debris *next;
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
	float size;
	float scaleAmount;
	int r;
	int g;
	int b;
	int a;
	SDL_Texture *texture;
	Effect *next;
};

struct Location {
	int active;
	char name[MAX_NAME_LENGTH];
	int x;
	int y;
	int size;
	Location *next;
};

struct Objective {
	int active;
	char id[MAX_DESCRIPTION_LENGTH];
	char description[MAX_DESCRIPTION_LENGTH];
	char targetName[MAX_NAME_LENGTH];
	int targetType;
	int currentValue;
	int targetValue;
	int status;
	int isCondition;
	int isEliminateAll;
	int hideNumbers;
	Objective *next;
};

struct Challenge {
	int type;
	int value;
	int passed;
	Challenge *next;
};

typedef struct {
	int isChallenge;
	int timeLimit;
	int killLimit;
	int lossLimit;
	int itemLimit;
	int playerItemLimit;
	int escapeLimit;
	int waypointLimit;
	int rescueLimit;
	int disableLimit;
	int surrenderLimit;
	int noMissiles;
	int noBoost;
	int noECM;
	int noGuns;
	int allowPlayerDeath;
	int clearWaypointEnemies;
	int eliminateThreats;
	int isDeathMatch;
	Challenge *challenges[MAX_CHALLENGES];
} ChallengeData;

struct Mission {
	char name[MAX_NAME_LENGTH];
	char description[MAX_DESCRIPTION_LENGTH];
	char filename[MAX_DESCRIPTION_LENGTH];
	int requires;
	int requiresOptional;
	char pilot[MAX_NAME_LENGTH];
	char squadron[MAX_NAME_LENGTH];
	char craft[MAX_NAME_LENGTH];
	int available;
	int completed;
	int completedChallenges;
	int totalChallenges;
	int epic;
	int isOptional;
	ChallengeData challengeData;
	SDL_Rect rect;
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
	int availableMissions;
	int fallsToPandorans;
	int type;
	Mission missionHead;
	StarSystem *next;
};

struct Quadtree {
	int depth;
	int x, y, w, h;
	Entity **ents;
	int capacity;
	int numEnts;
	int addedTo;
	Quadtree *node[4];
};

struct Spawner {
	char name[MAX_NAME_LENGTH];
	char **types;
	int numTypes;
	int side;
	int time;
	int interval;
	int total;
	int step;
	int offscreen;
	int active;
	char flags[MAX_DESCRIPTION_LENGTH];
	char aiFlags[MAX_DESCRIPTION_LENGTH];
	Spawner *next;
};

typedef struct {
	SDL_Point camera;
	int numAllies;
	int numEnemies;
	int numInitialEnemies;
	int status;
	int isEpic;
	int unlimitedEnemies;
	int epicFighterLimit;
	int playerSelect;
	int manualComplete;
	int unwinnable;
	int waypointAutoAdvance;
	int missionFinishedTimer;
	int boostTimer;
	int ecmTimer;
	int radarRange;
	int numPlayerGuns;
	int numObjectivesComplete, numObjectivesTotal, numConditions;
	int scriptedEnd;
	int hasThreats;
	int hasSuspicionLevel;
	int suspicionLevel;
	int suspicionCoolOff;
	int zackariaSuspicionLevel;
	Entity *missionTarget;
	Entity *jumpgate;
	Entity *messageSpeaker;
	Entity *lastKilledPlayer;
	SDL_Texture *background, *planetTexture;
	PointF planet;
	int planetWidth, planetHeight;
	Entity entityHead, *entityTail;
	Bullet bulletHead, *bulletTail;
	Debris debrisHead, *debrisTail;
	Effect effectHead, *effectTail;
	Objective objectiveHead, *objectiveTail;
	Location locationHead, *locationTail;
	Spawner spawnerHead, *spawnerTail;
	struct cJSON *missionJSON;
	unsigned int stats[STAT_MAX];
	Quadtree quadtree;
} Battle;

struct ScriptRunner {
	struct cJSON *line;
	long delay;
	int waitForMessageBox;
	ScriptRunner *next;
};

struct Trophy {
	char id[MAX_NAME_LENGTH];
	char title[MAX_DESCRIPTION_LENGTH];
	char description[MAX_DESCRIPTION_LENGTH];
	char awardDateStr[MAX_NAME_LENGTH];
	int value;
	int hidden;
	int stat;
	int statValue;
	int awarded;
	unsigned long awardDate;
	int notify;
	Trophy *next;
};

typedef struct {
	int friendlyFire;
	int autoSwitchPlayerTarget;
	int missileReTarget;
	int healthBars;
} Gameplay;

struct Tuple {
	char key[MAX_NAME_LENGTH];
	int value;
	Tuple *next;
};

typedef struct {
	StarSystem starSystemHead;
	Mission challengeMissionHead;
	Mission *currentMission;
	char selectedStarSystem[MAX_NAME_LENGTH];
	int completedMissions;
	int availableMissions;
	int totalMissions;
	int completedChallenges;
	int totalChallenges;
	unsigned int stats[STAT_MAX];
	Trophy trophyHead;
	Tuple fighterStatHead;
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
	int isModal;
	SDL_Rect rect;
	SDL_Texture *texture;
	void (*action)(void);
	void (*onChange)(char *value);
	Widget *parent;
	Widget *next;
};

struct HudMessage {
	char message[MAX_DESCRIPTION_LENGTH];
	SDL_Color color;
	int life;
	HudMessage *next;
};

struct MessageBox {
	char title[MAX_NAME_LENGTH];
	char body[MAX_DESCRIPTION_LENGTH];
	int time;
	int height;
	int type;
	MessageBox *next;
};

typedef struct {
	int x;
	int y;
	int w;
	int h;
	int dx;
	int dy;
	int button[MAX_MOUSE_BUTTONS];
} Mouse;

typedef struct {
	int resetTimeDelta;
	char saveDir[MAX_FILENAME_LENGTH];
	int saveGame;
	int winWidth;
	int winHeight;
	float scaleX;
	float scaleY;
	int fullscreen;
	int musicVolume;
	int soundVolume;
	Gameplay gameplay;
	Mouse mouse;
	int keyboard[MAX_KEYBOARD_KEYS];
	SDL_Texture *backBuffer;
	SDL_Renderer *renderer;
	SDL_Window *window;
	Delegate delegate;
	ModalDialog modalDialog;
	int awaitingWidgetInput;
	int lastKeyPressed;
	int lastButtonPressed;
	int keyControls[CONTROL_MAX];
	int mouseControls[CONTROL_MAX];
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

struct Credit {
	char *text;
	float y;
	int size;
	int h;
	Credit *next;
};

struct Bucket {
	char *key, *value;
	Bucket *next;
};

typedef struct {
	Bucket **bucket;
	int *bucketCount;
} HashTable;

typedef struct {
	int32_t magicNumber, version, stringCount;
	int32_t originalOffset, translationOffset;
} MOHeader;

typedef struct {
	int32_t length, offset;
} MOEntry;
