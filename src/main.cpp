#include <shim4/shim4.h>

#include "general.h"
#include "gui.h"
#include "main.h"

const float SPEED = 0.025f;
const float BOUNCER_SPEED = 0.005f;
const float BULLET_SPEED = 0.05f;
const int MAX_WEAPON_POWER = 2;
const Uint32 COLLISION_TIME = 250;
const int BARRIER_SCR_H = 512;

int orig_argc;
char **orig_argv;

std::string extra_args;
std::string extra_args_orig;

bool quit = false;

int joy_action;
int joy_back;

int key_action;
int key_back;

audio::MML *button_mml;

SDL_Colour YELLOW;

util::Translation *game_t;
util::Translation *english_game_t;

std::string language;

gfx::TTF *title_font;
gfx::TTF *small_font;

Game *game;

gfx::Image *square;

bool go_fullscreen;

bool paused;
Uint32 paused_time;
Uint32 pause_start;

std::vector<Bullet> bullets;
std::vector<Bullet_Collision> bullet_collisions;

audio::MML *shot_normal;
audio::MML *shot_big;

bool gameover;
Uint32 gameover_start;

int score;
int wave;
int objects_in_wave;
int objects_spawned;

int num_enemies;
int start_enemy;
int end_enemy;
bool done_all_enemies;

int spawn_delay;

std::vector<Score> scores;

bool rumble_enabled;

static util::Size<int> last_screen_size;
static bool save_the_settings;
static Uint32 save_settings_at;
static std::vector< util::Point<float> > offsets;
static float stick_1_x;
static float stick_1_y;
static float stick_2_x;
static float stick_2_y;
static bool was_paused;
static float small_radius;
static float big_radius;
static audio::Sample *explosion_small;
static audio::Sample *powerup;

// health here is 0-6
void draw_crystal(util::Point<float> centre, float radius, float start_angle, int health, float alpha)
{
	int sections = 6;

	if (health < 0) {
		health = 0;
	}

	SDL_Colour c1;
	SDL_Colour c2 = shim::white;
	SDL_Colour c;

	c1.r = 0;
	c1.g = 255;
	c1.b = 255;
	c1.a = 255;

	gfx::draw_primitives_start();

	if (sections == -1) {
		sections = M_PI * radius; // sections equal to half of circumference
	}

	if (sections < 4) {
		sections = 4;
	}

	for (int n = 0; n < sections; n++) {
		if (n < sections-health) {
			c.r = 216;
			c.g = 0;
			c.b = 0 ;
			c.a = 255;
		}
		else {
			float p = n / ((float)sections-1.0f);
			int red = c2.r - c1.r;
			int green = c2.g - c1.g;
			int blue = c2.b - c1.b;
			c.r = c1.r + red * p;
			c.g = c1.g + green * p;
			c.b = c1.b + blue * p;
			c.a = 255;
		}
		int n2 = (n+1) % sections;
		float a1 = start_angle + (float)n/sections * (float)M_PI * 2.0f;
		float a2 = start_angle + (float)n2/sections * (float)M_PI * 2.0f;
		noo::util::Point<float> a, b;
		a = centre + noo::util::Point<float>(cos(a1) * radius, sin(a1) * radius);
		b = centre + noo::util::Point<float>(cos(a2) * radius, sin(a2) * radius);
		SDL_Colour ca = c;
		ca.r *= alpha;
		ca.g *= alpha;
		ca.b *= alpha;
		ca.a *= alpha;
		SDL_Colour colours[3] = { ca, ca, ca };
		noo::gfx::Vertex_Cache::instance()->cache(colours, centre, a, b);
	}

	gfx::draw_primitives_end();
}

static util::Point<float> calc_offset()
{
	if (game == nullptr) {
		return util::Point<float>(0.0f, 0.0f);
	}

	util::Point<float> p(game->player_pos.x*shim::real_screen_size.h, game->player_pos.y*shim::real_screen_size.h);
	util::Point<float> offset(p.x-shim::real_screen_size.w/2.0f, p.y-shim::real_screen_size.h/2.0f);

	if (offset.x < 0.0f) {
		offset.x = 0.0f;
	}

	if (offset.y < 0.0f) {
		offset.y = 0.0f;
	}

	offset.x = MIN(shim::real_screen_size.h*game->arena_size.w-shim::real_screen_size.w, offset.x);
	offset.y = MIN(shim::real_screen_size.h*(game->arena_size.h-1), offset.y);

	return offset;
}
	
static void create_stars()
{
	for (int i = 0; i < 20*game->arena_size.w*game->arena_size.h; i++) {
		Game::Star s;
		s.pos.x = util::rand(0, 1000) / 1000.0f * game->arena_size.w;
		if (s.pos.x < 0.15f || s.pos.x > game->arena_size.w-0.15f) {
			continue;
		}
		s.pos.y = util::rand(0, 1000) / 1000.0f * game->arena_size.h;
		if (s.pos.y < 0.15f || s.pos.y > game->arena_size.h-0.15f) {
			continue;
		}
		s.colour.r = 144;
		s.colour.g = 144;
		s.colour.b = 144;
		s.colour.a = 255;
		s.twinkle = get_ticks() + util::rand(0, 2500);
		s.twinkle_end = s.twinkle + 15;
		game->stars.push_back(s);
	}
}

static void joystick_disconnected()
{
	if (shim::guis.size() > 0) {
		return;
	}
	button_mml->play(false);
	pause_timers();
	Pause_GUI *p = new Pause_GUI(false);
	shim::guis.push_back(p);
}

static void steam_overlay_callback()
{
	if (shim::guis.size() > 0) {
		return;
	}
	button_mml->play(false);
	pause_timers();
	Pause_GUI *p = new Pause_GUI(false);
	shim::guis.push_back(p);
}

static void load_fonts()
{
	int sheet_size = 512;

	while (sheet_size < shim::screen_size.w && sheet_size < shim::screen_size.h) {
		sheet_size *= 2;
	}

	if (sheet_size > 512 && (sheet_size > shim::screen_size.w || sheet_size > shim::screen_size.h)) {
		sheet_size /= 2;
	}

	title_font = new gfx::TTF("font.ttf", SQUARE_W*2, sheet_size);

	small_font = new gfx::TTF("font.ttf", SQUARE_W/2.0f, sheet_size);
}

static void destroy_fonts()
{
	delete title_font;
	title_font = nullptr;
	delete small_font;
	small_font = nullptr;
}

static gfx::Image *generate_barrier(Uint8 **buf_ret)
{
	int sq_w = 512 / 10;
	int r = sq_w * 2.0f;
	int r2 = sq_w * 1.5f;
	int w = r * 2;
	int w2 = r2 * 2;

	Uint8 *buf = new Uint8[w*w*4];

	memset(buf, 0, w*w*4);

	for (int y = 0; y < w; y++) {
		for (int x = 0; x < w; x++) {
			Uint8 *p = buf + y * w * 4 + x * 4;

			util::Point<float> a(r, r);
			util::Point<float> b(x, y);

			float dist = sqrt(pow(b.x - a.x, 2.0f) + pow(b.y - a.y, 2.0f));

			if (dist <= r && dist >= r2) {
				p[0] = 255;
				p[1] = 0;
				p[2] = 255;
				p[3] = 255;
			}
		}
	}

	gfx::Image *img = new gfx::Image(buf, util::Size<int>(w, w), false);

	*buf_ret = buf;

	return img;
}

static gfx::Image *generate_small_pixels(Uint8 **buf_ret)
{
	int sq_w = 512 / 10;
	float half = sq_w / 2.0f;
	int w = sqrt(half*half + half*half) * 2.0f;

	Uint8 *buf = new Uint8[w*w*4];

	memset(buf, 0, w*w*4);

	for (int y = 0; y < w; y++) {
		for (int x = 0; x < w; x++) {
			Uint8 *p = buf + y * w * 4 + x * 4;

			util::Point<float> a(w/2, w/2);
			util::Point<float> b(x, y);

			util::Point<float> dist(b.x - a.x, b.y - a.y);

			if (std::abs(dist.x)+std::abs(dist.y) <= w/2) {
				p[0] = 255;
				p[1] = 216;
				p[2] = 0;
				p[3] = 255;
			}
		}
	}

	gfx::Image *img = new gfx::Image(buf, util::Size<int>(w, w), false);

	*buf_ret = buf;

	return img;
}

static gfx::Image *generate_big_pixels(Uint8 **buf_ret)
{
	int sq_w = 512 / 10;
	float half = sq_w / 2.0f * 1.5f;
	int w = sqrt(half*half + half*half) * 2.0f;

	Uint8 *buf = new Uint8[w*w*4];

	memset(buf, 0, w*w*4);

	for (int y = 0; y < w; y++) {
		for (int x = 0; x < w; x++) {
			Uint8 *p = buf + y * w * 4 + x * 4;

			util::Point<float> a(w/2, w/2);
			util::Point<float> b(x, y);

			util::Point<float> dist(b.x - a.x, b.y - a.y);

			if (std::abs(dist.x)+std::abs(dist.y) <= w/2) {
				p[0] = 255;
				p[1] = 216;
				p[2] = 0;
				p[3] = 255;
			}
		}
	}

	gfx::Image *img = new gfx::Image(buf, util::Size<int>(w, w), false);

	*buf_ret = buf;

	return img;
}

void damage(Game::Crystal *crystal, int cx, int cy)
{
	float radius = game->weapon_power >= 2 ? big_radius : small_radius;
	radius *= BARRIER_SCR_H;
	int check_w = radius;
	float cxx = cx + 0.5f;
	float cyy = cy + 0.5f;

	int start_x = MAX(0, cx-check_w);
	int start_y = MAX(0, cy-check_w);
	int end_x = MIN(crystal->barrier->size.w, cx+check_w+1);
	int end_y = MIN(crystal->barrier->size.h, cy+check_w+1);

	for (int y = start_y; y < end_y; y++) {
		for (int x = start_x; x < end_x; x++) {
			unsigned char *p = crystal->buf + (crystal->barrier->size.h-y-1) * crystal->barrier->size.w * 4 + x * 4;

			float xx = cxx-(x+0.5f);
			float yy = cyy-(y+0.5f);

			float d = sqrtf(xx*xx+yy*yy);

			//float d = sqrt(pow((cx+0.5f)-(x+0.5f), 2.0f)+pow((cy+0.5f)-(y+0.5f), 2.0f));
			if (d <= radius && p[0] == 255) {
				//float a = 192.0f/255.0f;
				p[0] = 24;//32 * a;
				p[1] = 24;//32 * a;
				p[2] = 24;//32 * a;
				p[3] = 192;//255 * a;
			}
		}
	}
}

void enemy_damage(Game::Enemy *enemy, int cx, int cy)
{
	float radius = game->weapon_power >= 2 ? big_radius : small_radius;
	radius *= BARRIER_SCR_H;
	int check_w = radius;
	float cxx = cx + 0.5f;
	float cyy = cy + 0.5f;

	int start_x = MAX(0, cx-check_w);
	int start_y = MAX(0, cy-check_w);
	int end_x = MIN(enemy->barrier->size.w, cx+check_w+1);
	int end_y = MIN(enemy->barrier->size.h, cy+check_w+1);

	for (int y = start_y; y < end_y; y++) {
		for (int x = start_x; x < end_x; x++) {
			unsigned char *p = enemy->buf + (enemy->barrier->size.h-y-1) * enemy->barrier->size.w * 4 + x * 4;

			float xx = cxx-(x+0.5f);
			float yy = cyy-(y+0.5f);

			float d = sqrtf(xx*xx+yy*yy);

			//float d = sqrt(pow((cx+0.5f)-(x+0.5f), 2.0f)+pow((cy+0.5f)-(y+0.5f), 2.0f));
			if (d <= radius && p[0] == 255) {
				//float a = 192.0f/255.0f;
				p[0] = 24;//32 * a;
				p[1] = 24;//32 * a;
				p[2] = 24;//32 * a;
				p[3] = 192;//255 * a;
			}
		}
	}
}

static bool barrier_collision_check(Game::Crystal *crystal, int cx, int cy)
{
	Uint8 *bytes = crystal->buf;
	bool collided = false;

	for (int yy = -2; yy < 3; yy++) {
		for (int xx = -2; xx < 3; xx++) {
			int x = cx + xx;
			int y = cy + yy;
			if (x < 0 || y < 0 || x >= crystal->barrier->size.w || y >= crystal->barrier->size.h) {
				continue;
			}
			int o = (crystal->barrier->size.h-y-1)*crystal->barrier->size.w*4+x*4;
			if (bytes[o] == 255 && bytes[o+1] == 0 && bytes[o+2] == 255) {
				collided = true;
				break;
			}
		}
		if (collided) {
			break;
		}
	}

	if (collided == false) {
		return false;
	}

	damage(crystal, cx, cy);

	int w = crystal->barrier->size.w;
	delete crystal->barrier;
	crystal->barrier = new gfx::Image(bytes, util::Size<int>(w, w), false);

	return true;
}

static bool enemy_barrier_collision_check(Game::Enemy *enemy, int cx, int cy)
{
	Uint8 *bytes = enemy->buf;
	bool collided = false;

	for (int yy = -2; yy < 3; yy++) {
		for (int xx = -2; xx < 3; xx++) {
			int x = cx + xx;
			int y = cy + yy;
			if (x < 0 || y < 0 || x >= enemy->barrier->size.w || y >= enemy->barrier->size.h) {
				continue;
			}
			int o = (enemy->barrier->size.h-y-1)*enemy->barrier->size.w*4+x*4;
			if (bytes[o] == 255 && bytes[o+1] == 216 && bytes[o+2] == 0) {
				collided = true;
				break;
			}
		}
		if (collided) {
			break;
		}
	}

	if (collided == false) {
		return false;
	}

	enemy_damage(enemy, cx, cy);

	int w = enemy->barrier->size.w;
	delete enemy->barrier;
	enemy->barrier = new gfx::Image(bytes, util::Size<int>(w, w), false);

	return true;
}

static void generate_barriers()
{
	if (game == nullptr) {
		return;
	}
	for (size_t i = 0; i < game->crystals.size(); i++) {
		Game::Crystal &c = game->crystals[i];
		Uint8 *buf;
		c.barrier = generate_barrier(&buf);
		c.buf = buf;
	}
}

static void destroy_barriers()
{
	if (game == nullptr) {
		return;
	}
	for (size_t i = 0; i < game->crystals.size(); i++) {
		delete game->crystals[i].barrier;
		game->crystals[i].barrier = nullptr;
		//delete[] game->crystals[i].buf;
		//game->crystals[i].buf = nullptr;
	}
	for (size_t i = 0; i < game->enemies.size(); i++) {
		if (game->enemies[i].type == Game::ENEMY_SMALL_PIXELS) {
			delete game->enemies[i].barrier;
			game->enemies[i].barrier = nullptr;
		}
		else if (game->enemies[i].type == Game::ENEMY_BIG_PIXELS) {
			delete game->enemies[i].barrier;
			game->enemies[i].barrier = nullptr;
		}
	}
}

static void lost_device()
{
	destroy_fonts();
	destroy_barriers();
}

static void found_device()
{
	load_fonts();
	if (game) {
		game->stars.clear();
		create_stars();
	}
	offsets.clear();
	if (game) {
		for (size_t i = 0; i < game->crystals.size(); i++) {
			Game::Crystal *crystal = &game->crystals[i];
			int sq_w = 512 / 10;
			int r = sq_w * 2.0f;
			int w = r * 2;
			crystal->barrier = new gfx::Image(crystal->buf, util::Size<int>(w, w), false);
		}
		for (size_t i = 0; i < game->enemies.size(); i++) {
			int sq_w = 512 / 10;
			float half = sq_w / 2.0f;
			int w = sqrt(half*half + half*half) * 2.0f;
			game->enemies[i].barrier = new gfx::Image(game->enemies[i].buf, util::Size<int>(w, w), false);
		}
	}
}

static void draw_all()
{
	if (last_screen_size != shim::real_screen_size) {
		shim::font_size = shim::real_screen_size.h / 10.0f;
		gfx::resize_window(shim::real_screen_size.w, shim::real_screen_size.h);
		//shim::screen_size = shim::real_screen_size;
		//gfx::set_screen_size(shim::real_screen_size);
		//gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
		//gfx::update_projection();
		lost_device();
		found_device();
		last_screen_size = shim::real_screen_size;
	}

#ifdef _WIN32
	if (shim::opengl == false && gfx::is_d3d_lost() == true) {
		gfx::clear(shim::black);
		gfx::flip(); // this is where lost devices are handled...
		return;
	}
#endif

	gfx::clear_buffers();

	if (game) {
		util::Point<float> offset = calc_offset();

		util::Point<float> star_vec;

		if (offsets.size() == 0) {
			star_vec = util::Point<float>(0.0f, 0.0f);
		}
		else {
			star_vec = offset - offsets[0];
		}

		for (size_t i = 0; i < game->stars.size(); i++) {
			Game::Star &s = game->stars[i];

			util::Point<float> pos(s.pos.x*shim::real_screen_size.h, s.pos.y*shim::real_screen_size.h);

			float thick = shim::real_screen_size.h / 540.0f; // on 1080p, will be 2 pixels thick

			gfx::draw_line(s.colour, pos-offset, pos-offset+star_vec+util::Point<float>(thick, thick), thick);
		}

		// draw walls

		std::vector< util::Point<float> > start_light;
		std::vector< util::Point<float> > end_light;
		std::vector< util::Point<float> > start_dark;
		std::vector< util::Point<float> > end_dark;

		// left
		start_dark.push_back({0.0f, 0.0f});
		for (int i = 0; i < game->arena_size.h; i++) {
			end_dark.push_back({0.1f, (0.5f+i)-0.15f});
			start_dark.push_back({0.0f, (0.5f+i)+0.15f});
		}
		end_dark.push_back({0.1f, (float)game->arena_size.h});

		// right
		start_dark.push_back({game->arena_size.w-0.1f, 0.0f});
		for (int i = 0; i < game->arena_size.h; i++) {
			end_dark.push_back({(float)game->arena_size.w, (0.5f+i)-0.15f});
			start_dark.push_back({game->arena_size.w-0.1f, (0.5f+i)+0.15f});
		}
		end_dark.push_back({(float)game->arena_size.w, (float)game->arena_size.h});

		// top
		start_dark.push_back({0.0f, 0.0f});
		for (int i = 0; i < game->arena_size.w; i++) {
			end_dark.push_back({(0.5f+i)-0.15f, 0.1f});
			start_dark.push_back({(0.5f+i)+0.15f, 0.0f});
		}
		end_dark.push_back({(float)game->arena_size.w, 0.1f});

		// bottom
		start_dark.push_back({0.0f, game->arena_size.h-0.1f});
		for (int i = 0; i < game->arena_size.w; i++) {
			end_dark.push_back({(0.5f+i)-0.15f, (float)game->arena_size.h});
			start_dark.push_back({(0.5f+i)+0.15f, game->arena_size.h-0.1f});
		}
		end_dark.push_back({(float)game->arena_size.w, (float)game->arena_size.h});

		const float EXTEND = 1.0f / 64.0f;
		const float SQ = 0.1f;

		// left
		for (int i = 0; i < game->arena_size.h; i++) {
			start_light.push_back({0.0f-EXTEND, (0.5f+i)-0.15f-SQ-EXTEND});
			end_light.push_back({0.1f+EXTEND, (0.5f+i)-0.15f+EXTEND});
			start_light.push_back({0.0f-EXTEND, (0.5f+i)+0.15f-EXTEND});
			end_light.push_back({0.1f+EXTEND, (0.5f+i)+0.15f+SQ+EXTEND});
		}

		// right

		for (int i = 0; i < game->arena_size.h; i++) {
			start_light.push_back({game->arena_size.w-0.1f-EXTEND, (0.5f+i)-0.15f-SQ-EXTEND});
			end_light.push_back({game->arena_size.w+EXTEND, (0.5f+i)-0.15f+EXTEND});
			start_light.push_back({game->arena_size.w-0.1f-EXTEND, (0.5f+i)+0.15f-EXTEND});
			end_light.push_back({game->arena_size.w+EXTEND, (0.5f+i)+0.15f+SQ+EXTEND});
		}

		// top
		for (int i = 0; i < game->arena_size.w; i++) {
			start_light.push_back({(0.5f+i)-0.15f-SQ-EXTEND, 0.0f-EXTEND});
			end_light.push_back({(0.5f+i)-0.15f+EXTEND, 0.1f+EXTEND});
			start_light.push_back({(0.5f+i)+0.15f-EXTEND, 0.0f-EXTEND});
			end_light.push_back({(0.5f+i)+0.15f+SQ+EXTEND, 0.1f+EXTEND});
		}

		// bottom
		for (int i = 0; i < game->arena_size.w; i++) {
			start_light.push_back({(0.5f+i)-0.15f-SQ-EXTEND, game->arena_size.h-0.1f-EXTEND});
			end_light.push_back({(0.5f+i)-0.15f+EXTEND, game->arena_size.h+EXTEND});
			start_light.push_back({(0.5f+i)+0.15f-EXTEND, game->arena_size.h-0.1f-EXTEND});
			end_light.push_back({(0.5f+i)+0.15f+SQ+EXTEND, game->arena_size.h+EXTEND});
		}

		gfx::draw_primitives_start();

		for (size_t i = 0; i < start_dark.size(); i++) {
			util::Size<float> sz(end_dark[i].x-start_dark[i].x, end_dark[i].y-start_dark[i].y);
			sz *= shim::real_screen_size.h;
			SDL_Colour c;
			c.r = 64;
			c.g = 64;
			c.b = 64;
			c.a = 255;
			gfx::draw_filled_rectangle(c, start_dark[i]*shim::real_screen_size.h-offset, sz);
		}

		for (size_t i = 0; i < start_light.size(); i++) {
			util::Size<float> sz(end_light[i].x-start_light[i].x, end_light[i].y-start_light[i].y);
			sz *= shim::real_screen_size.h;
			SDL_Colour c;
			c.r = 128;
			c.g = 128;
			c.b = 128;
			c.a = 255;
			gfx::draw_filled_rectangle(c, start_light[i]*shim::real_screen_size.h-offset, sz);
		}

		gfx::draw_primitives_end();

		// crystals

		for (size_t i = 0; i < game->crystals.size(); i++) {
			Game::Crystal &c = game->crystals[i];
			util::Point<float> pos = c.pos;
			pos *= shim::real_screen_size.h;
			pos -= offset;
			int health;
			if (c.health <= 0) {
				health = 0;
			}
			else {
				health = c.health * 6; // round down
				health++;
			}
			draw_crystal(pos, SQUARE_W, 0.0f, health);
			SDL_Colour tint;
			tint.r = 192;
			tint.g = 192;
			tint.b = 192;
			tint.a = 192;
			//c.barrier->draw_tinted(tint, {pos.x-SQUARE_W*2.0f, pos.y-SQUARE_W*2.0f});
			c.barrier->stretch_region_tinted(tint, util::Point<float>(0.0f, 0.0f), c.barrier->size, {pos.x-SQUARE_W*2.0f, pos.y-SQUARE_W*2.0f}, util::Size<int>(SQUARE_W*4.0f, SQUARE_W*4.0f));
		}

		gfx::draw_primitives_start();

		// bullet collisions

		for (std::vector<Bullet_Collision>::iterator it = bullet_collisions.begin(); it != bullet_collisions.end();) {
			Bullet_Collision &bc = *it;
			util::Point<float> pos = bc.pos;
			pos *= shim::real_screen_size.h;
			pos -= offset;
			Uint32 now = get_ticks();
			Uint32 diff = now - bc.start_time;
			float p = diff / (float)COLLISION_TIME;
			if (p > 1.0f) {
				it = bullet_collisions.erase(it);
				continue;
			}
			SDL_Colour c;
			if (bc.friendly) {
				c.r = 0;
				c.g = 216;
				c.b = 0;
				c.a = 255;
			}
			else {
				c.r = 216;
				c.g = 0;
				c.b = 0;
				c.a = 255;
			}

			float radius;
			if (bc.weapon_power >= 2) {
				radius = big_radius * 1.5f;
			}
			else {
				radius = small_radius * 1.5f;
			}
			gfx::draw_circle(c, pos, p*radius*shim::real_screen_size.h, shim::real_screen_size.h / 540.0f);
			it++;
		}

		// bullets

		for (size_t i = 0; i < bullets.size(); i++) {
			Bullet &b = bullets[i];

			float sz;

			if (b.power >= 2) {
				sz = 0.02f;
			}
			else {
				sz = 0.01f;
			}

			// 2.0 is about 120 degrees for an equilateral triangle

			util::Point<float> v1(b.pos.x+cos(b.angle)*sz, b.pos.y+sin(b.angle)*sz);
			util::Point<float> v2(b.pos.x+cos(b.angle+2.0f)*sz, b.pos.y+sin(b.angle+2.0f)*sz);
			util::Point<float> v3(b.pos.x+cos(b.angle-2.0f)*sz, b.pos.y+sin(b.angle-2.0f)*sz);

			v1 = (v1*shim::real_screen_size.h) - offset;
			v2 = (v2*shim::real_screen_size.h) - offset;
			v3 = (v3*shim::real_screen_size.h) - offset;

			SDL_Colour c;

			if (b.friendly) {
				c.r = 0;
				c.g = 216;
				c.b = 0;
				c.a = 255;
			}
			else {
				c.r = 216;
				c.g = 0;
				c.b = 0;
				c.a = 255;
			}

			gfx::draw_filled_triangle(c, v1, v2, v3);
		}

		gfx::draw_primitives_end();

		// draw player

		util::Point<float> ppos(game->player_pos.x*shim::real_screen_size.h-offset.x, game->player_pos.y*shim::real_screen_size.h-offset.y);

		float wp;

		if (gameover) {
			Uint32 now = get_ticks();
			Uint32 diff = now - gameover_start;
			if (diff > 14650) {
				diff = 14650;
			}
			wp = 1.0f - (diff / 14650.0f);
		}
		else {
			wp = 1.0f;
		}

		SDL_Colour white_trans;
		white_trans.r = 192 * wp;
		white_trans.g = 192 * wp;
		white_trans.b = 192 * wp;
		white_trans.a = 192 * wp;
	
		for (size_t i = 0; i < game->powerups.size(); i++) {
			util::Point<float> pos = game->powerups[i].pos;
			pos *= shim::real_screen_size.h;
			pos -= offset;
			float angle = get_ticks() / 2500.0f * M_PI * 2;
			SDL_Colour c = game->powerups[i].colour;
			Uint32 now = get_ticks();
			if (now - game->powerups[i].start_time >= 27500) {
				float p = (now - game->powerups[i].start_time - 27500) / 2500.0f;
				p = 1.0f - p;
				if (p < 0.0f) {
					p = 0.0f;
				}
				if (p > 1.0f) {
					p = 1.0f;
				}
				c.r *= p;
				c.g *= p;
				c.b *= p;
				c.a *= p;
			}
			gfx::draw_filled_circle(c, pos, SQUARE_W/2.0f, 5, angle);
		}

		for (size_t i = 0; i < game->enemies.size(); i++) {
			if (game->enemies[i].type == Game::ENEMY_SMALL_PIXELS || game->enemies[i].type == Game::ENEMY_BIG_PIXELS) {
				Game::Enemy *enemy = &game->enemies[i];
				util::Point<float> pos = game->enemies[i].pos;
				pos *= shim::real_screen_size.h;
				pos -= offset;
				if (game->enemies[i].type == Game::ENEMY_SMALL_PIXELS) {
					enemy->barrier->stretch_region(util::Point<float>(0.0f, 0.0f), enemy->barrier->size, {pos.x-SQUARE_W/2.0f, pos.y-SQUARE_W/2.0f}, util::Size<int>(SQUARE_W, SQUARE_W));
				}
				else {
					enemy->barrier->stretch_region(util::Point<float>(0.0f, 0.0f), enemy->barrier->size, {pos.x-SQUARE_W/2.0f*1.5f, pos.y-SQUARE_W/2.0f*1.5f}, util::Size<int>(SQUARE_W*1.5f, SQUARE_W*1.5f));
				}
			}
			else {
				for (size_t j = 0; j < game->enemies[i].squares.size(); j++) {
					if (game->enemies[i].squares[j].dead) {
						continue;
					}

					util::Point<float> pos = game->enemies[i].squares[j].pos;
					pos *= shim::real_screen_size.h;
					pos -= offset;
					float angle = get_ticks() / -2500.0f * M_PI * 2;
			
					SDL_Colour c;
					c.r = 255;
					c.g = 216;
					c.b = 0;
					c.a = 255;

					square->draw_tinted_rotated_scaled(c, util::Point<float>(square->size.w/2.0f, square->size.h/2.0f), pos, angle, SQUARE_W*0.75f/(float)square->size.w, 0);
				}
			}
		}

		gfx::draw_filled_circle(white_trans, ppos, SQUARE_W/2.0f);

		// ui

		for (size_t i = 0; i < game->crystals.size(); i++) {
			Game::Crystal &c = game->crystals[i];
			int health;
			if (c.health <= 0) {
				health = 0;
			}
			else {
				health = c.health * 6; // round down
				health++;
			}
			draw_crystal(util::Point<float>(SQUARE_W+i*5+SQUARE_W/4+i*SQUARE_W/2, SQUARE_W/2.0f), SQUARE_W/4.0f, 0.0f, health);
		}

		std::string score_s = util::string_printf("%08d", score);
		util::Point<float> score_pos(shim::real_screen_size.w-shim::font->get_text_width(score_s)-SQUARE_W, SQUARE_W/2.0f-shim::font->get_height()/2.0f);
		shim::font->draw(shim::white, score_s, score_pos);
		
	}
	else {
		//gfx::clear(YELLOW);
		gfx::clear(shim::black);
	}

	gfx::draw_guis();
	
	gfx::draw_notifications();

	float cry_angle = (get_ticks() % 3333) / 3333.0f * M_PI * 2;

	//draw_crystal(util::Point<float>(100.0f+SQUARE_W*2.0f, 150.0f+SQUARE_W*2.0f), SQUARE_W, cry_angle, 4);
	//barrier1->draw(util::Point<float>(100.0f, 150.0f));

	gfx::flip();
}

bool start()
{
	// This is basically 16:9 only, with a tiny bit of leeway
	gfx::set_min_aspect_ratio(1.75f);
	gfx::set_max_aspect_ratio(1.79f);

	if (util::bool_arg(false, shim::argc, shim::argv, "logging")) {
		shim::logging = true;
	}

	if (util::bool_arg(false, shim::argc, shim::argv, "dump-images")) {
		gfx::Image::keep_data = true;
		gfx::Image::save_palettes = util::bool_arg(false, shim::argc, shim::argv, "save-palettes");
		gfx::Image::save_rle = true;
		gfx::Image::save_rgba = false;
		gfx::Image::premultiply_alpha = false;
	}

	/* get_desktop_resolution uses shim::adapter, but it's not normally set until shim::start is called, so set it here since
	 * it's used below.
	 */
	int adapter_i = util::check_args(shim::argc, shim::argv, "+adapter");
	if (adapter_i >= 0) {
		shim::adapter = atoi(shim::argv[adapter_i+1]);
		if (shim::adapter >= SDL_GetNumVideoDisplays()-1) {
			shim::adapter = 0;
		}
	}

	gfx::set_minimum_window_size(util::Size<int>(1280, 720));
	util::Size<int> desktop_resolution = gfx::get_desktop_resolution();
	gfx::set_maximum_window_size(desktop_resolution);

#if !defined IOS && !defined ANDROID
	const int min_supp_w = 1280;
	const int min_supp_h = 720;

	if (desktop_resolution.w < min_supp_w || desktop_resolution.h < min_supp_h) {
		gui::popup("Unsupported System", "The minimum resolution supported by this game is 1280x720, which this system does not meet. Exiting.", gui::OK);
		exit(1);
	}
#endif

	int win_w = 1280;
	int win_h = 720;

	if (desktop_resolution.w >= 1920*2 && desktop_resolution.h >= 1080*2) {
		win_w = 2560;
		win_h = 1440;
	}
	else if (desktop_resolution.w >= 2560 && desktop_resolution.h >= 1440) {
		win_w = 1920;
		win_h = 1080;
	}

	shim::font_size = win_h / 10.0f;

	//if (shim::start_all(0, 0, false, desktop_resolution.w, desktop_resolution.h) == false) {
	if (shim::start_all(-1, -1, false, win_w, win_h) == false) {
		gui::fatalerror("shim::start failed", "Initialization failed.", gui::OK, false);
		return false;
	}

#ifdef _WIN32
	gfx::enable_press_and_hold(false);
#endif

	if (shim::font == 0) {
		gui::fatalerror("Fatal Error", "Font not found! Aborting.", gui::OK, false);
		return false;
	}

	if (util::bool_arg(false, shim::argc, shim::argv, "dump-images")) {
		std::vector<std::string> filenames = shim::cpa->get_all_filenames();
		for (size_t i = 0; i < filenames.size(); i++) {
			std::string filename =  filenames[i];
			if (filename.find(".tga") != std::string::npos) {
				gfx::Image *image = new gfx::Image(filename, true);
				std::string path = "out/" + filename;
				std::string dir;
				size_t i;
				while ((i = path.find("/")) != std::string::npos) {
					dir += path.substr(0, i);
					util::mkdir(dir.c_str());
					path = path.substr(i+1);
					dir += "/";
				}
				image->save("out/" + filename);
				delete image;
			}
		}
		exit(0);
	}

	TGUI::set_focus_sloppiness(0);

	//shim::user_render = draw_all;

	gfx::register_lost_device_callbacks(lost_device, found_device);
	shim::joystick_disconnect_callback = joystick_disconnected;

#ifdef STEAMWORKS
	shim::steam_overlay_activated_callback = steam_overlay_callback;
#endif

	return true;
}

void handle_event(TGUI_Event *event)
{
	if (event->type == TGUI_UNKNOWN) {
		return;
	}
	else if (event->type == TGUI_QUIT) {
		quit = true;
	}
	else if (gameover == false && game && paused == false && was_paused == false && ((event->type == TGUI_JOY_DOWN && event->joystick.button == joy_back) || (event->type == TGUI_KEY_DOWN && event->keyboard.code == key_back))) {
		button_mml->play(false);
		pause_timers();
		Pause_GUI *p = new Pause_GUI(false);
		shim::guis.push_back(p);
	}
}

void game_logic()
{
	if (game && !paused) {
		if (gameover == false && input::is_joystick_connected()) {
			SDL_JoystickID id = input::get_controller_id(0);
			SDL_GameController *gc = input::get_sdl_gamecontroller(id);

			Sint16 xi = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTX);
			Sint16 yi = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTY);

			float x;
			float y;

			if (xi < 0) {
				x = xi / 32768.0f;
			}
			else {
				x = xi / 32767.0f;
			}

			if (yi < 0) {
				y = yi / 32768.0f;
			}
			else {
				y = yi / 32767.0f;
			}

			stick_1_x = fabs(x) < 0.1f ? 0.0f : x;
			stick_1_y = fabs(y) < 0.1f ? 0.0f : y;

			xi = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_RIGHTX);
			yi = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_RIGHTY);

			if (xi < 0) {
				x = xi / 32768.0f;
			}
			else {
				x = xi / 32767.0f;
			}

			if (yi < 0) {
				y = yi / 32768.0f;
			}
			else {
				y = yi / 32767.0f;
			}

			stick_2_x = fabs(x) < 0.1f ? 0.0f : x;
			stick_2_y = fabs(y) < 0.1f ? 0.0f : y;
		}

		game->player_pos += util::Point<float>(stick_1_x*SPEED, stick_1_y*SPEED);

		for (std::vector<Bullet>::iterator it = bullets.begin(); it != bullets.end();) {
			Bullet &b = *it;

			float speed;

			bool hit = false;
				
			bool delete_it = false;

			int steps = 100;
			
			// huge hack rather than doing lots of math I inch along very fine increments
			for (int i = 0; i < steps; i++) {
				b.pos.x += cos(b.angle) * BULLET_SPEED/steps;
				b.pos.y += sin(b.angle) * BULLET_SPEED/steps;

				// Check collisions with crystal barriers

				for (size_t i = 0; i < game->crystals.size(); i++) {
					Game::Crystal &c = game->crystals[i];
					util::Point<float> pos(b.pos.x-(c.pos.x-0.2f), b.pos.y-(c.pos.y-0.2f));
					float radius = game->weapon_power >= 2 ? big_radius : small_radius;
					if (pos.x >= -radius && pos.y >= -radius && pos.x <= 0.4f+radius && pos.y <= 0.4f+radius) {
						int x = pos.x * BARRIER_SCR_H;
						int y = pos.y * BARRIER_SCR_H;
						if (barrier_collision_check(&c, x, y)) {
							Bullet_Collision bc;
							bc.pos = b.pos;
							bc.start_time = get_ticks();
							bc.weapon_power = b.power;
							bc.friendly = b.friendly;
							bullet_collisions.push_back(bc);
							explosion_small->play(false);
							delete_it = true;
							hit = true;
							break;
						}
					}

					// check collisions with crystals

					util::Point<float> d = c.pos - b.pos;
					float dist = d.length();
					if (dist <= 0.075f) {
						Bullet_Collision bc;
						bc.pos = b.pos;
						bc.start_time = get_ticks();
						bc.weapon_power = b.power;
						bc.friendly = b.friendly;
						bullet_collisions.push_back(bc);
						explosion_small->play(false);
						if (b.power >= 2) {
							c.health -= 2/60.0f;
						}
						else {
							c.health -= 1/60.0f;
						}
						delete_it = true;
						hit = true;
						break;
					}

					if (hit) {
						break;
					}
				}

				// check collisions with enemies
				if (b.friendly) {
					for (std::vector<Game::Enemy>::iterator it = game->enemies.begin(); it != game->enemies.end();) {
						Game::Enemy &e = *it;

						if (e.type == Game::ENEMY_SMALL_PIXELS || e.type == Game::ENEMY_BIG_PIXELS) {
							// Check collisions with crystal barriers

							float inc;

							if (e.type == Game::ENEMY_SMALL_PIXELS) {
								inc = sqrt(0.05f*0.05f + 0.05f*0.05f);
							}
							else {
								inc = sqrt(0.075f*0.075f + 0.075f*0.075f);
							}

							util::Point<float> pos(b.pos.x-(e.pos.x-inc), b.pos.y-(e.pos.y-inc));
							float radius = game->weapon_power >= 2 ? big_radius : small_radius;
							if (pos.x >= -radius && pos.y >= -radius && pos.x <= (inc*2)+radius && pos.y <= (inc*2)+radius) {
								int x = pos.x * BARRIER_SCR_H;
								int y = pos.y * BARRIER_SCR_H;
								if (enemy_barrier_collision_check(&e, x, y)) {
									Bullet_Collision bc;
									bc.pos = b.pos;
									bc.start_time = get_ticks();
									bc.weapon_power = b.power;
									bc.friendly = b.friendly;
									bullet_collisions.push_back(bc);
									explosion_small->play(false);
									delete_it = true;
									hit = true;
								}
							
								if (hit) {
									bool all_dead = true;
									for (size_t i = 0; i < e.barrier->size.w*e.barrier->size.h*4; i += 4) {
										if (e.buf[i] == 255) {
											all_dead = false;
											break;
										}
									}
									if (all_dead) {
										it = game->enemies.erase(it);
									}
									else {
										it++;
									}
								}
								else {
									it++;
								}
							}
							else {
								it++;
							}
						}
						else {
							for (std::vector<Game::Enemy_Square>::iterator it2 = e.squares.begin(); it2 != e.squares.end();) {
								Game::Enemy_Square &sq = *it2;

								if (sq.dead) {
									it2++;
								}
								else {
									float dist = (sq.pos-b.pos).length();

									if (dist < 0.05f) {
										Bullet_Collision bc;
										bc.pos = b.pos;
										bc.start_time = get_ticks();
										bc.weapon_power = e.weapon_power;
										bc.friendly = b.friendly;
										bullet_collisions.push_back(bc);
										explosion_small->play(false);
										delete_it = true;
										hit = true;
										sq.dead = true;
										if (e.type == Game::ENEMY_SMALL_PIXELS) {
											score += 1000;
										}
										else if (e.type == Game::ENEMY_BIG_PIXELS) {
											score += 2500;
										}
										else {
											score += 100;
										}
										break;
									}
									else {
										it2++;
									}
								}
							}

							if (hit) {
								bool all_dead = true;
								for (size_t i = 0; i < e.squares.size(); i++) {
									if (e.squares[i].dead == false) {
										all_dead = false;
										break;
									}
								}
								if (all_dead) {
									it = game->enemies.erase(it);
								}
								else {
									it++;
								}
								break;
							}
							else {
								it++;
							}
						}
					}

					if (hit) {
						break;
					}
				}

				if (hit) {
					break;
				}
			}

			if (delete_it || (b.pos.x < -0.1f || b.pos.y < -0.1f || b.pos.x > game->arena_size.w+0.1f || b.pos.y > game->arena_size.h+0.1f)) {
				it = bullets.erase(it);
			}
			else {
				it++;
			}
		}

		if (game->next_bullet <= get_ticks() && (fabs(stick_2_x) >= 0.1f || fabs(stick_2_y) >= 0.1f)) {
			if (rumble_enabled) {
				input::rumble(game->weapon_power >= 2.0f ? 100 : 50);
			}
			float angle = atan2(stick_2_y, stick_2_x);

			util::Point<float> pos(game->player_pos.x+cos(angle)*0.075f, game->player_pos.y+sin(angle)*0.075f);

			Bullet b;

			b.friendly = true;
			b.pos = pos;
			b.power = game->weapon_power;
			b.angle = angle;

			if (b.power >= 2) {
				shot_big->play(false);
			}
			else {
				shot_normal->play(false);
			}

			bullets.push_back(b);

			if (game->weapon_power == 0) {
				game->next_bullet = get_ticks() + 200;
			}
			else {
				game->next_bullet = get_ticks() + 100;
			}
		}

		// border is SQUARE_W which is 1/10 screen height. 0.15f is 1/10 plus half that for half player width

		if (game->player_pos.x < 0.15f) {
			game->player_pos.x = 0.15f;
		}
		if (game->player_pos.x > game->arena_size.w-0.15f) {
			game->player_pos.x = game->arena_size.w-0.15f;
		}

		if (game->player_pos.y < 0.15f) {
			game->player_pos.y = 0.15f;
		}
		if (game->player_pos.y > game->arena_size.h-0.15f) {
			game->player_pos.y = game->arena_size.h-0.15f;
		}

		offsets.push_back(calc_offset());
		while (offsets.size() > 3) {
			offsets.erase(offsets.begin());
		}

		for (size_t i = 0; i < game->stars.size(); i++) {
			Game::Star &s = game->stars[i];
			if (s.twinkle_end < get_ticks()) {
				s.colour.r = 144;
				s.colour.g = 144;
				s.colour.b = 144;
				s.twinkle = get_ticks() + util::rand(0, 2500);
				s.twinkle_end = s.twinkle + 15;
			}
			else if (s.twinkle < get_ticks()) {
				s.colour.r = 255;
				s.colour.g = 255;
				s.colour.b = 255;
			}
		}

		for (size_t i = 0; i < game->crystals.size(); i++) {
			Game::Crystal &c = game->crystals[i];
			c.pos += c.speed;
			if (c.pos.x < 0.3f) {
				c.pos.x = 0.3f;
				c.speed.x = -c.speed.x;
			}
			if (c.pos.y < 0.3f) {
				c.pos.y = 0.3f;
				c.speed.y = -c.speed.y;
			}
			if (c.pos.x > game->arena_size.w-0.3f) {
				c.pos.x = game->arena_size.w-0.3f;
				c.speed.x = -c.speed.x;
			}
			if (c.pos.y > game->arena_size.h-0.3f) {
				c.pos.y = game->arena_size.h-0.3f;
				c.speed.y = -c.speed.y;
			}
		}

		if (objects_spawned < objects_in_wave && get_ticks() > game->next_object) {
			objects_spawned++;

			game->next_object = get_ticks() + spawn_delay;

			if (util::rand(0, 9) == 0) {
				Game::Powerup o;

				o.start_portal = util::rand(0, (game->arena_size.w*2+game->arena_size.h*2)-1);

				bool crystals_low = false;

				for (size_t i = 0; i < game->crystals.size(); i++) {
					if (game->crystals[i].health <= 4.99f/6.0f) {
						crystals_low = true;
						break;
					}
				}

				if (crystals_low && util::rand(0, 49) == 0) {
					o.type = 2;
				}
				else if (game->enemies.size() > 0 && util::rand(0, 24) == 0) {
					o.type = 3;
				}
				else {
					o.type = util::rand(0, 1);
				}

				o.starting = true;

				float trans = 192.0f/255.0f;

				if (o.type == 0) {
					o.colour.r = 255 * trans;
					o.colour.g = 0 * trans;
					o.colour.b = 255 * trans;
					o.colour.a = 255 * trans;
				}
				else if (o.type == 1) {
					o.colour.r = 0 * trans;
					o.colour.g = 216 * trans;
					o.colour.b = 0 * trans;
					o.colour.a = 255 * trans;
				}
				else if (o.type == 2) {
					o.colour.r = 0 * trans;
					o.colour.g = 255 * trans;
					o.colour.b = 255 * trans;
					o.colour.a = 255 * trans;
				}
				else {
					o.colour.r = 255 * trans;
					o.colour.g = 0 * trans;
					o.colour.b = 0 * trans;
					o.colour.a = 255 * trans;
				}

				if (o.start_portal < game->arena_size.w) {
					o.pos = util::Point<float>(0.5f+o.start_portal, -0.25f);
					o.speed.x = 0.0f;
					o.speed.y = BOUNCER_SPEED/2.0f;
				}
				else if (o.start_portal < game->arena_size.w*2) {
					o.pos = util::Point<float>(0.5f+(o.start_portal-game->arena_size.w), game->arena_size.h+0.25f);
					o.speed.x = 0.0f;
					o.speed.y = -BOUNCER_SPEED/2.0f;
				}
				else if (o.start_portal < game->arena_size.w*2+game->arena_size.h) {
					o.pos = util::Point<float>(-0.25f, 0.5f+(o.start_portal-game->arena_size.w*2));
					o.speed.x = BOUNCER_SPEED/2.0f;
					o.speed.y = 0.0f;
				}
				else {
					o.pos = util::Point<float>(game->arena_size.w+0.25f, 0.5f+(o.start_portal-game->arena_size.w*2-game->arena_size.h));
					o.speed.x = -BOUNCER_SPEED/2.0f;
					o.speed.y = 0.0f;
				}

				o.start_time = get_ticks();

				game->powerups.push_back(o);
			}
			else {
				Game::Enemy o;

				o.start_portal = util::rand(0, (game->arena_size.w*2+game->arena_size.h*2)-1);

				int r = util::rand(start_enemy, end_enemy);

				if (r == 4) {
					if (util::rand(0, 9) == 0) {
						o.type = Game::ENEMY_SMALL_PIXELS;
					}
					else {
						while (r == 4 || r == 5) {
							r = util::rand(start_enemy, end_enemy);
						}
					}
				}

				if (r == 5) {
					if (util::rand(0, 9) == 0) {
						o.type = Game::ENEMY_BIG_PIXELS;
					}
					else {
						while (r == 4 || r == 5) {
							r = util::rand(start_enemy, end_enemy);
						}
					}
				}

				if (r == 1) {
					o.type = Game::ENEMY_SQUARE;
				}
				else if (r == 2) {
					o.type = Game::ENEMY_SPIN;
				}
				else if (r == 0) {
					o.type = Game::ENEMY_STRAIGHT;
					o.next_dir = 0;
				}
				else if (r == 3) {
					o.type = Game::ENEMY_CIRCLE;
				}

				Game::Enemy_Square sq;

				if (o.start_portal < game->arena_size.w) {
					sq.pos = util::Point<float>(0.5f+o.start_portal, -0.25f);
					sq.speed.x = 0.0f;
					sq.speed.y = BOUNCER_SPEED/2.0f;
				}
				else if (o.start_portal < game->arena_size.w*2) {
					sq.pos = util::Point<float>(0.5f+(o.start_portal-game->arena_size.w), game->arena_size.h+0.25f);
					sq.speed.x = 0.0f;
					sq.speed.y = -BOUNCER_SPEED/2.0f;
				}
				else if (o.start_portal < game->arena_size.w*2+game->arena_size.h) {
					sq.pos = util::Point<float>(-0.25f, 0.5f+(o.start_portal-game->arena_size.w*2));
					sq.speed.x = BOUNCER_SPEED/2.0f;
					sq.speed.y = 0.0f;
				}
				else {
					sq.pos = util::Point<float>(game->arena_size.w+0.25f, 0.5f+(o.start_portal-game->arena_size.w*2-game->arena_size.h));
					sq.speed.x = -BOUNCER_SPEED/2.0f;
					sq.speed.y = 0.0f;
				}

				sq.linear_pos = sq.pos;
				
				sq.starting = true;

				sq.spinning = false;

				sq.shot_angle = 0.0f;

				sq.dead = false;

				o.squares.push_back(sq);

				o.start_time = get_ticks();

				o.weapon_power = 0;

				if (o.type == Game::ENEMY_SMALL_PIXELS) {
					o.barrier = generate_small_pixels(&o.buf);
				}
				else if (o.type == Game::ENEMY_BIG_PIXELS) {
					o.barrier = generate_big_pixels(&o.buf);
				}

				game->enemies.push_back(o);
			}
		}

		if (objects_spawned >= objects_in_wave && game->enemies.size() == 0 && game->powerups.size() == 0) {
			wave++;
			objects_in_wave += 25;
			objects_spawned = 0;
			
			spawn_delay -= 250;
			if (spawn_delay < 500) {
				spawn_delay = 500;
			}

			if (end_enemy < num_enemies-1) {
				end_enemy++;
			}
		}


		for (std::vector<Game::Powerup>::iterator it = game->powerups.begin(); it != game->powerups.end();) {
			Game::Powerup &o = *it;

			o.pos += o.speed;

			if (o.starting) {
				if (o.start_portal < game->arena_size.w) {
					if (o.pos.y >= 0.25f) {
						o.pos.y = 0.25f;
						o.speed.x = util::rand(0, 1) == 0 ? -BOUNCER_SPEED : BOUNCER_SPEED;
						o.speed.y = BOUNCER_SPEED;
						o.starting = false;
					}
				}
				else if (o.start_portal < game->arena_size.w*2) {
					if (o.pos.y <= game->arena_size.h-0.25f) {
						o.pos.y = game->arena_size.h-0.25f;
						o.speed.x = util::rand(0, 1) == 0 ? -BOUNCER_SPEED : BOUNCER_SPEED;
						o.speed.y = -BOUNCER_SPEED;
						o.starting = false;
					}
				}
				else if (o.start_portal < game->arena_size.w*2+game->arena_size.h) {
					if (o.pos.x >= 0.25f) {
						o.pos.x = 0.25f;
						o.speed.x = BOUNCER_SPEED;
						o.speed.y = util::rand(0, 1) == 0 ? -BOUNCER_SPEED : BOUNCER_SPEED;
						o.starting = false;
					}
				}
				else {
					if (o.pos.x <= game->arena_size.w-0.25f) {
						o.pos.x = game->arena_size.w-0.25f;
						o.speed.x = -BOUNCER_SPEED;
						o.speed.y = util::rand(0, 1) == 0 ? -BOUNCER_SPEED : BOUNCER_SPEED;
						o.starting = false;
					}
				}
			}
			else {
				if (o.pos.x < 0.15f) {
					o.pos.x = 0.15f;
					o.speed.x = -o.speed.x;
				}
				if (o.pos.y < 0.15f) {
					o.pos.y = 0.15f;
					o.speed.y = -o.speed.y;
				}
				if (o.pos.x > game->arena_size.w-0.15f) {
					o.pos.x = game->arena_size.w-0.15f;
					o.speed.x = -o.speed.x;
				}
				if (o.pos.y > game->arena_size.h-0.15f) {
					o.pos.y = game->arena_size.h-0.15f;
					o.speed.y = -o.speed.y;
				}
			}

			if (gameover == false && (o.pos-game->player_pos).length() <= 0.1f) {
				if (o.type == 0) {
					int i = util::rand(0, game->crystals.size()-1);
					delete game->crystals[i].barrier;
					delete[] game->crystals[i].buf;
					Uint8 *buf;
					game->crystals[i].barrier = generate_barrier(&buf);
					game->crystals[i].buf = buf;
				}
				else if (o.type == 1) {
					game->weapon_power++;
					if (game->weapon_power > 2) {
						game->weapon_power = 0;
					}
				}
				else if (o.type == 2) {
					int c = util::rand(0, game->crystals.size()-1);
					for (int i = 0; i < 3; i++) {
						int ii = (c + i) % 3;
						if (game->crystals[ii].health <= 0.0f) {
							continue;
						}
						c = ii;
						break;
					}
					game->crystals[c].health += 1.01f/6.0f;
				}
				else {
					for (std::vector<Game::Enemy>::iterator it = game->enemies.begin(); it != game->enemies.end();) {
						Game::Enemy &e = *it;
						for (size_t i = 0; i < e.squares.size(); i++) {
							Bullet_Collision bc;
							bc.pos = e.squares[i].pos;
							bc.start_time = get_ticks();
							bc.weapon_power = 2;
							bc.friendly = true;
							bullet_collisions.push_back(bc);
							explosion_small->play(false);
							if (e.type != Game::ENEMY_SMALL_PIXELS && e.type != Game::ENEMY_BIG_PIXELS) {
								score += 100;
							}
						}
						if (e.type == Game::ENEMY_SMALL_PIXELS) {
							score += 1000;
						}
						else if (e.type == Game::ENEMY_BIG_PIXELS) {
							score += 2500;
						}
						it = game->enemies.erase(it);
					}
				}

				powerup->play(false);

				it = game->powerups.erase(it);

				score += 500;
			}
			else if (get_ticks() - o.start_time > 30000) {
				it = game->powerups.erase(it);
			}
			else {
				it++;
			}
		}

		for (std::vector<Game::Enemy>::iterator it = game->enemies.begin(); it != game->enemies.end();) {
			Game::Enemy &o = *it;

			for (size_t i = 0; i < o.squares.size(); i++) {
				bool done = false;

				if (o.squares[i].starting) {
					o.squares[i].pos += o.squares[i].speed;

					if (o.start_portal < game->arena_size.w) {
						if (o.squares[i].pos.y >= 0.25f) {
							o.squares[i].pos.y = 0.25f;
							o.squares[i].starting = false;
						}
					}
					else if (o.start_portal < game->arena_size.w*2) {
						if (o.squares[i].pos.y <= game->arena_size.h-0.25f) {
							o.squares[i].pos.y = game->arena_size.h-0.25f;
							o.squares[i].starting = false;
						}
					}
					else if (o.start_portal < game->arena_size.w*2+game->arena_size.h) {
						if (o.squares[i].pos.x >= 0.25f) {
							o.squares[i].pos.x = 0.25f;
							o.squares[i].starting = false;
						}
					}
					else {
						if (o.squares[i].pos.x <= game->arena_size.w-0.25f) {
							o.squares[i].pos.x = game->arena_size.w-0.25f;
							o.squares[i].starting = false;
						}
					}
					o.pos = o.squares[i].pos;
					if (o.squares[i].starting == false) {
						if (o.type == Game::ENEMY_SMALL_PIXELS || o.type == Game::ENEMY_BIG_PIXELS) {
							o.squares[i].next_shot = util::rand(1000, 2000) + get_ticks();
						}
						else {
							o.squares[i].next_shot = util::rand(2500, 5000) + get_ticks();
						}
						o.squares[i].path.push_back(o.squares[i].pos);
						o.squares[i].linear_pos = o.squares[i].pos;
						for (int j = 0; j < 3; j++) {
							float x = util::rand(0, 1000) / 1000.0f * (game->arena_size.w - 0.5f) + 0.25f;
							float y = util::rand(0, 1000) / 1000.0f * (game->arena_size.h - 0.5f) + 0.25f;
							if (o.type == Game::ENEMY_STRAIGHT) {
								if (o.next_dir % 2 == 0) {
									x = o.squares[i].path[j].x;
								}
								else {
									y = o.squares[i].path[j].y;
								}
								o.next_dir++;
							}
							o.squares[i].path.push_back(util::Point<float>(x, y));
						}

						float len = (o.squares[i].path[0]-o.squares[i].path[1]).length();
						len = len < 0 ? -len : len;
						int ticks = len / BOUNCER_SPEED;

						o.squares[i].ticks = ticks;

						//o.squares[i].interp_x.start(o.squares[i].path[0].x, o.squares[i].path[0].x, o.squares[i].path[1].x, o.squares[i].path[2].x, ticks);
						//o.squares[i].interp_y.start(o.squares[i].path[0].y, o.squares[i].path[0].y, o.squares[i].path[1].y, o.squares[i].path[2].y, ticks);

						if (o.type == Game::ENEMY_CIRCLE) {
							Game::Enemy_Square &sq = o.squares[i];
							for (int i = 0; i < 3; i++) {
								o.squares.push_back(sq);
							}
							o.start_time = get_ticks();
							done = true;
						}
					}
				}
				else { // not starting
					if (o.type != Game::ENEMY_SPIN || o.squares[i].spinning == false) {
						if (o.type == Game::ENEMY_CIRCLE) {
							if (i == 0) {
								o.angle = get_ticks() / -5000.0f * M_PI * 2;
								float angle = atan2(o.squares[i].path[1].y-o.squares[i].path[0].y, o.squares[i].path[1].x-o.squares[i].path[0].x);
								//float angle = atan2(o.squares[i].path[0].y-o.squares[i].linear_pos.y, o.squares[i].path[0].x-o.squares[i].linear_pos.x);
								o.squares[i].linear_pos.x += cos(angle) * BOUNCER_SPEED;
								o.squares[i].linear_pos.y += sin(angle) * BOUNCER_SPEED;
								//o.squares[i].pos.x = o.squares[i].interp_x.get_value();
								//o.squares[i].pos.y = o.squares[i].interp_y.get_value();
								o.pos = o.squares[i].linear_pos;

								for (size_t j = 0; j < o.squares.size(); j++) {
									util::Point<float> pos = o.pos;
									float angle = (M_PI*2.0f)/o.squares.size()*j;
									Uint32 now = get_ticks();
									Uint32 diff = now - o.start_time;
									float p = diff / 1000.0f;
									if (p > 1.0f) {
										p = 1.0f;
									}
									float len = 0.1f * p;
									pos.x += cos(o.angle+angle) * len;
									pos.y += sin(o.angle+angle) * len;
									o.squares[j].pos = pos;
								}
							}
						}
						else {

							//o.squares[i].interp_x.interpolate(1);
							//o.squares[i].interp_y.interpolate(1);
							float angle = atan2(o.squares[i].path[1].y-o.squares[i].path[0].y, o.squares[i].path[1].x-o.squares[i].path[0].x);
							//float angle = atan2(o.squares[i].path[0].y-o.squares[i].linear_pos.y, o.squares[i].path[0].x-o.squares[i].linear_pos.x);
							o.squares[i].linear_pos.x += cos(angle) * BOUNCER_SPEED;
							o.squares[i].linear_pos.y += sin(angle) * BOUNCER_SPEED;
							//o.squares[i].pos.x = o.squares[i].interp_x.get_value();
							//o.squares[i].pos.y = o.squares[i].interp_y.get_value();
							o.squares[i].pos = o.squares[i].linear_pos;
							o.pos = o.squares[i].pos;
						}

						if (i == 0) {
							o.squares[i].ticks--;
							
							if (o.squares[i].ticks <= 0) {
								float x = util::rand(0, 1000) / 1000.0f * (game->arena_size.w - 0.5f) + 0.25f;
								float y = util::rand(0, 1000) / 1000.0f * (game->arena_size.h - 0.5f) + 0.25f;
								if (o.type == Game::ENEMY_STRAIGHT) {
									if (o.next_dir % 2 == 0) {
										x = o.squares[i].path[o.squares[i].path.size()-1].x;
									}
									else {
										y = o.squares[i].path[o.squares[i].path.size()-1].y;
									}
									o.next_dir++;
								}
								o.squares[i].path.push_back(util::Point<float>(x, y));
								
								o.squares[i].last_popped_path = o.squares[i].path[0];
								o.squares[i].path.erase(o.squares[i].path.begin());

								o.squares[i].path[0] = o.squares[i].linear_pos;

								float len = (o.squares[i].path[0]-o.squares[i].path[1]).length();
								len = len < 0 ? -len : len;
								int ticks = len / BOUNCER_SPEED;

								o.squares[i].ticks = ticks;

								if (o.type == Game::ENEMY_SPIN) {
									o.squares[i].spinning = true;
									o.squares[i].spin_start = get_ticks();
									o.squares[i].next_shot = get_ticks() + 100;
								}
								else if (o.type == Game::ENEMY_STRAIGHT) {
									util::Point<float> dir = o.squares[i].path[1] - o.squares[i].path[0];
									float angle = atan2(dir.y, dir.x);

									Bullet b;

									b.friendly = false;
									b.angle = angle;
									b.pos = o.squares[i].pos;
									float f = 0.075f;
									float sqr2 = sqrtf(f*f+f*f);
									b.pos.x += cos(angle) * sqr2 * 1.05f;
									b.pos.y += sin(angle) * sqr2 * 1.05f;
									
									b.power = 0;

									shot_normal->play(false);

									bullets.push_back(b);
								}
							}
						}

						//o.squares[i].interp_x.start(o.squares[i].last_popped_path.x, o.squares[i].path[0].x, o.squares[i].path[1].x, o.squares[i].path[2].x, ticks);
						//o.squares[i].interp_y.start(o.squares[i].last_popped_path.y, o.squares[i].path[0].y, o.squares[i].path[1].y, o.squares[i].path[2].y, ticks);
					}

					if (o.type != Game::ENEMY_STRAIGHT && get_ticks() > o.squares[i].next_shot) {
						if (o.type != Game::ENEMY_SPIN || o.squares[i].spinning == true) {
							float angle = -(45.0f/360.0f)*M_PI;

							if (o.type == Game::ENEMY_SQUARE) {
								o.squares[i].next_shot = util::rand(2500, 5000) + get_ticks();
								angle = angle*2-M_PI/2.0f*util::rand(0, 3);
							}
							else if (o.type == Game::ENEMY_CIRCLE) {
								angle += o.angle;
								angle += i/(float)o.squares.size()*M_PI*2.0f;
								o.squares[i].next_shot = util::rand(2500, 5000) + get_ticks();
							}
							else if (o.type == Game::ENEMY_SMALL_PIXELS || o.type == Game::ENEMY_BIG_PIXELS) {
								o.squares[i].next_shot = util::rand(1000, 2000) + get_ticks();
							}
							else {
								if (get_ticks() > o.squares[i].spin_start+1000) {
									o.squares[i].spinning = false;
								}
								o.squares[i].next_shot = get_ticks() + 100;
								angle += o.squares[i].shot_angle;
								o.squares[i].shot_angle -= M_PI/2.0f;
							}
							angle += get_ticks() / -2500.0f * M_PI * 2;

							if (o.type == Game::ENEMY_SMALL_PIXELS || o.type == Game::ENEMY_SMALL_PIXELS) {
								// don't spin above
								angle = util::rand(0, 3)/4.0f * M_PI * 2.0f;
							}
							Bullet b;

							b.friendly = false;
							b.angle = angle;
							b.pos = o.squares[i].pos;
							float f = 0.075f;
							float sqr2 = sqrtf(f*f+f*f);
							b.pos.x += cos(angle) * sqr2 * 1.05f;
							b.pos.y += sin(angle) * sqr2 * 1.05f;
							
							b.power = 0;

							shot_normal->play(false);

							bullets.push_back(b);
						}
					}
				}

				if (done) {
					break;
				}
			}

			it++;
		}

		if (gameover == false) {
			bool have_health = false;

			for (size_t i = 0; i < game->crystals.size(); i++) {
				if (game->crystals[i].health > 0.0f) {
					have_health = true;
					break;
				}
			}

			if (have_health == false) {
				gameover = true;
				gameover_start = get_ticks();
				stick_1_x = 0.0f;
				stick_1_y = 0.0f;
				stick_2_x = 0.0f;
				stick_2_y = 0.0f;
				audio::play_music("music/gameover_prerendered.mml");
			}
		}
	}
}

static void loop()
{
	// These keep the logic running at 60Hz and drawing at refresh rate is possible
	// NOTE: screen refresh rate has to be 60Hz or higher for this to work.
	const float target_fps = shim::refresh_rate <= 0 ? 60.0f : shim::refresh_rate;
	Uint32 start = SDL_GetTicks();
	int logic_frames = 0;
	int drawing_frames = 0;
	bool can_draw = true;
	bool can_logic = true;
	std::string old_music_name = "";
#if defined IOS || defined ANDROID
	float old_volume = 1.0f;
#endif
	int curr_logic_rate = shim::logic_rate;

	while (quit == false) {
		if (save_the_settings && get_ticks() >= save_settings_at) {
			save_the_settings = false;
			save_settings();
		}

		// EVENTS
		while (true) {
			SDL_Event sdl_event;
			TGUI_Event *e = nullptr;

			bool all_done = false;

			if (!SDL_PollEvent(&sdl_event)) {
				e = shim::pop_pushed_event();
				if (e == nullptr) {
					all_done = true;
				}
			}

			if (all_done) {
				break;
			}

			if (e == nullptr) {
				if (sdl_event.type == SDL_QUIT) {
					if (can_logic == false) {
						shim::handle_event(&sdl_event);
						quit = true;
						break;
					}
				}
				// right mouse clicks are transformed to escape keys
				else if (can_logic && sdl_event.type == SDL_MOUSEBUTTONDOWN && sdl_event.button.button == SDL_BUTTON_RIGHT) {
					TGUI_Event e;
					e.type = TGUI_KEY_DOWN;
					e.keyboard.code = key_back;
					e.keyboard.is_repeat = false;
					e.keyboard.simulated = true;
					shim::push_event(e);
					e.type = TGUI_KEY_UP;
					shim::push_event(e);
					continue;
				}
				else if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == SDLK_F11) {
					save_settings_at = get_ticks() + 10000;
					save_the_settings = true;
				}
				else if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == SDLK_F12) {
					//load_translation();
				}
			}

			was_paused = paused;
				
			TGUI_Event *event;

			if (e) {
				event = e;
			}
			else {
				if (sdl_event.type == SDL_QUIT) {
					static TGUI_Event quit_event;
					quit_event.type = TGUI_QUIT;
					event = &quit_event;
				}
				else {
					event = shim::handle_event(&sdl_event);
				}
			}

			handle_event(event);
		}

		// Logic rate can change in devsettings
		if (shim::logic_rate != curr_logic_rate) {
			curr_logic_rate = shim::logic_rate;
			logic_frames = 0;
			drawing_frames = 0;
			start = SDL_GetTicks();
		}

		// TIMING
		Uint32 now = SDL_GetTicks();
		int diff = now - start;
		bool skip_drawing = false;
		int logic_reps = diff / 16;

		if (logic_reps > 0) {
			start += 16 * logic_reps;
		}

		for (int logic = 0; logic < logic_reps; logic++) {
			can_draw = shim::update();

			// Generate a timer tick event (TGUI_TICK)
			SDL_Event sdl_event;
			sdl_event.type = shim::timer_event_id;
			TGUI_Event *event = shim::handle_event(&sdl_event);
			handle_event(event);

			game_logic();

			logic_frames++;
		}

		// LOGIC
		if (can_logic) {
			for (int logic = 0; logic < logic_reps; logic++) {
				gfx::update_animations();
				// logic
			}
		}

		// DRAWING
		if (skip_drawing == false && can_draw) {
			draw_all();
		}

		if (quit || (shim::guis.size() == 0 && game == nullptr)) {
			break;
		}

		if (game && gameover && get_ticks() >= gameover_start+14650) {
#ifdef STEAMWORKS
			util::set_steam_leaderboard("highscores", score);
#endif

			if (score >= scores[0].score) {
				Initials_GUI *i = new Initials_GUI(false);
				shim::guis.push_back(i);
			}
			else {
				Title_GUI *title = new Title_GUI(false);
				shim::guis.push_back(title);
			}
			for (size_t i = 0; i < game->crystals.size(); i++) {
				delete[] game->crystals[i].buf;
			}
			delete game;
			game = nullptr;
		}

		drawing_frames++;
	}
}

bool go()
{
	Title_GUI *title = new Title_GUI(false);
	shim::guis.push_back(title);

	loop();

	return true;
}

void end()
{
	//shim::user_render = NULL;

	//delete GAME;
	//GAME = nullptr;

	// If Alt-F4 is pressed the title gui can remain in shim::guis. Leaving it to shim to destruct won't work, because ~Title_GUI accesses Globals which is destroyed below
	for (std::vector<gui::GUI *>::iterator it = shim::guis.begin(); it != shim::guis.end();) {
		gui::GUI *gui = *it;
		delete gui;
		it = shim::guis.erase(it);
	}

	shim::end_all();
}

static int set_orig_args(bool forced, bool count_only)
{
	int count = 0;

	for (int i = 0; i < orig_argc; i++) {
		int skip = 0;
		if (forced &&
			(std::string(orig_argv[i]) == "+windowed" ||
			std::string(orig_argv[i]) == "+fullscreen")) {
			skip = 1;
		}
		else if (forced &&
			(std::string(orig_argv[i]) == "+width" ||
			std::string(orig_argv[i]) == "+height" ||
			std::string(orig_argv[i]) == "+scale")) {
			skip = 2;
		}

		if (skip > 0) {
			i += skip-1;
			continue;
		}

		if (count_only == false) {
			shim::argv[count] = new char[strlen(orig_argv[i])+1];
			strcpy(shim::argv[count], orig_argv[i]);
		}
		
		count++;
	}

	return count;
}

void set_shim_args(bool initial, bool force_windowed, bool force_fullscreen)
{
	if (initial) {
		for (int i = 0; i < orig_argc; i++) {
			if (std::string(orig_argv[i]) == "+windowed" || std::string(orig_argv[i]) == "+fullscreen" || std::string(orig_argv[i]) == "+width" || std::string(orig_argv[i]) == "+height" || std::string(orig_argv[i]) == "+adapter") {
				force_windowed = false;
				force_fullscreen = false;
				break;
			}
		}
	}

	bool force = force_windowed || force_fullscreen;
	
	int count = set_orig_args(force, true);

	if (force) {
		count++;
	}

	std::vector<std::string> v;
	if (extra_args != "") {
		util::Tokenizer t(extra_args, ',');
		std::string tok;
		while ((tok = t.next()) != "") {
			v.push_back(tok);
		}
		count += v.size();
	}
	extra_args = ""; // Do this?

	shim::argc = count;
	shim::argv = new char *[count];

	int i = set_orig_args(force, false);

	if (force_windowed) {
		shim::argv[i] = new char[strlen("+windowed")+1];
		strcpy(shim::argv[i], "+windowed");
		i++;
	}
	else if (force_fullscreen) {
		shim::argv[i] = new char[strlen("+fullscreen")+1];
		strcpy(shim::argv[i], "+fullscreen");
		i++;
	}

	for (auto s : v) {
		shim::argv[i] = new char[s.length()+1];
		strcpy(shim::argv[i], s.c_str());
		i++;
	}
}

int score_sort(Score &a, Score &b)
{
	return a.score < b.score;
}

void init_scores()
{
	scores.clear();
	Score sc;
	sc.name = "AAA";
	sc.score = 100000;
	scores.push_back(sc);
	sc.score = 125000;
	scores.push_back(sc);
	sc.score = 150000;
	scores.push_back(sc);
	sc.score = 175000;
	scores.push_back(sc);
	sc.score = 200000;
	scores.push_back(sc);

	std::sort(scores.begin(), scores.end(), score_sort);
}

void create_game()
{
	score = 0;
	wave = 0;
	objects_in_wave = 25;
	objects_spawned = 0;

	num_enemies = 6;
	start_enemy = 0;
	end_enemy = 0;
	done_all_enemies = false;
	
	spawn_delay = 2500;

	game = new Game;

	game->arena_size = util::Size<int>(3, 3);

	game->player_pos = util::Point<float>(game->arena_size.w/2.0f, game->arena_size.h/2.0f);
	game->next_bullet = 0;
	game->weapon_power = 0;

	create_stars();
	
	paused = false;
	paused_time = 0;

	offsets.clear();

	bullets.clear();

	gameover = false;

	for (int i = 0; i < 3; i++) {
		Game::Crystal c;
		c.pos = {game->arena_size.w/2.0f, game->arena_size.h/2.0f};
		c.pos.x += cos((-90.0f/360.0f)*M_PI*2+M_PI*2.0f/3.0f*i) * 0.5f;
		c.pos.y += sin((-90.0f/360.0f)*M_PI*2+M_PI*2.0f/3.0f*i) * 0.5f;
		c.speed.x = (util::rand(0, 1) == 0 ? -1 : 1) * BOUNCER_SPEED;
		c.speed.y = (util::rand(0, 1) == 0 ? -1 : 1) * BOUNCER_SPEED;
		c.health = 1.0f;
		game->crystals.push_back(c);
	}
	
	generate_barriers();

	game->next_object = get_ticks() + 5000;
}

int main(int argc, char **argv)
{
#ifdef _WIN32
	SDL_RegisterApp("3 Crystals", 0, 0);
#endif

	orig_argc = argc;
	orig_argv = argv;

	// this must be before static_start which inits SDL
#ifdef _WIN32
	bool directsound = util::bool_arg(true, argc, argv, "directsound");
	bool wasapi = util::bool_arg(false, argc, argv, "wasapi");
	bool winmm = util::bool_arg(false, argc, argv, "winmm");

	if (directsound) {
		_putenv_s("SDL_AUDIODRIVER", "directsound");
	}
	else if (wasapi) {
		_putenv_s("SDL_AUDIODRIVER", "wasapi");
	}
	else if (winmm) {
		_putenv_s("SDL_AUDIODRIVER", "winmm");
	}
#endif

	shim::window_title = "3 Crystals";
	shim::organisation_name = "b1stable";
	shim::game_name = "3 Crystals";
	//
	shim::logging = true;
	gfx::Image::ignore_palette = true;

	// Need to be set temporarily to original values so +error-level works. They get changed below
	shim::argc = orig_argc;
	shim::argv = orig_argv;

	//set_shim_args(true, false, true);
	set_shim_args(true, true, false);

	shim::static_start_all();

	util::srand((uint32_t)time(NULL));

	//shim::create_depth_buffer = true;

	init_scores();

	rumble_enabled = true;
	
	Settings_GUI::static_start();

	load_settings();
	load_scores();

	start();

	joy_action = 0;
	joy_back = 1;

	key_action = TGUIK_RETURN;
	key_back = TGUIK_ESCAPE;
	shim::fullscreen_key = TGUIK_F11;

	button_mml = new audio::MML("sfx/button.mml");

	YELLOW.r = 255;
	YELLOW.g = 216;
	YELLOW.b = 0;
	YELLOW.a = 255;

	load_fonts();

	language = "English";

	load_translation();

	last_screen_size = shim::real_screen_size;

	square = new gfx::Image("misc/square.tga");

	if (go_fullscreen) {
		gen_f11();
	}

	shot_normal = new audio::MML("sfx/shot.mml");
	shot_big = new audio::MML("sfx/bigshot.mml");
	explosion_small = new audio::Sample("explosion_small.flac");
	powerup = new audio::Sample("powerup.flac");

	if (input::is_joystick_connected() == false) {
		gui::popup("Gamepad Required", "This game requires a dual analog stick gamepad.", gui::OK);
	}

	small_radius = 0.025f;
	big_radius = 0.05f;

#ifdef STEAMWORKS
	util::load_steam_leaderboard("highscores");
#endif

	go();

	save_settings();
	save_scores();

	end();

	return 0;
}
