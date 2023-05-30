#ifndef MAIN_H
#define MAIN_H

#include <shim4/shim4.h>

#include "game.h"

struct Bullet
{
	bool friendly;
	util::Point<float> pos;
	int power;
	float angle;
};

struct Bullet_Collision
{
	util::Point<float> pos;
	Uint32 start_time;
	int weapon_power;
	bool friendly;
};

extern int joy_action;
extern int joy_back;

extern int key_action;
extern int key_back;

extern audio::MML *button_mml;

extern SDL_Colour YELLOW;

extern util::Translation *game_t;
extern util::Translation *english_game_t;

#define SQUARE_W (int)(shim::screen_size.h/10.0f)

extern std::string language;

extern gfx::Image *button_9patch;

extern gfx::TTF *title_font;
extern gfx::TTF *small_font;

extern Game *game;

struct Title_Square
{
	util::Point<float> pos;
	float speed;
};

extern gfx::Image *square;

extern bool go_fullscreen;

extern bool paused;
extern Uint32 paused_time;
extern Uint32 pause_start;

extern std::vector<Bullet> bullets;

extern audio::MML *shot_normal;
extern audio::MML *shot_big;

extern bool gameover;
extern Uint32 gameover_start;

extern int score;

struct Score
{
	std::string name;
	int score;
};

extern std::vector<Score> scores;

extern bool rumble_enabled;

void create_game();

int score_sort(Score &a, Score &b);

void draw_crystal(util::Point<float> centre, float radius, float start_angle, int health, float alpha = 1.0f);

#endif // MAIN_H
