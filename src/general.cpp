#include "general.h"
#include "main.h"

SDL_Colour make_translucent(SDL_Colour colour, float alpha)
{
	SDL_Colour c = colour;
	c.r *= alpha;
	c.g *= alpha;
	c.b *= alpha;
	c.a *= alpha;
	return c;
}

void load_translation()
{
	delete game_t;
	delete english_game_t;

	std::string game_t_text = util::load_text(std::string("text/") + language + std::string(".utf8"));
	std::string english_game_t_text = util::load_text("text/English.utf8");

	game_t = new util::Translation(game_t_text);
	english_game_t = new util::Translation(english_game_t_text);
	
	//shim::font->cache_glyphs(game_t->get_entire_translation());
	//title_font->cache_glyphs(game_t->get_entire_translation());
}

Uint32 get_ticks()
{
	if (paused) {
		Uint32 now = SDL_GetTicks();
		Uint32 diff = now - pause_start;
		return now - paused_time - diff;
	}
	else {
		return SDL_GetTicks() - paused_time;
	}
}

void pause_timers()
{
	paused = true;
	pause_start = SDL_GetTicks();
}

void unpause_timers()
{
	paused = false;
	paused_time += SDL_GetTicks() - pause_start;
}

static std::string cfg_filename()
{
	return util::get_appdata_dir() + "/config.txt";
}

void save_settings()
{
	std::string fn = cfg_filename();
	FILE *f = fopen(fn.c_str(), "w");
	fprintf(f, "music=%d\n", shim::music_volume == 0 ? 0 : 1);
	fprintf(f, "sfx=%d\n", shim::sfx_volume == 0 ? 0 : 1);
	fprintf(f, "fullscreen=%d\n", gfx::is_fullscreen() == true ? 1 : 0);
	fprintf(f, "rumble=%d\n", rumble_enabled == true ? 1 : 0);
	fclose(f);
}

void load_settings()
{
	int sz;
	char *bytes;

	try {
		bytes = util::slurp_file_from_filesystem(cfg_filename(), &sz);
	}
	catch (util::Error &e) {
		return;
	}

	std::string s = bytes;

	util::Tokenizer t(s, '\n');

	std::string line;

	while ((line = t.next()) != "") {
		util::Tokenizer t2(line, '=');
		std::string name = t2.next();
		std::string value = t2.next();

		if (name == "music") {
			bool m = atoi(value.c_str()) != 0;
			if (m) {
				shim::music_volume = 1.0f;
			}
			else {
				shim::music_volume = 0.0f;
			}
		}
		else if (name == "sfx") {
			bool s = atoi(value.c_str()) != 0;
			if (s) {
				shim::sfx_volume = 1.0f;
			}
			else {
				shim::sfx_volume = 0.0f;
			}
		}
		else if (name == "fullscreen") {
			go_fullscreen = atoi(value.c_str()) != 0;
		}
		else if (name == "rumble") {
			rumble_enabled = atoi(value.c_str()) != 0;
		}
	}
}

static std::string score_filename()
{
	return util::get_appdata_dir() + "/scores.txt";
}

void save_scores()
{
	std::string fn = score_filename();
	FILE *f = fopen(fn.c_str(), "w");
	for (size_t i = 0; i < 5; i++) {
		fprintf(f, "%s=%d\n", scores[i].name.c_str(), scores[i].score);
	}
	fclose(f);
}

void load_scores()
{
	int sz;
	char *bytes;

	try {
		bytes = util::slurp_file_from_filesystem(score_filename(), &sz);
	}
	catch (util::Error &e) {
		return;
	}

	std::string s = bytes;

	util::Tokenizer t(s, '\n');

	std::string line;

	int i = 0;

	while ((line = t.next()) != "") {
		util::Tokenizer t2(line, '=');
		std::string name = t2.next();
		std::string score_s = t2.next();
		int score = atoi(score_s.c_str());

		if (i < 5) {
			scores[i].name = name.substr(0, 3);
			scores[i].score = score;
			i++;
		}
	}
}

void record_score(std::string name)
{
	std::sort(scores.begin(), scores.end(), score_sort);

	scores.erase(scores.begin());

	Score sc;
	sc.name = name;
	sc.score = score;
	
	scores.push_back(sc);
	
	std::sort(scores.begin(), scores.end(), score_sort);

	save_scores();
}

void gen_f11()
{
	TGUI_Event event;
	event.type = TGUI_KEY_DOWN;
	event.keyboard.is_repeat = false;
	event.keyboard.code = TGUIK_F11;
	event.keyboard.simulated = true;
	shim::push_event(event);
}
