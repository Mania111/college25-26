// ============================================================================
// Beat 'em up (SDL2) - Project code (cleaned + academically commented)
// Key requirement addressed:
//   "Character movement and control parameters should be easy to change"
// Implementation:
//   - All feel/tuning parameters are centralized in GameTuning (one place).
//   - Movement uses acceleration + friction (smooth control; easy to tune).
// ============================================================================

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./SDL2-2.0.10/include/SDL.h"
#include "./SDL2-2.0.10/include/SDL_main.h"

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

// ----------------------------------------------------------------------------
//  Drawing helpers (provided style)
// ----------------------------------------------------------------------------

// Draw text using a charset (8x8 per character), starting at (x,y).
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;  s.h = 8;
	d.w = 8;  d.h = 8;
	while (*text) {
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

// Draw a sprite surface centered at (x,y).
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
}

void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
}

void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	}
}

void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (int i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
}

// ----------------------------------------------------------------------------
//  Small utility + math
// ----------------------------------------------------------------------------

struct RectD { double x, y, w, h; };

bool RectOverlap(double ax, double ay, double aw, double ah,
                 double bx, double by, double bw, double bh) {
	return (ax < bx + bw) && (ax + aw > bx) && (ay < by + bh) && (ay + ah > by);
}

double Rand01() { return rand() / (double)RAND_MAX; }
double RandRange(double a, double b) { return a + (b - a) * Rand01(); }

// Approach value v towards target by at most maxDelta this frame.
static double Approach(double v, double target, double maxDelta) {
	if (v < target) {
		v += maxDelta;
		if (v > target) v = target;
	} else if (v > target) {
		v -= maxDelta;
		if (v < target) v = target;
	}
	return v;
}

// ----------------------------------------------------------------------------
//  Game tuning (ALL feel/control parameters in one place)
//  This directly satisfies the requirement: "easy to change to achieve smooth control."
// ----------------------------------------------------------------------------
typedef struct GameTuning {
	// Smooth movement feel (acceleration model in the floor lane)
	double moveMaxSpeed;   // px/s
	double moveAccel;      // px/s^2 (how quickly you reach max speed)
	double moveFriction;   // px/s^2 (how quickly you stop when no input)

	// Speed multipliers under states
	double lightMoveScale;
	double heavyMoveScale;
	double hurtMoveScale;

	// Jump physics (visual Z axis)
	double gravity;        // px/s^2
	double jumpVel;        // px/s
	double tripleJumpScale;

	// Attack timing
	double lightAttackDuration;
	double heavyAttackDuration;

	// Attack hitboxes (player always attacks to the RIGHT)
	double lightRange, lightHeight;
	double heavyRange, heavyHeight;
	double attackFrontOffset;

	// Player "feet" hitbox (used for stepping on spikes)
	double footBoxW, footBoxH;

	// Spikes cooldown (prevents rapid HP melting)
	double spikeCooldownTime;

	// Dash + combo buffer windows
	double dashSpeed;
	double dashDuration;
	double inputWindow;
	double doubleTapWindow;

	// Camera dead zone (X follow)
	int deadLeft;
	int deadRight;

	// Floor lane clamp
	double footTopOffset; // lane top = FLOOR_Y + offset
} GameTuning;

static GameTuning TuningDefault() {
	GameTuning g;
	// Smooth feel defaults (edit here to tune)
	g.moveMaxSpeed   = 260.0;
	g.moveAccel      = 2600.0;
	g.moveFriction   = 3200.0;

	g.lightMoveScale = 0.55;
	g.heavyMoveScale = 0.25;
	g.hurtMoveScale  = 0.0;

	g.gravity        = 2200.0;
	g.jumpVel        = 950.0;
	g.tripleJumpScale = 1.35;

	g.lightAttackDuration = 0.18;
	g.heavyAttackDuration = 0.35;

	// Big hitboxes (easier to see results in short demo)
	g.lightRange   = 160.0; g.lightHeight = 120.0;
	g.heavyRange   = 240.0; g.heavyHeight = 160.0;
	g.attackFrontOffset = 60.0;

	g.footBoxW = 80.0;
	g.footBoxH = 40.0;

	g.spikeCooldownTime = 0.60;

	g.dashSpeed = 1100.0;
	g.dashDuration = 0.12;
	g.inputWindow = 0.55;
	g.doubleTapWindow = 0.28;

	g.deadLeft = 220;
	g.deadRight = 420;

	g.footTopOffset = 30.0;
	return g;
}

// ----------------------------------------------------------------------------
//  World objects (boxes and spikes)
// ----------------------------------------------------------------------------
struct GameObject {
	int type;          // 0 = box, 1 = spikes
	double x, y;       // FEET contact point (world coords)
	double hitbox_w, hitbox_h;
	int boxHP;         // boxes only
	bool alive;

	// Prevent dealing damage every frame for a single attack press.
	int lastHitAttackId;
};

// ----------------------------------------------------------------------------
//  Input buffer / combos (developer-mode visible)
// ----------------------------------------------------------------------------
enum InputCmd {
	CMD_NONE = 0,
	CMD_X, // jump
	CMD_Z, // light attack
	CMD_Y, // heavy attack
	CMD_L, // left tap
	CMD_R, // right tap
	CMD_U, // up tap
	CMD_D  // down tap
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
		default:    return "_";
	}
}

struct InputEvent {
	InputCmd cmd;
	double t; // time in seconds (stageTime)
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

// Match newest-first pattern p[0..n-1] within a time window.
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
	CA_TRIPLE_JUMP, // XXX
	CA_FURY,        // YYY
	CA_UPPERCUT,    // X Y X
	CA_DASH         // double-tap direction
};

const char* ActionName(ComboAction a) {
	switch (a) {
		case CA_TRIPLE_JUMP: return "TRIPLE_JUMP";
		case CA_FURY:        return "FURY";
		case CA_UPPERCUT:    return "UPPERCUT";
		case CA_DASH:        return "DASH";
		default:             return "NONE";
	}
}

// ----------------------------------------------------------------------------
//  Reset helper (kept simple)
// ----------------------------------------------------------------------------
void NewGame(double &stageTime, double &cameraX, double &cameraY, double &playerX, double &playerY) {
	stageTime = 0.0;
	cameraX = 0.0;
	cameraY = 0.0;
	playerX = 200.0;
	playerY = 350.0;
}

// ----------------------------------------------------------------------------
//  Main
// ----------------------------------------------------------------------------
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc;
	double delta, stageTime, fpsTimer, fps;

	SDL_Event event;
	SDL_Surface *screen, *charset;

	SDL_Surface *sprStand, *sprWalk1, *sprWalk2;
	SDL_Surface *sprJump, *sprAttackHeavy, *sprAttackLight;
	SDL_Surface *sprHurt;

	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	srand((unsigned)SDL_GetTicks());

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	if (rc != 0) {
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_ShowCursor(SDL_DISABLE);

	// Charset (8x8 bitmap font)
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (!charset) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}
	SDL_SetColorKey(charset, true, 0x000000);

	// Load sprites (BMP + colorkey transparency)
	sprStand = SDL_LoadBMP("./player_stand.bmp");
	sprWalk1 = SDL_LoadBMP("./player_walk1.bmp");
	sprWalk2 = SDL_LoadBMP("./player_walk2.bmp");
	sprJump  = SDL_LoadBMP("./player_jump.bmp");
	sprAttackHeavy = SDL_LoadBMP("./player_attack.bmp");
	sprAttackLight = SDL_LoadBMP("./player_attack2.bmp");
	sprHurt = SDL_LoadBMP("./player_hurt.bmp");

	if (!sprStand || !sprWalk1 || !sprWalk2 || !sprJump || !sprAttackHeavy || !sprAttackLight || !sprHurt) {
		printf("SDL_LoadBMP(player sprites) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// Pink colorkey (255,0,243) treated as transparent
	Uint32 key = SDL_MapRGB(sprStand->format, 255, 0, 243);
	SDL_SetColorKey(sprStand, SDL_TRUE, key);
	SDL_SetColorKey(sprWalk1, SDL_TRUE, key);
	SDL_SetColorKey(sprWalk2, SDL_TRUE, key);
	SDL_SetColorKey(sprJump, SDL_TRUE, key);
	SDL_SetColorKey(sprAttackHeavy, SDL_TRUE, key);
	SDL_SetColorKey(sprAttackLight, SDL_TRUE, key);
	SDL_SetColorKey(sprHurt, SDL_TRUE, key);

	// UI colors
	char text[512];
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	// ------------------------------------------------------------------------
	// Stage (world)
	// ------------------------------------------------------------------------
	const double STAGE_W = 2000.0;
	const int FLOOR_Y = 260;
	const int FLOOR_H = 200;

	// Centralized gameplay tuning (edit here for "smooth control")
	GameTuning g = TuningDefault();

	// ------------------------------------------------------------------------
	// Player state
	// NOTE: playerX/playerY represent the FEET contact point on the floor lane.
	// ------------------------------------------------------------------------
	double playerX = 200.0;
	double playerY = FLOOR_Y + FLOOR_H / 2.0;

	// Smooth movement uses velocity and acceleration:
	double velX = 0.0;
	double velY = 0.0;

	// Jump state (visual Z)
	bool inJump = false;
	double z = 0.0;
	double vz = 0.0;

	// Hurt state (briefly freezes movement + shows OW sprite)
	bool inHurt = false;
	double hurtTimer = 0.0;
	const double HURT_DURATION = 0.25;

	// Attack state
	enum Action { ACT_NONE, ACT_LIGHT, ACT_HEAVY };
	Action action = ACT_NONE;
	double actionTimer = 0.0;

	// Unique id per attack press (used to avoid repeated damage per frame)
	int attackId = 0;

	// Score + combo multiplier
	int score = 0;
	int comboCount = 0;
	double comboTimer = 0.0;
	const double COMBO_WINDOW = 1.0;

	// Player HP
	int playerHP = 100;
	const int playerHPMax = 100;

	// Spikes cooldown
	double spikeCooldown = 0.0;

	// ------------------------------------------------------------------------
	// Objects: ALWAYS 2 boxes + 1 spikes
	// ------------------------------------------------------------------------
	const int MAX_OBJ = 3;
	GameObject objs[MAX_OBJ];
	int objCount = 0;

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

	double objMinFeetY = FLOOR_Y + g.footTopOffset;
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

	// ------------------------------------------------------------------------
	// Camera (X follow only)
	// ------------------------------------------------------------------------
	double cameraX = 0.0;
	double cameraY = 0.0; // unused (assignment allowed X-only)

	// ------------------------------------------------------------------------
	// Walk animation state (stand -> walk1 -> stand -> walk2 -> stand ...)
	// ------------------------------------------------------------------------
	double animTimer = 0.0;
	int animStep = 0;
	const double ANIM_STEP_TIME = 0.12;

	// ------------------------------------------------------------------------
	// Combo input buffer + developer mode
	// ------------------------------------------------------------------------
	InputBuffer inputBuf; inputBuf.count = 0;
	bool devMode = false;

	ComboAction currentComboAction = CA_NONE;
	double comboActionTimer = 0.0;

	// Dash state (direction is set by combo detection)
	double dashVX = 0.0, dashVY = 0.0;

	// Combo evaluation function (pattern-table style; easy to extend)
	auto TryStartComboAction = [&](double now) -> void {
		if (comboActionTimer > 0.0) return; // already doing one

		// XXX => triple jump
		{
			InputCmd p[] = { CMD_X, CMD_X, CMD_X };
			if (BufMatch(&inputBuf, p, 3, now, g.inputWindow)) {
				currentComboAction = CA_TRIPLE_JUMP;
				comboActionTimer = 0.30;
				return;
			}
		}

		// YYY => fury
		{
			InputCmd p[] = { CMD_Y, CMD_Y, CMD_Y };
			if (BufMatch(&inputBuf, p, 3, now, g.inputWindow)) {
				currentComboAction = CA_FURY;
				comboActionTimer = 0.55;
				return;
			}
		}

		// X Y X => uppercut
		{
			InputCmd p[] = { CMD_X, CMD_Y, CMD_X };
			if (BufMatch(&inputBuf, p, 3, now, g.inputWindow)) {
				currentComboAction = CA_UPPERCUT;
				comboActionTimer = 0.40;
				return;
			}
		}

		// Dash => double tap direction
		{
			InputCmd pR[] = { CMD_R, CMD_R };
			InputCmd pL[] = { CMD_L, CMD_L };
			InputCmd pU[] = { CMD_U, CMD_U };
			InputCmd pD[] = { CMD_D, CMD_D };

			if (BufMatch(&inputBuf, pR, 2, now, g.doubleTapWindow)) {
				currentComboAction = CA_DASH;
				comboActionTimer = g.dashDuration;
				dashVX = +1.0; dashVY = 0.0;
				return;
			}
			if (BufMatch(&inputBuf, pL, 2, now, g.doubleTapWindow)) {
				currentComboAction = CA_DASH;
				comboActionTimer = g.dashDuration;
				dashVX = -1.0; dashVY = 0.0;
				return;
			}
			if (BufMatch(&inputBuf, pU, 2, now, g.doubleTapWindow)) {
				currentComboAction = CA_DASH;
				comboActionTimer = g.dashDuration;
				dashVX = 0.0; dashVY = -1.0;
				return;
			}
			if (BufMatch(&inputBuf, pD, 2, now, g.doubleTapWindow)) {
				currentComboAction = CA_DASH;
				comboActionTimer = g.dashDuration;
				dashVX = 0.0; dashVY = +1.0;
				return;
			}
		}
	};

	// ------------------------------------------------------------------------
	// Time init
	// ------------------------------------------------------------------------
	t1 = SDL_GetTicks();
	stageTime = 0.0;
	fpsTimer = 0.0;
	fps = 0.0;
	frames = 0;
	quit = 0;

	// ------------------------------------------------------------------------
	// Main loop
	// ------------------------------------------------------------------------
	while (!quit) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		stageTime += delta;

		// --- combo-action timer update ---
		if (comboActionTimer > 0.0) {
			comboActionTimer -= delta;
			if (comboActionTimer <= 0.0) {
				comboActionTimer = 0.0;
				currentComboAction = CA_NONE;
				dashVX = dashVY = 0.0;
			}
		}

		// --- score combo timer decay ---
		if (comboTimer > 0.0) {
			comboTimer -= delta;
			if (comboTimer <= 0.0) {
				comboTimer = 0.0;
				comboCount = 0;
			}
		}

		// --- spikes cooldown ---
		if (spikeCooldown > 0.0) {
			spikeCooldown -= delta;
			if (spikeCooldown < 0.0) spikeCooldown = 0.0;
		}

		// --- hurt timer ---
		if (inHurt) {
			hurtTimer -= delta;
			if (hurtTimer <= 0.0) {
				inHurt = false;
				hurtTimer = 0.0;
			}
		}

		// --- jump physics (visual Z axis) ---
		if (inJump) {
			vz -= g.gravity * delta;
			z  += vz * delta;
			if (z <= 0.0) {
				z = 0.0;
				vz = 0.0;
				inJump = false;
			}
		}

		// --- attack timer ---
		if (action != ACT_NONE) {
			actionTimer -= delta;
			if (actionTimer <= 0.0) {
				action = ACT_NONE;
				actionTimer = 0.0;
			}
		}

		// --------------------------------------------------------------------
		// Movement input (continuous) + smooth control using accel/friction.
		// --------------------------------------------------------------------
		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		double inX = 0.0, inY = 0.0;
		if (keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A]) inX -= 1.0;
		if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) inX += 1.0;
		if (keys[SDL_SCANCODE_UP]    || keys[SDL_SCANCODE_W]) inY -= 1.0;
		if (keys[SDL_SCANCODE_DOWN]  || keys[SDL_SCANCODE_S]) inY += 1.0;

		double inLen = sqrt(inX*inX + inY*inY);
		if (inLen > 0.0) { inX /= inLen; inY /= inLen; }
		bool moving = (inLen > 0.0);

		// Speed multipliers based on state
		double speedScale = 1.0;
		if (action == ACT_LIGHT) speedScale = g.lightMoveScale;
		if (action == ACT_HEAVY) speedScale = g.heavyMoveScale;
		if (inHurt) speedScale = g.hurtMoveScale;

		// Dash overrides normal movement
		if (currentComboAction == CA_DASH && comboActionTimer > 0.0) {
			playerX += dashVX * g.dashSpeed * delta;
			playerY += dashVY * g.dashSpeed * delta;
			velX = 0.0; velY = 0.0; // avoid "snap back" after dash
		} else {
			// Desired velocities based on input
			double targetVX = inX * g.moveMaxSpeed * speedScale;
			double targetVY = inY * g.moveMaxSpeed * speedScale;

			if (moving) {
				velX = Approach(velX, targetVX, g.moveAccel * delta);
				velY = Approach(velY, targetVY, g.moveAccel * delta);
			} else {
				velX = Approach(velX, 0.0, g.moveFriction * delta);
				velY = Approach(velY, 0.0, g.moveFriction * delta);
			}

			playerX += velX * delta;
			playerY += velY * delta;
		}

		// Walking animation (only when not attacking/jumping/hurt)
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

		// Clamp player X to stage boundaries
		if (playerX < 0) playerX = 0;
		if (playerX > STAGE_W) playerX = STAGE_W;

		// Clamp player Y to the floor lane ("can move within floor, not into sky")
		double footTop = FLOOR_Y + g.footTopOffset;
		double footBottom = FLOOR_Y + FLOOR_H;
		if (playerY < footTop) playerY = footTop;
		if (playerY > footBottom) playerY = footBottom;

		// Camera follow on X axis with deadzone
		double playerScreenX = playerX - cameraX;
		if (playerScreenX < g.deadLeft)  cameraX = playerX - g.deadLeft;
		if (playerScreenX > g.deadRight) cameraX = playerX - g.deadRight;
		if (cameraX < 0) cameraX = 0;
		if (cameraX > STAGE_W - SCREEN_WIDTH) cameraX = STAGE_W - SCREEN_WIDTH;

		// --------------------------------------------------------------------
		// Sprite selection priority: hurt > attack > jump > walk > stand
		// --------------------------------------------------------------------
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

		// --------------------------------------------------------------------
		// Hitboxes
		// --------------------------------------------------------------------

		// Attack hitbox (always right of player feet)
		RectD attackBox = {0,0,0,0};
		bool attackActive = (action != ACT_NONE);

		if (attackActive) {
			double range  = (action == ACT_LIGHT) ? g.lightRange  : g.heavyRange;
			double height = (action == ACT_LIGHT) ? g.lightHeight : g.heavyHeight;

			attackBox.w = range;
			attackBox.h = height;
			attackBox.x = playerX + g.attackFrontOffset;
			attackBox.y = playerY - height;
		}

		// Player feet hitbox (used for spikes)
		double pBoxX = playerX - g.footBoxW / 2.0;
		double pBoxY = playerY - g.footBoxH;
		double pBoxW = g.footBoxW;
		double pBoxH = g.footBoxH;

		// --------------------------------------------------------------------
		// Interactions
		//  - Boxes: damaged by attack hitbox (once per press via attackId)
		//  - Spikes: damage player when feet overlaps (with cooldown)
		// --------------------------------------------------------------------
		for (int i = 0; i < objCount; i++) {
			if (!objs[i].alive) continue;

			double oLeft = objs[i].x - objs[i].hitbox_w / 2.0;
			double oTop  = objs[i].y - objs[i].hitbox_h;
			double oW    = objs[i].hitbox_w;
			double oH    = objs[i].hitbox_h;

			// Boxes
			if (objs[i].type == 0) {
				if (attackActive &&
				    RectOverlap(attackBox.x, attackBox.y, attackBox.w, attackBox.h,
				                oLeft, oTop, oW, oH)) {

					// Damage only once per attack press per object
					if (objs[i].lastHitAttackId != attackId) {
						objs[i].lastHitAttackId = attackId;

						int dmg = (action == ACT_LIGHT) ? 1 : 2;
						// Combo-action bonuses
						if (currentComboAction == CA_FURY) dmg += 1;
						if (currentComboAction == CA_UPPERCUT) dmg += 2;

						objs[i].boxHP -= dmg;

						// Scoring with multiplier
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

			// Spikes
			if (objs[i].type == 1) {
				if (RectOverlap(pBoxX, pBoxY, pBoxW, pBoxH, oLeft, oTop, oW, oH)) {
					if (spikeCooldown <= 0.0) {
						playerHP -= 10;
						if (playerHP < 0) playerHP = 0;

						// Getting hit resets score combo
						comboCount = 0;
						comboTimer = 0.0;

						// Show hurt sprite briefly
						inHurt = true;
						hurtTimer = HURT_DURATION;

						spikeCooldown = g.spikeCooldownTime;
					}
				}
			}
		}

		// --------------------------------------------------------------------
		// DRAW
		// --------------------------------------------------------------------
		int sky      = SDL_MapRGB(screen->format, 25, 25, 55);
		int floorCol = SDL_MapRGB(screen->format, 85, 95, 85);
		int floorEdge= SDL_MapRGB(screen->format, 25, 25, 30);

		SDL_FillRect(screen, NULL, sky);

		// Floor (screen coords). CameraY is unused by design.
		DrawRectangle(screen, 0, FLOOR_Y, SCREEN_WIDTH, FLOOR_H, floorEdge, floorCol);

		// Draw objects as rectangles (can be replaced with sprites later)
		int boxOut = SDL_MapRGB(screen->format, 200, 200, 200);
		int boxIn  = SDL_MapRGB(screen->format, 90, 90, 90);
		int spikeOut = SDL_MapRGB(screen->format, 255, 80, 80);
		int spikeIn  = SDL_MapRGB(screen->format, 140, 30, 30);

		for (int i = 0; i < objCount; i++) {
			if (!objs[i].alive) continue;

			int sx = (int)(objs[i].x - cameraX);
			int syFeet = (int)(objs[i].y); // cameraY not used
			int w = (int)objs[i].hitbox_w;
			int h = (int)objs[i].hitbox_h;

			int left = sx - w/2;
			int top  = syFeet - h;

			if (objs[i].type == 0) DrawRectangle(screen, left, top, w, h, boxOut, boxIn);
			else DrawRectangle(screen, left, top, w, h, spikeOut, spikeIn);
		}

		// Draw player: sprite center Y = feetY - (spriteH/2) - z
		int px = (int)(playerX - cameraX);
		int pyCenter = (int)(playerY - z) - currentSprite->h / 2;
		DrawSurface(screen, currentSprite, px, pyCenter);

		// FPS calc
		fpsTimer += delta;
		if (fpsTimer > 0.5) {
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

		// Developer mode: show buffer + current action + tuning snapshot
		if (devMode) {
			int y0 = 64;
			sprintf(text, "DEV: action=%s (%.2fs)", ActionName(currentComboAction), comboActionTimer);
			DrawString(screen, 10, y0, text, charset);

			sprintf(text, "TUNE: vmax=%.0f accel=%.0f fric=%.0f",
			        g.moveMaxSpeed, g.moveAccel, g.moveFriction);
			DrawString(screen, 10, y0 + 12, text, charset);

			char bufLine[512];
			strcpy(bufLine, "BUF: ");
			for (int i = 0; i < inputBuf.count; i++) {
				char part[64];
				double age = stageTime - inputBuf.e[i].t;
				sprintf(part, "%s(%.2f) ", CmdName(inputBuf.e[i].cmd), age);
				strcat(bufLine, part);
			}
			DrawString(screen, 10, y0 + 24, bufLine, charset);
		}

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// --------------------------------------------------------------------
		// Event handling (discrete presses go into the input buffer)
		// --------------------------------------------------------------------
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN: {
					SDL_Keycode k = event.key.keysym.sym;

					if (k == SDLK_ESCAPE) quit = 1;

					// Toggle dev mode
					if (k == SDLK_F1 && event.key.repeat == 0) {
						devMode = !devMode;
					}

					// New game reset
					else if (k == SDLK_n) {
						NewGame(stageTime, cameraX, cameraY, playerX, playerY);

						velX = velY = 0.0;

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

					// Buffer inputs (only once per press)
					else if (event.key.repeat == 0) {
						// Movement taps for dash detection
						if (k == SDLK_a || k == SDLK_LEFT)  { BufPush(&inputBuf, CMD_L, stageTime); TryStartComboAction(stageTime); }
						if (k == SDLK_d || k == SDLK_RIGHT) { BufPush(&inputBuf, CMD_R, stageTime); TryStartComboAction(stageTime); }
						if (k == SDLK_w || k == SDLK_UP)    { BufPush(&inputBuf, CMD_U, stageTime); TryStartComboAction(stageTime); }
						if (k == SDLK_s || k == SDLK_DOWN)  { BufPush(&inputBuf, CMD_D, stageTime); TryStartComboAction(stageTime); }

						// Jump (X) + buffer
						if (k == SDLK_x) {
							BufPush(&inputBuf, CMD_X, stageTime);
							TryStartComboAction(stageTime);

							double usedJumpVel = g.jumpVel;
							if (currentComboAction == CA_TRIPLE_JUMP) usedJumpVel *= g.tripleJumpScale;

							if (!inJump) {
								inJump = true;
								vz = usedJumpVel;
							}
						}

						// Light attack (Z) + buffer
						else if (k == SDLK_z) {
							BufPush(&inputBuf, CMD_Z, stageTime);
							TryStartComboAction(stageTime);

							if (action == ACT_NONE) {
								action = ACT_LIGHT;
								actionTimer = g.lightAttackDuration;
								attackId++; // unique id for this press
							}
						}

						// Heavy attack (Y) + buffer
						else if (k == SDLK_y) {
							BufPush(&inputBuf, CMD_Y, stageTime);
							TryStartComboAction(stageTime);

							if (action == ACT_NONE) {
								action = ACT_HEAVY;
								actionTimer = g.heavyAttackDuration;
								attackId++; // unique id for this press
							}
						}
					}
				} break;

				case SDL_QUIT:
					quit = 1;
					break;
			}
		}

		frames++;
	}

	// ------------------------------------------------------------------------
	// Cleanup
	// ------------------------------------------------------------------------
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
