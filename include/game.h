#ifndef GAME_H
#define GAME_H

struct Game
{
	util::Size<int> arena_size;

	util::Point<float> player_pos;
	Uint32 next_bullet;
	int weapon_power;

	struct Star {
		util::Point<float> pos;
		SDL_Colour colour;
		Uint32 twinkle;
		Uint32 twinkle_end;
	};

	std::vector<Star> stars;

	struct Crystal {
		util::Point<float> pos;
		util::Point<float> speed;
		float health;
		gfx::Image *barrier;
		Uint8 *buf;
	};

	std::vector<Crystal> crystals;

	struct Entity {
		Uint32 start_time;
		int start_portal;
		int type;
	};

	struct Powerup : public Entity {
		bool starting;
		util::Point<float> pos;
		util::Point<float> speed;
		SDL_Colour colour;
		Uint32 start_time;
	};

	struct Enemy_Square {
		bool starting;
		util::Point<float> pos;
		util::Point<float> linear_pos;
		util::Point<float> speed;
		std::vector< util::Point<float> > path;
		util::Point<float> last_popped_path;
		//math::I_Hermite interp_x;
		//math::I_Hermite interp_y;
		int ticks;
		Uint32 next_shot;
		Uint32 spin_start;
		float shot_angle;
		bool spinning;
		bool dead;
	};

	enum Enemy_Type {
		ENEMY_NONE,
		ENEMY_SQUARE,
		ENEMY_SPIN,
		ENEMY_STRAIGHT,
		ENEMY_CIRCLE,
		ENEMY_SMALL_PIXELS,
		ENEMY_BIG_PIXELS
	};

	struct Enemy : public Entity {
		std::vector< Enemy_Square > squares;
		int weapon_power;
		Enemy_Type type;
		int next_dir;
		float angle;
		util::Point<float> pos;
		Uint32 start_time;
		gfx::Image *barrier;
		Uint8 *buf;
	};

	std::vector<Powerup> powerups;
	std::vector<Enemy> enemies;

	Uint32 next_object;
};

#endif // GAME_H
