#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>

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
	int type; // 0 = box, 1 = spikes (later: enemies)
	double x, y; // FEET contact point in world coords
	double hitbox_w, hitbox_h; // hitbox size
	int boxHP;     // only for box
	bool alive;
};

// AABB overlap
bool RectOverlap(double ax, double ay, double aw, double ah,
                 double bx, double by, double bw, double bh) {
	return (ax < bx + bw) && (ax + aw > bx) && (ay < by + bh) && (ay + ah > by);
}

int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc;
	double delta, stageTime, fpsTimer, fps;

	SDL_Event event;
	SDL_Surface *screen, *charset;

	// player sprites
	SDL_Surface *sprStand, *sprWalk1, *sprWalk2;
	SDL_Surface *sprJump, *sprAttackHeavy, *sprAttackLight;
	SDL_Surface *sprHurt; // OW sprite

	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	printf("printf output goes here\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

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
	sprAttackHeavy = SDL_LoadBMP("./player_attack.bmp");   // heavy
	sprAttackLight = SDL_LoadBMP("./player_attack2.bmp");  // light
	sprHurt = SDL_LoadBMP("./player_hurt.bmp");            // OW sprite

	if(!sprStand || !sprWalk1 || !sprWalk2 || !sprJump || !sprAttackHeavy || !sprAttackLight || !sprHurt) {
		printf("SDL_LoadBMP(player sprites) error: %s\n", SDL_GetError());
		if (sprStand) SDL_FreeSurface(sprStand);
		if (sprWalk1) SDL_FreeSurface(sprWalk1);
		if (sprWalk2) SDL_FreeSurface(sprWalk2);
		if (sprJump) SDL_FreeSurface(sprJump);
		if (sprAttackHeavy) SDL_FreeSurface(sprAttackHeavy);
		if (sprAttackLight) SDL_FreeSurface(sprAttackLight);
		if (sprHurt) SDL_FreeSurface(sprHurt);
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

	char text[256];
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	// ---- stage ----
	const double STAGE_W = 2000.0;
	const double STAGE_H = 900.0;

	const int FLOOR_Y = 260;
	const int FLOOR_H = 200;

	// ---- player ----
	double playerX = 200.0;
	double playerY = FLOOR_Y + FLOOR_H / 2.0;
	double playerSpeed = 260.0;

	// jump tuning
	double gravity = 2200.0;
	double jumpVel = 950.0;

	// action tuning (IMPORTANT: declare BEFORE using)
	double lightAttackDuration = 0.18;
	double heavyAttackDuration = 0.35;

	double lightRange = 90.0;
	double lightHeight = 70.0;

	double heavyRange = 140.0;
	double heavyHeight = 90.0;

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

	// score + combo
	int score = 0;
	int comboCount = 0;
	double comboTimer = 0.0;
	const double COMBO_WINDOW = 1.0; // seconds

	// player HP
	int playerHP = 100;
	int playerHPMax = 100;

	// spikes damage cooldown (prevents HP melting instantly)
	double spikeCooldown = 0.0;
	const double SPIKE_COOLDOWN_TIME = 0.6;

	// ---- objects (boxes + spikes now, enemies later) ----
	const int MAX_OBJ = 20;
	GameObject objs[MAX_OBJ];
	int objCount = 0;

	// spawn helpers (C++ lambda)
	auto AddBox = [&](double x, double feetY) {
		if (objCount >= MAX_OBJ) return;
		objs[objCount++] = {0, x, feetY, 70.0, 60.0, 3, true}; // type 0 = box, HP=3
	};
	auto AddSpikes = [&](double x, double feetY) {
		if (objCount >= MAX_OBJ) return;
		objs[objCount++] = {1, x, feetY, 70.0, 25.0, 0, true}; // type 1 = spikes
	};

	// spawn objects
	objCount = 0;
	AddBox(500,  FLOOR_Y + FLOOR_H);
	AddSpikes(650, FLOOR_Y + FLOOR_H);
	AddBox(800,  FLOOR_Y + FLOOR_H);
	AddSpikes(1100, FLOOR_Y + FLOOR_H);

	// ---- camera ----
	double cameraX = 0.0;
	double cameraY = 0.0; // currently unused in your game
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

	while(!quit) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		stageTime += delta;

		// combo timer decay
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

		// optional: freeze movement briefly when hurt
		if (inHurt) speedScale = 0.0;

		playerX += vx * playerSpeed * speedScale * delta;
		playerY += vy * playerSpeed * speedScale * delta;

		// walking animation (only when not attacking/jumping/hurt)
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

		// clamp to stage bounds (X)
		if (playerX < 0) playerX = 0;
		if (playerX > STAGE_W) playerX = STAGE_W;

		// floor lane clamp (Y) - feet stay on floor lane
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

		// choose sprite priority: hurt > attack > jump > walk > stand
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

		// ---- ATTACK HITBOX (always attacks to the RIGHT) ----
		RectD attackBox = {0,0,0,0};
		bool attackActive = (action != ACT_NONE);

		if (attackActive) {
			double range  = (action == ACT_LIGHT) ? lightRange  : heavyRange;
			double height = (action == ACT_LIGHT) ? lightHeight : heavyHeight;

			attackBox.w = range;
			attackBox.h = height;

			// starts a bit in front of player feet point
			double frontOffset = 40.0;

			attackBox.x = playerX + frontOffset;
			attackBox.y = playerY - height;
		}

		// ---- PLAYER HITBOX (for spikes) ----
		// Make a small foot area around the player's FEET contact point
		double playerFootW = 40.0;
		double playerFootH = 20.0;
		double pBoxX = playerX - playerFootW / 2.0;
		double pBoxY = playerY - playerFootH; // just above feet
		double pBoxW = playerFootW;
		double pBoxH = playerFootH;

		// ---- OBJECT INTERACTIONS ----
		for (int i = 0; i < objCount; i++) {
			if (!objs[i].alive) continue;

			double oLeft = objs[i].x - objs[i].hitbox_w / 2.0;
			double oTop  = objs[i].y - objs[i].hitbox_h;
			double oW    = objs[i].hitbox_w;
			double oH    = objs[i].hitbox_h;

			// box: can be hit by attacks
			if (objs[i].type == 0) {
				if (attackActive) {
					if (RectOverlap(attackBox.x, attackBox.y, attackBox.w, attackBox.h,
					                oLeft, oTop, oW, oH)) {
						// deal damage once per attack start? (simple version: per frame)
						// We'll reduce HP only when actionTimer is near its start:
						bool nearStart = false;
						if (action == ACT_LIGHT && actionTimer > lightAttackDuration - 0.08) nearStart = true;
						if (action == ACT_HEAVY && actionTimer > heavyAttackDuration - 0.10) nearStart = true;

						if (nearStart) {
							objs[i].boxHP -= (action == ACT_LIGHT) ? 1 : 2;

							// scoring + combo
							int base = (action == ACT_LIGHT) ? 10 : 15;
							int multiplier = 1 + comboCount;  // 1,2,3,...
							score += base * multiplier;

							comboCount += 1;
							comboTimer = COMBO_WINDOW;

							if (objs[i].boxHP <= 0) {
								objs[i].alive = false;
								score += 50 * (1 + comboCount); // bonus for breaking box
							}
						}
					}
				}
			}

			// spikes: damage player when stepping on them
			if (objs[i].type == 1) {
				if (RectOverlap(pBoxX, pBoxY, pBoxW, pBoxH, oLeft, oTop, oW, oH)) {
					if (spikeCooldown <= 0.0) {
						playerHP -= 10;
						if (playerHP < 0) playerHP = 0;

						// getting hit resets combo
						comboCount = 0;
						comboTimer = 0.0;

						// show OW sprite
						inHurt = true;
						hurtTimer = HURT_DURATION;

						spikeCooldown = SPIKE_COOLDOWN_TIME;
					}
				}
			}
		}

		// ---- DRAW ----
		int sky      = SDL_MapRGB(screen->format, 25, 25, 55);
		int stripe   = SDL_MapRGB(screen->format, 150, 125, 65);
		int floorCol = SDL_MapRGB(screen->format, 85, 95, 85);
		int floorEdge= SDL_MapRGB(screen->format, 25, 25, 30);

		SDL_FillRect(screen, NULL, sky);

		// background stripes
		for (int i = 0; i < SCREEN_WIDTH; i += 80) {
			int x = i - ((int)(cameraX * 0.2) % 80);
			DrawRectangle(screen, x, 60, 40, 80, stripe, stripe);
		}

		// floor
		int floorScreenY = FLOOR_Y - (int)cameraY;
		DrawRectangle(screen, 0, floorScreenY, SCREEN_WIDTH, FLOOR_H, floorEdge, floorCol);

		// draw objects (simple rectangles, can be replaced with sprites later)
		int boxOut = SDL_MapRGB(screen->format, 200, 200, 200);
		int boxIn  = SDL_MapRGB(screen->format, 90, 90, 90);
		int spikeOut = SDL_MapRGB(screen->format, 255, 80, 80);
		int spikeIn  = SDL_MapRGB(screen->format, 140, 30, 30);

		for (int i = 0; i < objCount; i++) {
			if (!objs[i].alive) continue;

			int sx = (int)(objs[i].x - cameraX);
			int syFeet = (int)(objs[i].y - cameraY);
			int w = (int)objs[i].hitbox_w;
			int h = (int)objs[i].hitbox_h;

			int left = sx - w/2;
			int top  = syFeet - h;

			if (objs[i].type == 0) {
				DrawRectangle(screen, left, top, w, h, boxOut, boxIn);
			} else {
				DrawRectangle(screen, left, top, w, h, spikeOut, spikeIn);
			}
		}

		// optionally draw attack hitbox for debugging
		// if (attackActive) {
		// 	int hbOut = SDL_MapRGB(screen->format, 0, 255, 0);
		// 	int hbIn  = SDL_MapRGB(screen->format, 0, 100, 0);
		// 	int ax = (int)(attackBox.x - cameraX);
		// 	int ay = (int)(attackBox.y - cameraY);
		// 	DrawRectangle(screen, ax, ay, (int)attackBox.w, (int)attackBox.h, hbOut, hbIn);
		// }

		// player
		int px = (int)(playerX - cameraX);
		int py = (int)(playerY - cameraY - z) - currentSprite->h / 2;
		DrawSurface(screen, currentSprite, px, py);

		// FPS calc
		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		}

		// ---- UI: top panel ----
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 52, czerwony, niebieski);

		// time + fps
		sprintf(text, "time = %.1lf s | fps = %.0lf | score=%d | combo=%d", stageTime, fps, score, comboCount);
		DrawString(screen, 10, 10, text, charset);

		// controls
		sprintf(text, "Esc quit | N new | X jump | Z light atk | Y heavy atk");
		DrawString(screen, 10, 26, text, charset);

		// ---- UI: HP bar (top-left, inside panel) ----
		int barX = 10;
		int barY = 42;
		int barW = 180;
		int barH = 10;

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

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// ---- events ----
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;

					// new game
					else if(event.key.keysym.sym == SDLK_n) {
						NewGame(stageTime, cameraX, cameraY, playerX, playerY);

						// reset player
						playerHP = playerHPMax;
						score = 0;
						comboCount = 0;
						comboTimer = 0.0;

						// reset actions
						action = ACT_NONE; actionTimer = 0.0;
						inJump = false; z = 0.0; vz = 0.0;

						// reset hurt
						inHurt = false; hurtTimer = 0.0;
						spikeCooldown = 0.0;

						// respawn objects
						objCount = 0;
						AddBox(500,  FLOOR_Y + FLOOR_H);
						AddSpikes(650, FLOOR_Y + FLOOR_H);
						AddBox(800,  FLOOR_Y + FLOOR_H);
						AddSpikes(1100, FLOOR_Y + FLOOR_H);
					}

					// actions (only once per key press)
					else if(event.key.repeat == 0) {

						// jump
						if(event.key.keysym.sym == SDLK_x) {
							if(!inJump) {
								inJump = true;
								vz = jumpVel;
							}
						}

						// light attack (fast)
						else if(event.key.keysym.sym == SDLK_z) {
							if(action == ACT_NONE) {
								action = ACT_LIGHT;
								actionTimer = lightAttackDuration;
							}
						}

						// heavy attack (slow)
						else if(event.key.keysym.sym == SDLK_y) {
							if(action == ACT_NONE) {
								action = ACT_HEAVY;
								actionTimer = heavyAttackDuration;
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
