#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./SDL2-2.0.10/include/SDL.h"
#include "./SDL2-2.0.10/include/SDL_main.h"

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;  s.h = 8;
	d.w = 8;  d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;  s.y = py;
		d.x = x;   d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	}
}

// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
}

// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
}

// draw a vertical (dx=0,dy=1) or horizontal (dx=1,dy=0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	}
}

// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(int i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
}

void NewGame(double &stageTime, double &cameraX, double &cameraY, double &playerX, double &playerY){
	stageTime = 0.0;
	cameraX = 0.0;
	cameraY = 0.0;
	playerX = 200.0;
	playerY = 350.0;
}

// rect type for attack hitbox (world coords)
struct RectD { double x, y, w, h; };

struct GameObject {
	int type; // 0 = box, 1 = spikes
	double x, y; // FEET contact point in world coords
	double hitbox_w, hitbox_h; // hitbox size
	int boxHP;     // only for box
	bool alive;

	// prevent damage every frame per single attack press
	int lastHitAttackId;
};

// overlap
bool RectOverlap(double ax, double ay, double aw, double ah,
                 double bx, double by, double bw, double bh) {
	return (ax < bx + bw) && (ax + aw > bx) && (ay < by + bh) && (ay + ah > by);
}

// ---------- COMBO / INPUT BUFFER ----------

enum InputCmd {
	CMD_NONE = 0,
	CMD_X,	// jump button (X)
	CMD_Z,	// light attack (Z)
	CMD_Y,	// heavy attack (Y)
	CMD_L,	// move left tap (A/Left)
	CMD_R,	// move right tap (D/Right)
	CMD_U,	// move up tap (W/Up)
	CMD_D,	// move down tap (S/Down)
};

const char* CmdName(InputCmd c) {
	switch (c) {
		case CMD_X: return "X";
		case CMD_Z: return "Z";
		case CMD_Y: return "Y";
		case CMD_L: return "L";
		case CMD_R: return "R";
		case CMD_U: return "U";
		case CMD_D: return "D";
		default: return "_";
	}
}

struct InputEvent {
	InputCmd cmd;
	double t; // seconds (stageTime)
};

struct InputBuffer {
	enum { CAP = 16 };
	InputEvent e[CAP];
	int count;
};

void BufPush(InputBuffer* b, InputCmd cmd, double now) {
	if (b->count < InputBuffer::CAP) b->count++;
	for (int i = b->count - 1; i > 0; --i) b->e[i] = b->e[i - 1];
	b->e[0].cmd = cmd;
	b->e[0].t = now;
}

bool BufMatch(const InputBuffer* b, const InputCmd* p, int n, double now, double windowSec) {
	if (b->count < n) return false;
	for (int i = 0; i < n; i++) {
		if (b->e[i].cmd != p[i]) return false;
		if (now - b->e[i].t > windowSec) return false;
	}
	return true;
}

enum ComboAction {
	CA_NONE = 0,
	CA_TRIPLE_JUMP,	// XXX
	CA_FURY,        // YYY
	CA_UPPERCUT,    // X Y X
	CA_DASH,        // double-tap direction
};

const char* ActionName(ComboAction a) {
	switch (a) {
		case CA_TRIPLE_JUMP: return "TRIPLE_JUMP";
		case CA_FURY: return "FURY";
		case CA_UPPERCUT: return "UPPERCUT";
		case CA_DASH: return "DASH";
		default: return "NONE";
	}
}

double Rand01() { return rand() / (double)RAND_MAX; }
double RandRange(double a, double b) { return a + (b - a) * Rand01(); }

int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc;
	double delta, stageTime, fpsTimer, fps;

	SDL_Event event;
	SDL_Surface *screen, *charset;

	// player sprites
	SDL_Surface *sprStand, *sprWalk1, *sprWalk2;
	SDL_Surface *sprJump, *sprAttackHeavy, *sprAttackLight;
	SDL_Surface *sprHurt;

	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	printf("printf output goes here\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	srand((unsigned)SDL_GetTicks());

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_ShowCursor(SDL_DISABLE);

	// charset
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	SDL_SetColorKey(charset, true, 0x000000);

	// load sprites
	sprStand = SDL_LoadBMP("./player_stand.bmp");
	sprWalk1 = SDL_LoadBMP("./player_walk1.bmp");
	sprWalk2 = SDL_LoadBMP("./player_walk2.bmp");
	sprJump  = SDL_LoadBMP("./player_jump.bmp");
	sprAttackHeavy = SDL_LoadBMP("./player_attack.bmp");
	sprAttackLight = SDL_LoadBMP("./player_attack2.bmp");
	sprHurt = SDL_LoadBMP("./player_hurt.bmp");

	if(!sprStand || !sprWalk1 || !sprWalk2 || !sprJump || !sprAttackHeavy || !sprAttackLight || !sprHurt) {
		printf("SDL_LoadBMP(player sprites) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}

	// transparency key (pink)
	Uint32 key = SDL_MapRGB(sprStand->format, 255, 0, 243);
	SDL_SetColorKey(sprStand, SDL_TRUE, key);
	SDL_SetColorKey(sprWalk1, SDL_TRUE, key);
	SDL_SetColorKey(sprWalk2, SDL_TRUE, key);
	SDL_SetColorKey(sprJump, SDL_TRUE, key);
	SDL_SetColorKey(sprAttackHeavy, SDL_TRUE, key);
	SDL_SetColorKey(sprAttackLight, SDL_TRUE, key);
	SDL_SetColorKey(sprHurt, SDL_TRUE, key);

	char text[512];
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	// ---- stage ----
	const double STAGE_W = 2000.0;
	const int FLOOR_Y = 260;
	const int FLOOR_H = 200;

	// ---- player (playerX,playerY are FEET coords) ----
	double playerX = 200.0;
	double playerY = FLOOR_Y + FLOOR_H / 2.0;
	double playerSpeed = 260.0;

	// jump tuning
	double gravity = 2200.0;
	double jumpVel = 950.0;

	// action tuning
	double lightAttackDuration = 0.18;
	double heavyAttackDuration = 0.35;

	// hitboxes (big)
	double lightRange = 160.0;
	double lightHeight = 120.0;
	double heavyRange = 240.0;
	double heavyHeight = 160.0;

	double lightMoveScale = 0.55;
	double heavyMoveScale = 0.25;

	// jump state
	bool inJump = false;
	double z = 0.0;
	double vz = 0.0;

	// hurt state
	bool inHurt = false;
	double hurtTimer = 0.0;
	const double HURT_DURATION = 0.25;

	// attack state
	enum Action { ACT_NONE, ACT_LIGHT, ACT_HEAVY };
	Action action = ACT_NONE;
	double actionTimer = 0.0;

	// unique id per attack press
	int attackId = 0;

	// -------- combo input system --------
	InputBuffer inputBuf; inputBuf.count = 0;

	bool devMode = false;
	ComboAction currentComboAction = CA_NONE;
	double comboActionTimer = 0.0;

	// dash tuning
	double dashSpeed = 1100.0;
	double dashDuration = 0.12;
	double dashVX = 0.0, dashVY = 0.0;

	// buffer tuning
	const double INPUT_WINDOW = 0.55;
	const double DOUBLE_TAP_WINDOW = 0.28;

	// score + combo
	int score = 0;
	int comboCount = 0;
	double comboTimer = 0.0;
	const double COMBO_WINDOW = 1.0;

	// player HP
	int playerHP = 100;
	int playerHPMax = 100;

	// spikes damage cooldown
	double spikeCooldown = 0.0;
	const double SPIKE_COOLDOWN_TIME = 0.6;

	// objects (ALWAYS 2 boxes + 1 spikes)
	const int MAX_OBJ = 3;
	GameObject objs[MAX_OBJ];
	int objCount = 0;

	// spawn helpers (big)
	auto AddBox = [&](double x, double feetY) {
		if (objCount >= MAX_OBJ) return;
		objs[objCount].type = 0;
		objs[objCount].x = x;
		objs[objCount].y = feetY;
		objs[objCount].hitbox_w = 220.0;
		objs[objCount].hitbox_h = 180.0;
		objs[objCount].boxHP = 6;
		objs[objCount].alive = true;
		objs[objCount].lastHitAttackId = -1;
		objCount++;
	};
	auto AddSpikes = [&](double x, double feetY) {
		if (objCount >= MAX_OBJ) return;
		objs[objCount].type = 1;
		objs[objCount].x = x;
		objs[objCount].y = feetY;
		objs[objCount].hitbox_w = 240.0;
		objs[objCount].hitbox_h = 70.0;
		objs[objCount].boxHP = 0;
		objs[objCount].alive = true;
		objs[objCount].lastHitAttackId = -1;
		objCount++;
	};

	double objMinFeetY = FLOOR_Y + 30.0;
	double objMaxFeetY = FLOOR_Y + FLOOR_H;
	double minX = 200.0;
	double maxX = STAGE_W - 200.0;

	auto RespawnObjects = [&]() {
		objCount = 0;
		AddBox(RandRange(minX, maxX), RandRange(objMinFeetY, objMaxFeetY));
		AddBox(RandRange(minX, maxX), RandRange(objMinFeetY, objMaxFeetY));
		AddSpikes(RandRange(minX, maxX), RandRange(objMinFeetY, objMaxFeetY));
	};
	RespawnObjects();

	// camera
	double cameraX = 0.0;
	const int DEAD_LEFT = 220;
	const int DEAD_RIGHT = 420;

	// walk animation
	double animTimer = 0.0;
	int animStep = 0;
	const double ANIM_STEP_TIME = 0.12;

	// time init
	t1 = SDL_GetTicks();
	stageTime = 0.0;
	fpsTimer = 0.0;
	fps = 0.0;
	frames = 0;
	quit = 0;

	// Evaluate combos (pattern table)
	auto TryStartComboAction = [&](double now) -> void {
		if (comboActionTimer > 0.0) return;

		{ // XXX => TRIPLE JUMP
			InputCmd p[] = { CMD_X, CMD_X, CMD_X };
			if (BufMatch(&inputBuf, p, 3, now, INPUT_WINDOW)) {
				currentComboAction = CA_TRIPLE_JUMP;
				comboActionTimer = 0.30;
				return;
			}
		}

		{ // YYY => FURY
			InputCmd p[] = { CMD_Y, CMD_Y, CMD_Y };
			if (BufMatch(&inputBuf, p, 3, now, INPUT_WINDOW)) {
				currentComboAction = CA_FURY;
				comboActionTimer = 0.55; // lasts a bit
				return;
			}
		}

		{ // X Y X => UPPERCUT
			InputCmd p[] = { CMD_X, CMD_Y, CMD_X };
			if (BufMatch(&inputBuf, p, 3, now, INPUT_WINDOW)) {
				currentComboAction = CA_UPPERCUT;
				comboActionTimer = 0.40;
				return;
			}
		}

		{ // dash: double tap direction
			InputCmd pR[] = { CMD_R, CMD_R };
			InputCmd pL[] = { CMD_L, CMD_L };
			InputCmd pU[] = { CMD_U, CMD_U };
			InputCmd pD[] = { CMD_D, CMD_D };

			if (BufMatch(&inputBuf, pR, 2, now, DOUBLE_TAP_WINDOW)) {
				currentComboAction = CA_DASH;
				comboActionTimer = dashDuration;
				dashVX = +1.0; dashVY = 0.0;
				return;
			}
			if (BufMatch(&inputBuf, pL, 2, now, DOUBLE_TAP_WINDOW)) {
				currentComboAction = CA_DASH;
				comboActionTimer = dashDuration;
				dashVX = -1.0; dashVY = 0.0;
				return;
			}
			if (BufMatch(&inputBuf, pU, 2, now, DOUBLE_TAP_WINDOW)) {
				currentComboAction = CA_DASH;
				comboActionTimer = dashDuration;
				dashVX = 0.0; dashVY = -1.0;
				return;
			}
			if (BufMatch(&inputBuf, pD, 2, now, DOUBLE_TAP_WINDOW)) {
				currentComboAction = CA_DASH;
				comboActionTimer = dashDuration;
				dashVX = 0.0; dashVY = +1.0;
				return;
			}
		}
	};

	while(!quit) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		stageTime += delta;

		// ---- FIX: combo-action timer update MUST be inside the loop ----
		if (comboActionTimer > 0.0) {
			comboActionTimer -= delta;
			if (comboActionTimer <= 0.0) {
				comboActionTimer = 0.0;
				currentComboAction = CA_NONE;   // FIX typo
				dashVX = dashVY = 0.0;
			}
		}

		// combo score timer decay
		if (comboTimer > 0.0) {
			comboTimer -= delta;
			if (comboTimer <= 0.0) {
				comboTimer = 0.0;
				comboCount = 0;
			}
		}

		// spikes cooldown
		if (spikeCooldown > 0.0) {
			spikeCooldown -= delta;
			if (spikeCooldown < 0.0) spikeCooldown = 0.0;
		}

		// hurt timer
		if (inHurt) {
			hurtTimer -= delta;
			if (hurtTimer <= 0.0) {
				inHurt = false;
				hurtTimer = 0.0;
			}
		}

		// jump physics
		if (inJump) {
			vz -= gravity * delta;
			z  += vz * delta;
			if (z <= 0.0) {
				z = 0.0;
				vz = 0.0;
				inJump = false;
			}
		}

		// attack timer
		if (action != ACT_NONE) {
			actionTimer -= delta;
			if (actionTimer <= 0.0) {
				action = ACT_NONE;
				actionTimer = 0.0;
			}
		}

		// --- movement input ---
		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		double vx = 0.0, vy = 0.0;
		if(keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A]) vx -= 1.0;
		if(keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) vx += 1.0;
		if(keys[SDL_SCANCODE_UP]    || keys[SDL_SCANCODE_W]) vy -= 1.0;
		if(keys[SDL_SCANCODE_DOWN]  || keys[SDL_SCANCODE_S]) vy += 1.0;

		double len = sqrt(vx*vx + vy*vy);
		if(len > 0.0) { vx /= len; vy /= len; }
		bool moving = (len > 0.0);

		// movement reduced while attacking
		double speedScale = 1.0;
		if (action == ACT_LIGHT) speedScale = lightMoveScale;
		if (action == ACT_HEAVY) speedScale = heavyMoveScale;
		if (inHurt) speedScale = 0.0;

		// dash overrides movement
		if (currentComboAction == CA_DASH && comboActionTimer > 0.0) {
			playerX += dashVX * dashSpeed * delta;
			playerY += dashVY * dashSpeed * delta;
		} else {
			playerX += vx * playerSpeed * speedScale * delta;
			playerY += vy * playerSpeed * speedScale * delta;
		}

		// walking animation
		if (moving && action == ACT_NONE && !inJump && !inHurt) {
			animTimer += delta;
			while (animTimer >= ANIM_STEP_TIME) {
				animTimer -= ANIM_STEP_TIME;
				animStep = (animStep + 1) % 4;
			}
		} else {
			animTimer = 0.0;
			animStep = 0;
		}

		// clamp player X to stage
		if (playerX < 0) playerX = 0;
		if (playerX > STAGE_W) playerX = STAGE_W;

		// clamp player Y to floor lane
		double footTop = FLOOR_Y + 30;
		double footBottom = FLOOR_Y + FLOOR_H;
		if (playerY < footTop) playerY = footTop;
		if (playerY > footBottom) playerY = footBottom;

		// camera follow (X only)
		double playerScreenX = playerX - cameraX;
		if (playerScreenX < DEAD_LEFT)  cameraX = playerX - DEAD_LEFT;
		if (playerScreenX > DEAD_RIGHT) cameraX = playerX - DEAD_RIGHT;
		if (cameraX < 0) cameraX = 0;
		if (cameraX > STAGE_W - SCREEN_WIDTH) cameraX = STAGE_W - SCREEN_WIDTH;

		// choose sprite
		SDL_Surface* currentSprite = sprStand;
		if (inHurt) currentSprite = sprHurt;
		else if (action == ACT_LIGHT) currentSprite = sprAttackLight;
		else if (action == ACT_HEAVY) currentSprite = sprAttackHeavy;
		else if (inJump) currentSprite = sprJump;
		else {
			if (moving) {
				if (animStep == 1) currentSprite = sprWalk1;
				else if (animStep == 3) currentSprite = sprWalk2;
				else currentSprite = sprStand;
			} else {
				currentSprite = sprStand;
			}
		}

		// ---- ATTACK HITBOX ----
		RectD attackBox = {0,0,0,0};
		bool attackActive = (action != ACT_NONE);

		if (attackActive) {
			double range  = (action == ACT_LIGHT) ? lightRange  : heavyRange;
			double height = (action == ACT_LIGHT) ? lightHeight : heavyHeight;

			attackBox.w = range;
			attackBox.h = height;

			double frontOffset = 60.0;
			attackBox.x = playerX + frontOffset;
			attackBox.y = playerY - height;
		}

		// ---- PLAYER FEET HITBOX ----
		double playerFootW = 80.0;
		double playerFootH = 40.0;
		double pBoxX = playerX - playerFootW / 2.0;
		double pBoxY = playerY - playerFootH;
		double pBoxW = playerFootW;
		double pBoxH = playerFootH;

		// ---- OBJECT INTERACTIONS ----
		for (int i = 0; i < objCount; i++) {
			if (!objs[i].alive) continue;

			double oLeft = objs[i].x - objs[i].hitbox_w / 2.0;
			double oTop  = objs[i].y - objs[i].hitbox_h;
			double oW    = objs[i].hitbox_w;
			double oH    = objs[i].hitbox_h;

			// BOX: hit by attack
			if (objs[i].type == 0) {
				if (attackActive) {
					if (RectOverlap(attackBox.x, attackBox.y, attackBox.w, attackBox.h,
					                oLeft, oTop, oW, oH)) {

						if (objs[i].lastHitAttackId != attackId) {
							objs[i].lastHitAttackId = attackId;

							// base damage
							int dmg = (action == ACT_LIGHT) ? 1 : 2;

							// combo-action bonuses
							if (currentComboAction == CA_FURY) dmg += 1;
							if (currentComboAction == CA_UPPERCUT) dmg += 2;

							objs[i].boxHP -= dmg;

							int base = (action == ACT_LIGHT) ? 10 : 15;
							int multiplier = 1 + comboCount;
							score += base * multiplier;

							comboCount += 1;
							comboTimer = COMBO_WINDOW;

							if (objs[i].boxHP <= 0) {
								objs[i].alive = false;
								score += 50 * (1 + comboCount);
							}
						}
					}
				}
			}

			// SPIKES: damage player when FEET overlap
			if (objs[i].type == 1) {
				if (RectOverlap(pBoxX, pBoxY, pBoxW, pBoxH, oLeft, oTop, oW, oH)) {
					if (spikeCooldown <= 0.0) {
						playerHP -= 10;
						if (playerHP < 0) playerHP = 0;

						comboCount = 0;
						comboTimer = 0.0;

						inHurt = true;
						hurtTimer = HURT_DURATION;

						spikeCooldown = SPIKE_COOLDOWN_TIME;
					}
				}
			}
		}

		// ---- DRAW ----
		int sky      = SDL_MapRGB(screen->format, 25, 25, 55);
		int floorCol = SDL_MapRGB(screen->format, 85, 95, 85);
		int floorEdge= SDL_MapRGB(screen->format, 25, 25, 30);

		SDL_FillRect(screen, NULL, sky);

		// floor
		DrawRectangle(screen, 0, FLOOR_Y, SCREEN_WIDTH, FLOOR_H, floorEdge, floorCol);

		// objects
		int boxOut = SDL_MapRGB(screen->format, 200, 200, 200);
		int boxIn  = SDL_MapRGB(screen->format, 90, 90, 90);
		int spikeOut = SDL_MapRGB(screen->format, 255, 80, 80);
		int spikeIn  = SDL_MapRGB(screen->format, 140, 30, 30);

		for (int i = 0; i < objCount; i++) {
			if (!objs[i].alive) continue;

			int sx = (int)(objs[i].x - cameraX);
			int syFeet = (int)(objs[i].y);
			int w = (int)objs[i].hitbox_w;
			int h = (int)objs[i].hitbox_h;

			int left = sx - w/2;
			int top  = syFeet - h;

			if (objs[i].type == 0) DrawRectangle(screen, left, top, w, h, boxOut, boxIn);
			else DrawRectangle(screen, left, top, w, h, spikeOut, spikeIn);
		}

		// player (playerY is FEET, so centerY = feet - h/2)
		int px = (int)(playerX - cameraX);
		int pyCenter = (int)(playerY - z) - currentSprite->h / 2;
		DrawSurface(screen, currentSprite, px, pyCenter);

		// FPS calc
		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		}

		// UI panel
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 52, czerwony, niebieski);

		sprintf(text, "time=%.1lf | fps=%.0lf | score=%d | combo=%d", stageTime, fps, score, comboCount);
		DrawString(screen, 10, 10, text, charset);

		sprintf(text, "Esc quit | N new | X jump | Z light | Y heavy | F1 dev");
		DrawString(screen, 10, 26, text, charset);

		// HP bar
		int barX = 10, barY = 42, barW = 180, barH = 10;
		int hpOut = SDL_MapRGB(screen->format, 220, 220, 220);
		int hpBack = SDL_MapRGB(screen->format, 40, 40, 40);
		int hpFill = SDL_MapRGB(screen->format, 40, 220, 40);

		DrawRectangle(screen, barX, barY, barW, barH, hpOut, hpBack);
		int fillW = (int)((barW - 2) * (playerHP / (double)playerHPMax));
		if (fillW < 0) fillW = 0;
		DrawRectangle(screen, barX + 1, barY + 1, fillW, barH - 2, hpFill, hpFill);

		DrawString(screen, barX + barW + 8, barY - 1, "Player Health", charset);
		sprintf(text, "%d/%d", playerHP, playerHPMax);
		DrawString(screen, barX + barW - 48, barY - 1, text, charset);

		// DEV MODE: show buffer + current combo action
		if (devMode) {
			int y0 = 64;
			sprintf(text, "DEV: action=%s (%.2fs)", ActionName(currentComboAction), comboActionTimer);
			DrawString(screen, 10, y0, text, charset);

			char bufLine[512];
			strcpy(bufLine, "BUF: ");
			for (int i = 0; i < inputBuf.count; i++) {
				char part[32];
				double age = stageTime - inputBuf.e[i].t;
				sprintf(part, "%s(%.2f) ", CmdName(inputBuf.e[i].cmd), age);
				strcat(bufLine, part);
			}
			DrawString(screen, 10, y0 + 12, bufLine, charset);
		}

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// ---- events ----
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;

					// dev mode
					if(event.key.keysym.sym == SDLK_F1 && event.key.repeat == 0) {
						devMode = !devMode;
					}

					// new game
					else if(event.key.keysym.sym == SDLK_n) {
						NewGame(stageTime, cameraX, (double&)cameraX /*unused*/, playerX, playerY);

						playerHP = playerHPMax;
						score = 0;
						comboCount = 0;
						comboTimer = 0.0;

						action = ACT_NONE; actionTimer = 0.0;
						inJump = false; z = 0.0; vz = 0.0;

						inHurt = false; hurtTimer = 0.0;
						spikeCooldown = 0.0;

						currentComboAction = CA_NONE;
						comboActionTimer = 0.0;
						dashVX = dashVY = 0.0;

						inputBuf.count = 0;

						RespawnObjects();
					}

					// one-tap inputs for buffer (dash detection)
					else if(event.key.repeat == 0) {
						SDL_Keycode k = event.key.keysym.sym;

						// movement taps -> buffer
						if (k == SDLK_a || k == SDLK_LEFT)  { BufPush(&inputBuf, CMD_L, stageTime); TryStartComboAction(stageTime); }
						if (k == SDLK_d || k == SDLK_RIGHT) { BufPush(&inputBuf, CMD_R, stageTime); TryStartComboAction(stageTime); }
						if (k == SDLK_w || k == SDLK_UP)    { BufPush(&inputBuf, CMD_U, stageTime); TryStartComboAction(stageTime); }
						if (k == SDLK_s || k == SDLK_DOWN)  { BufPush(&inputBuf, CMD_D, stageTime); TryStartComboAction(stageTime); }

						// X: jump + buffer + triple jump effect
						if(k == SDLK_x) {
							BufPush(&inputBuf, CMD_X, stageTime);
							TryStartComboAction(stageTime);

							double usedJumpVel = jumpVel;
							if (currentComboAction == CA_TRIPLE_JUMP) usedJumpVel = jumpVel * 1.35;

							if(!inJump) {
								inJump = true;
								vz = usedJumpVel;
							}
						}

						// Z: light attack + buffer
						else if(k == SDLK_z) {
							BufPush(&inputBuf, CMD_Z, stageTime);
							TryStartComboAction(stageTime);

							if(action == ACT_NONE) {
								action = ACT_LIGHT;
								actionTimer = lightAttackDuration;
								attackId++;
							}
						}

						// Y: heavy attack + buffer (+ uppercut can be treated as buffed heavy)
						else if(k == SDLK_y) {
							BufPush(&inputBuf, CMD_Y, stageTime);
							TryStartComboAction(stageTime);

							if(action == ACT_NONE) {
								action = ACT_HEAVY;
								actionTimer = heavyAttackDuration;
								attackId++;
							}
						}
					}
					break;

				case SDL_QUIT:
					quit = 1;
					break;
			}
		}
		frames++;
	}

	// free surfaces
	SDL_FreeSurface(sprStand);
	SDL_FreeSurface(sprWalk1);
	SDL_FreeSurface(sprWalk2);
	SDL_FreeSurface(sprJump);
	SDL_FreeSurface(sprAttackHeavy);
	SDL_FreeSurface(sprAttackLight);
	SDL_FreeSurface(sprHurt);
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);

	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
