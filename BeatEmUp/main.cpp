#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480

// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
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

// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
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
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
}

void NewGame(double &stageTime, double &cameraX, double &cameraY, double &playerX, double &playerY){
	stageTime = 0.0;
	cameraX = 0.0;
	cameraY = 0.0;
	playerX = 200.0;
	playerY = 350.0;
}

// placeholder rect type for attack hitbox (enemy will be added later)
struct RectD { double x, y, w, h; };

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int newGame = 1;
	double gameStartTime = 0.0;
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps, distance, etiSpeed;
	SDL_Event event;
	SDL_Surface *screen, *charset;

	// player sprites
	SDL_Surface *sprStand, *sprWalk1, *sprWalk2;
	SDL_Surface *sprJump, *sprAttackHeavy, *sprAttackLight;

	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	printf("printf output goes here\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2017");

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_ShowCursor(SDL_DISABLE);

	// loading the image cs8x8.bmp
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
	sprJump = SDL_LoadBMP("./player_jump.bmp");
	sprAttackHeavy = SDL_LoadBMP("./player_attack.bmp");     // heavy attack sprite
	sprAttackLight = SDL_LoadBMP("./player_attack2.bmp");    // light attack sprite

	if(!sprStand || !sprWalk1 || !sprWalk2 || !sprJump || !sprAttackHeavy || !sprAttackLight) {
		printf("SDL_LoadBMP(player sprites) error: %s\n", SDL_GetError());
		if (sprStand) SDL_FreeSurface(sprStand);
		if (sprWalk1) SDL_FreeSurface(sprWalk1);
		if (sprWalk2) SDL_FreeSurface(sprWalk2);
		if (sprJump) SDL_FreeSurface(sprJump);
		if (sprAttackHeavy) SDL_FreeSurface(sprAttackHeavy);
		if (sprAttackLight) SDL_FreeSurface(sprAttackLight);
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

	char text[128];
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	double stageTime = 0.0;

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	distance = 0;
	etiSpeed = 1;

	const double STAGE_W = 2000.0;
	const double STAGE_H = 900.0;

	const int FLOOR_Y = 260;
	const int FLOOR_H = 200;

	// player variables
	double playerX = 200.0;
	double playerY = FLOOR_Y + FLOOR_H / 2.0;
	double playerSpeed = 260.0;

	// for action (easy to tune)
	double gravity = 2200.0;
	double jumpVel = 950.0; // bigger = higher jump
	double lightAttackDuration = 0.18; // seconds (fast)
	double heavyAttackDuration = 0.35; // seconds (slow)
	double attackMoveScale = 0.35; // how much you can move during an attack

	// jump state
	bool inJump = false;
	double z = 0.0;   // vertical offset (purely visual)
	double vz = 0.0;  // vertical velocity

	// attack state (light/heavy)
	enum Action { ACT_NONE, ACT_LIGHT, ACT_HEAVY };
	Action action = ACT_NONE;
	double actionTimer = 0.0;

	// camera variables
	double cameraX = 0.0;
	double cameraY = 0.0;
	const int DEAD_LEFT = 220;
	const int DEAD_RIGHT = 420;

	NewGame(stageTime, cameraX, cameraY, playerX, playerY);

	// walking animation
	double animTimer = 0.0;
	int animStep = 0;
	const double ANIM_STEP_TIME = 0.12;

	while(!quit) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		stageTime += delta;

		// --- jump physics update ---
		if (inJump) {
			vz -= gravity * delta;
			z  += vz * delta;
			if (z <= 0.0) {
				z = 0.0;
				vz = 0.0;
				inJump = false;
			}
		}

		// --- attack timer update ---
		if (action != ACT_NONE) {
			actionTimer -= delta;
			if (actionTimer <= 0.0) {
				action = ACT_NONE;
				actionTimer = 0.0;
			}
		}

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		double vx = 0.0, vy = 0.0;
		if(keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) vx -= 1.0;
		if(keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) vx += 1.0;
		if(keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) vy -= 1.0;
		if(keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) vy += 1.0;

		// diagonal movement normalization
		double len = sqrt(vx*vx + vy*vy);
		if(len > 0.0) { vx /= len; vy /= len; }

		bool moving = (len > 0.0);

		// movement reduced during attack
		double speedScale = 1.0;
		if (action != ACT_NONE) speedScale = attackMoveScale;

		playerX += vx * playerSpeed * speedScale * delta;
		playerY += vy * playerSpeed * speedScale * delta;

		// walking animation update (only if not attacking/jumping)
		if (moving && action == ACT_NONE && !inJump) {
			animTimer += delta;
			while (animTimer >= ANIM_STEP_TIME) {
				animTimer -= ANIM_STEP_TIME;
				animStep = (animStep + 1) % 4;
			}
		} else {
			animTimer = 0.0;
			animStep = 0;
		}

		// clamp player to stage bounds (X)
		if (playerX < 0) playerX = 0;
		if (playerX > STAGE_W) playerX = STAGE_W;

		// floor lane clamp (Y)
		double footTop = FLOOR_Y + 30;
		double footBottom = FLOOR_Y + FLOOR_H;
		if (playerY < footTop) playerY = footTop;
		if (playerY > footBottom) playerY = footBottom;

		// camera follow (X only)
		double playerScreenX = playerX - cameraX;
		if (playerScreenX < DEAD_LEFT) cameraX = playerX - DEAD_LEFT;
		if (playerScreenX > DEAD_RIGHT) cameraX = playerX - DEAD_RIGHT;
		if (cameraX < 0) cameraX = 0;
		if (cameraX > STAGE_W - SCREEN_WIDTH) cameraX = STAGE_W - SCREEN_WIDTH;

		// choose current sprite (priority: attack > jump > walk > stand)
		SDL_Surface* currentSprite = sprStand;

		if (action == ACT_LIGHT) currentSprite = sprAttackLight;
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

		// attack hitbox placeholder (for later enemy)
		RectD attackBox = {0,0,0,0};
		bool attackActive = (action != ACT_NONE);
		if (attackActive) {
			double range = 90.0;
			double height = 80.0;

			attackBox.w = range;
			attackBox.h = height;
			attackBox.x = playerX + 60.0;   // later: use facing direction
			attackBox.y = playerY - height; // above feet
		}

		// background
		int sky = SDL_MapRGB(screen->format, 25, 25, 55);
		int bg2 = SDL_MapRGB(screen->format, 150, 125, 65);
		int floorCol = SDL_MapRGB(screen->format, 85, 95, 85);
		int floorEdge = SDL_MapRGB(screen->format, 25, 25, 30);

		SDL_FillRect(screen, NULL, sky);

		// background stripes
		for (int i = 0; i < SCREEN_WIDTH; i += 80) {
			int x = i - (int)(cameraX * 0.2) % 80;
			DrawRectangle(screen, x, 60, 40, 80, bg2, bg2);
		}

		// floor
		int floorScreenY = FLOOR_Y - (int)cameraY;
		DrawRectangle(screen, 0, floorScreenY, SCREEN_WIDTH, FLOOR_H, floorEdge, floorCol);

		// player
		int px = (int)(playerX - cameraX);
		int py = (int)(playerY - cameraY - z) - currentSprite->h / 2;
		DrawSurface(screen, currentSprite, px, py);

		// fps
		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		}

		// info text
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		sprintf(text, "Beat 'em up Game | time = %.1lf s | fps =  %.0lf", stageTime, fps);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		sprintf(text, "Esc quit | N new | X jump | Z light atk | Y heavy atk");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		// handling of events
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if(event.key.keysym.sym == SDLK_n) {
						NewGame(stageTime, cameraX, cameraY, playerX, playerY);
						// reset jump + attacks
						action = ACT_NONE; actionTimer = 0.0;
						inJump = false; z = 0.0; vz = 0.0;
					}
					else if(event.key.repeat == 0) {
						if(event.key.keysym.sym == SDLK_x) {
							if(!inJump) {
								inJump = true;
								vz = jumpVel;
							}
						}
						else if(event.key.keysym.sym == SDLK_z) {
							if(action == ACT_NONE) {
								action = ACT_LIGHT;
								actionTimer = lightAttackDuration;
							}
						}
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

	// freeing all surfaces
	SDL_FreeSurface(sprStand);
	SDL_FreeSurface(sprWalk1);
	SDL_FreeSurface(sprWalk2);
	SDL_FreeSurface(sprJump);
	SDL_FreeSurface(sprAttackHeavy);
	SDL_FreeSurface(sprAttackLight);
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}
