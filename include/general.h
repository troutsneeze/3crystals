#ifndef GENERAL_H
#define GENERAL_H

#include <shim4/shim4.h>

SDL_Colour make_translucent(SDL_Colour colour, float alpha);
void load_translation();
Uint32 get_ticks();
void pause_timers();
void unpause_timers();
void load_settings();
void save_settings();
void load_scores();
void save_scores();
void record_score(std::string name);
void gen_f11();

#endif // GENERAL_H
