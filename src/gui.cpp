#include "general.h"
#include "gui.h"
#include "main.h"

//--

#ifdef ANDROID
#include <jni.h>

static SDL_mutex *android_mutex;
static SDL_cond *android_cond;
static bool android_bool;

extern "C" {
	JNIEXPORT void JNICALL Java_com_b1stable_threecrystals_ThreeCrystals_1Activity_resume_1after_1showing_1license
	  (JNIEnv *, jclass)
	{
		SDL_LockMutex(android_mutex);
		android_bool = true;
		SDL_CondSignal(android_cond);
		SDL_UnlockMutex(android_mutex);
	}
}

static int get_show_license_result()
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "getShowLicenseResult", "()I");

	int result = env->CallIntMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return result;
}
#endif

CRY_GUI::CRY_GUI()
{
	transition = false;
}

CRY_GUI::~CRY_GUI()
{
}

//--

Title_GUI::Title_GUI(bool transition)
{
	if (transition) {
		this->transition = true;
		transition_is_slide_vertical = true;
	}
	else {
		this->transition = false;
	}

	audio::play_music("music/title_prerendered.mml");

	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	std::string s1 = game_t->translate(0/* Play */);
	std::string s2 = game_t->translate(10/* High Scores */);
	std::string s3 = game_t->translate(1/* Settings */);
	std::string s4 = game_t->translate(2/* Quit */);

	int bw = shim::real_screen_size.w/2;

	container = new TGUI_Widget(bw, SQUARE_W*3+5*2);
	container->set_centre_x(true);
	container->set_parent(modal_main_widget);
	container->set_parent(modal_main_widget);
	container->set_padding_top(int(SQUARE_W*3.5f));

	play_button = new Widget_Text_Button(s1, bw);
	play_button->set_parent(container);
	play_button->set_padding_bottom(5);

	scores_button = new Widget_Text_Button(s2, bw);
	scores_button->set_break_line(true);
	scores_button->set_parent(container);
	scores_button->set_padding_bottom(5);

	settings_button = new Widget_Text_Button(s3, bw);
	settings_button->set_break_line(true);
	settings_button->set_parent(container);
	settings_button->set_padding_bottom(5);

	quit_button = new Widget_Text_Button(s4, bw);
	quit_button->set_break_line(true);
	quit_button->set_parent(container);

	// Wrap cursor
	play_button->set_up_widget(quit_button);
	quit_button->set_down_widget(play_button);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(play_button);
}

Title_GUI::~Title_GUI()
{
}

void Title_GUI::draw()
{
	draw_crystal(util::Point<float>(SQUARE_W*3, (float)shim::screen_size.h)-SQUARE_W*2, (float)shim::screen_size.h*0.75f, 0.0f, 6, 0.25f);

	std::string txt = game_t->translate(3/* 3 Crystals */);
	int txt_w = title_font->get_text_width(txt);

	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f+1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f+1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f+1});
	
	txt = game_t->translate(4)/* © 2022 b1stable */;
	txt_w = small_font->get_text_width(txt);
	
	small_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f, shim::screen_size.h-SQUARE_W*1.5f});

	GUI::draw();
}

void Title_GUI::update()
{
	CRY_GUI::update();

	if (quit_button->pressed()) {
		exit();
	}
	else if (play_button->pressed()) {
		create_game();
		audio::play_music("music/game_prerendered.mml");
		exit();
	}
	else if (scores_button->pressed()) {
		HighScore_GUI *s = new HighScore_GUI(false, 1);
		//Initials_GUI *s = new Initials_GUI(false);
		shim::guis.push_back(s);
		exit();
	}
	else if (settings_button->pressed()) {
		Settings_GUI *s = new Settings_GUI(false);
		shim::guis.push_back(s);
		exit();
	}
}

void Title_GUI::resize(util::Size<int> new_size)
{
	container->set_padding_top(int(SQUARE_W*3.5f));
	container->set_width(new_size.w/2);
	play_button->set_size();
	scores_button->set_size();
	settings_button->set_size();
	quit_button->set_size();
	play_button->set_width(new_size.w/2);
	scores_button->set_width(new_size.w/2);
	settings_button->set_width(new_size.w/2);
	quit_button->set_width(new_size.w/2);
	gui->resize(new_size.w, new_size.h);
	gui->layout();
}

void Title_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == joy_back)) {
		button_mml->play(false);
		exit();
	}
	else
	{
		gui::GUI::handle_event(event);
	}
}

void Title_GUI::focus_button(int n)
{
	switch (n) {
		case 0:
			gui->set_focus(play_button);
			break;
		case 1:
			gui->set_focus(scores_button);
			break;
		case 2:
			gui->set_focus(settings_button);
			break;
		case 3:
			gui->set_focus(quit_button);
			break;
	}
}

//--

void Settings_GUI::static_start()
{
#ifdef ANDROID
	android_bool = true;
#endif
}

Settings_GUI::Settings_GUI(bool transition)
{
	if (transition) {
		this->transition = true;
		transition_is_slide_vertical = true;
	}
	else {
		this->transition = false;
	}

	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	std::string s1 = game_t->translate(5/* Music */);
	std::string s2 = game_t->translate(6/* SFX */);
	std::string s3 = game_t->translate(7/* Fullscreen */);
#ifdef ANDROID
	std::string s4 = game_t->translate(13/* Licenses */);
#else
	std::string s4 = game_t->translate(12/* Rumble */);
#endif

	int bw = shim::real_screen_size.w/2;

	container = new TGUI_Widget(bw, SQUARE_W*2+5*1);
	container->set_centre_x(true);
	container->set_parent(modal_main_widget);
	container->set_parent(modal_main_widget);
	container->set_padding_top(int(SQUARE_W*3.5f));

	music_checkbox = new Widget_Checkbox(shim::music_volume != 0.0f, s1, bw);
	music_checkbox->set_parent(container);
	music_checkbox->set_padding_bottom(5);

	sfx_checkbox = new Widget_Checkbox(shim::sfx_volume != 0.0f, s2, bw);
	sfx_checkbox->set_break_line(true);
	sfx_checkbox->set_parent(container);
	sfx_checkbox->set_padding_bottom(5);

#ifdef ANDROID
	licenses_button = new Widget_Text_Button(s4, bw);
	licenses_button->set_break_line(true);
	licenses_button->set_parent(container);
#else
	fullscreen_checkbox = new Widget_Checkbox(gfx::is_fullscreen(), s3, bw);
	fullscreen_checkbox->set_break_line(true);
	fullscreen_checkbox->set_parent(container);
	fullscreen_checkbox->set_padding_bottom(5);

	rumble_checkbox = new Widget_Checkbox(rumble_enabled, s4, bw);
	rumble_checkbox->set_break_line(true);
	rumble_checkbox->set_parent(container);
#endif

	// Wrap cursor
#ifdef ANDROID
	music_checkbox->set_up_widget(licenses_button);
	licenses_button->set_down_widget(music_checkbox);
#else
	music_checkbox->set_up_widget(rumble_checkbox);
	rumble_checkbox->set_down_widget(music_checkbox);
#endif

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(music_checkbox);
}

Settings_GUI::~Settings_GUI()
{
}

void Settings_GUI::draw()
{
	draw_crystal(util::Point<float>(SQUARE_W*3, (float)shim::screen_size.h)-SQUARE_W*2, (float)shim::screen_size.h*0.75f, 0.0f, 6, 0.25f);
	
	std::string txt = game_t->translate(1/* Settings */);
	int txt_w = title_font->get_text_width(txt);

	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f+1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f+1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f+1});

	GUI::draw();
}

void Settings_GUI::update()
{
	CRY_GUI::update();

#ifdef ANDROID
	if (licenses_button->pressed()) {
		if (android_bool == true) {
			android_bool = false;

			int result = 1;

			android_mutex = SDL_CreateMutex();

			if (android_mutex) {
				android_cond = SDL_CreateCond();

				if (android_cond) {
					JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
					jobject activity = (jobject)SDL_AndroidGetActivity();
					jclass clazz(env->GetObjectClass(activity));

					jmethodID method_id = env->GetMethodID(clazz, "showLicense", "()V");

					env->CallVoidMethod(activity, method_id);

					env->DeleteLocalRef(activity);
					env->DeleteLocalRef(clazz);

					float old_volume = shim::music->get_master_volume();

					std::string music_backup = shim::music->get_name();
					audio::stop_music();

					SDL_LockMutex(android_mutex);
					while (!android_bool) {
						SDL_CondWait(android_cond, android_mutex);
					}
					SDL_UnlockMutex(android_mutex);

					SDL_DestroyCond(android_cond);

					result = get_show_license_result();

					shim::music = new audio::MML(music_backup);
					shim::music->play(old_volume, true);
				}

				SDL_DestroyMutex(android_mutex);
			}

			if (result == 1) {
				// failed, should show a message
			}
		}
	}
#endif
}

void Settings_GUI::resize(util::Size<int> new_size)
{
	container->set_padding_top(int(SQUARE_W*3.5f));
	container->set_width(new_size.w/2);
	music_checkbox->set_size();
	sfx_checkbox->set_size();
#ifdef ANDROID
	licenses_button->set_size();
#else
	fullscreen_checkbox->set_size();
	rumble_checkbox->set_size();
#endif
	music_checkbox->set_width(new_size.w/2);
	sfx_checkbox->set_width(new_size.w/2);
#ifdef ANDROID
	licenses_button->set_width(new_size.w/2);
#else
	fullscreen_checkbox->set_width(new_size.w/2);
	rumble_checkbox->set_width(new_size.w/2);
#endif
	gui->resize(new_size.w, new_size.h);
	gui->layout();
}

void Settings_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == joy_back)) {
		Title_GUI *t = new Title_GUI(false);
		t->focus_button(2);
		shim::guis.push_back(t);
		button_mml->play(false);
		save_settings();
		exit();
	}
	else
	{
		bool m = music_checkbox->is_checked();
		bool s = sfx_checkbox->is_checked();
#ifndef ANDROID
		bool f = fullscreen_checkbox->is_checked();
		bool r = rumble_checkbox->is_checked();
#endif

		gui::GUI::handle_event(event);

		if (music_checkbox->is_checked() != m) {
			if (music_checkbox->is_checked()) {
				shim::music_volume = 1.0f;
				shim::music->set_master_volume(shim::music_volume);
			}
			else {
				shim::music_volume = 0.0f;
				shim::music->set_master_volume(shim::music_volume);
			}
		}

		if (sfx_checkbox->is_checked() != s) {
			if (sfx_checkbox->is_checked()) {
				shim::sfx_volume = 1.0f;
			}
			else {
				shim::sfx_volume = 0.0f;
			}
		}

#ifndef ANDROID
		if (fullscreen_checkbox->is_checked() != f) {
			gen_f11();
		}
		else if (fullscreen_checkbox->is_checked() != gfx::is_fullscreen()) {
			fullscreen_checkbox->set_checked(gfx::is_fullscreen());
		}

		if (rumble_checkbox->is_checked() != r) {
			rumble_enabled = !rumble_enabled;
			if (rumble_enabled) {
				input::rumble(250);
			}
		}
#endif
	}
}

//--

Pause_GUI::Pause_GUI(bool transition)
{
	save_vol = shim::music_volume;
	shim::music_volume *= 0.5f;
	audio::play_music("music/pause_prerendered.mml");

	if (transition) {
		this->transition = true;
		transition_is_slide_vertical = true;
	}
	else {
		this->transition = false;
	}

	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	std::string s1 = game_t->translate(8/* Resume */);
	std::string s2 = game_t->translate(2/* Quit */);

	int bw = shim::real_screen_size.w/2;

	container = new TGUI_Widget(bw, SQUARE_W*2+5);
	container->set_centre_x(true);
	container->set_parent(modal_main_widget);
	container->set_parent(modal_main_widget);
	container->set_padding_top(int(SQUARE_W*3.5f));

	resume_button = new Widget_Text_Button(s1, bw);
	resume_button->set_parent(container);
	resume_button->set_padding_bottom(5);

	quit_button = new Widget_Text_Button(s2, bw);
	quit_button->set_break_line(true);
	quit_button->set_parent(container);

	// Wrap cursor
	resume_button->set_up_widget(quit_button);
	quit_button->set_down_widget(resume_button);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(resume_button);
}

Pause_GUI::~Pause_GUI()
{
}

void Pause_GUI::draw()
{
	//gfx::clear(YELLOW);
	gfx::clear(shim::black);

	draw_crystal(util::Point<float>(SQUARE_W*3, (float)shim::screen_size.h)-SQUARE_W*2, (float)shim::screen_size.h*0.75f, 0.0f, 6, 0.25f);

	std::string txt = game_t->translate(9/* Paused */);
	int txt_w = title_font->get_text_width(txt);

	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f+1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f+1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f+1});
	
	GUI::draw();
}

void Pause_GUI::update()
{
	CRY_GUI::update();

	if (quit_button->pressed()) {
		shim::music_volume = save_vol;
		audio::play_music("music/game_prerendered.mml");
		delete game;
		game = nullptr;
		Title_GUI *t = new Title_GUI(false);
		shim::guis.push_back(t);
		unpause_timers();
		exit();
	}
	else if (resume_button->pressed()) {
		shim::music_volume = save_vol;
		audio::play_music("music/game_prerendered.mml");
		unpause_timers();
		exit();
	}
}

void Pause_GUI::resize(util::Size<int> new_size)
{
	container->set_padding_top(int(SQUARE_W*3.5f));
	container->set_width(new_size.w/2);
	resume_button->set_size();
	quit_button->set_size();
	resume_button->set_width(new_size.w/2);
	quit_button->set_width(new_size.w/2);
	gui->resize(new_size.w, new_size.h);
	gui->layout();
}

void Pause_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == joy_back)) {
		shim::music_volume = save_vol;
		audio::play_music("music/game_prerendered.mml");
		button_mml->play(false);
		unpause_timers();
		exit();
	}
	else
	{
		gui::GUI::handle_event(event);
	}
}

//--

HighScore_GUI::HighScore_GUI(bool transition, int next_menu) :
	next_menu(next_menu)
{
	if (transition) {
		this->transition = true;
		transition_is_slide_vertical = true;
	}
	else {
		this->transition = false;
	}
}

HighScore_GUI::~HighScore_GUI()
{
}

void HighScore_GUI::draw()
{
	draw_crystal(util::Point<float>(SQUARE_W*3, (float)shim::screen_size.h)-SQUARE_W*2, (float)shim::screen_size.h*0.75f, 0.0f, 6, 0.25f);

	std::string txt = game_t->translate(10/* High Scores */);
	int txt_w = title_font->get_text_width(txt);

	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f+1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f+1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f+1});

	for (int i = 0; i < 5; i++) {
		std::string txt = scores[i].name;
		std::string txt2 = util::string_printf("%08d", scores[i].score);
		int col1_w = shim::font->get_text_width("AAA") + SQUARE_W/5.0f;
		int col2_w = 0;
		for (int j = 0; j < 5; j++) {
			col2_w = MAX(col2_w, shim::font->get_text_width(util::string_printf("%08d", scores[j].score)));
		}
		int w = col1_w + col2_w + SQUARE_W/2.0f;
		float x1 = shim::screen_size.w/2.0f-w/2.0f;
		float x2 = x1 + col1_w + SQUARE_W/2.0f;
		float y = SQUARE_W*3.5f + (4-i)*shim::font->get_height();
		shim::font->draw(YELLOW, txt, {x1, y});
		shim::font->draw(YELLOW, txt2, {x2, y});
	}
	
	GUI::draw();
}

void HighScore_GUI::update()
{
	CRY_GUI::update();
}

void HighScore_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == joy_back)) {
		button_mml->play(false);
		Title_GUI *t = new Title_GUI(false);
		t->focus_button(next_menu);
		shim::guis.push_back(t);
		exit();
	}
	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == key_action) || (event->type == TGUI_JOY_DOWN && event->joystick.button == joy_action)) {
		button_mml->play(false);
		Title_GUI *t = new Title_GUI(false);
		t->focus_button(next_menu);
		shim::guis.push_back(t);
		exit();
	}
	else
	{
		gui::GUI::handle_event(event);
	}
}

//--

Initials_GUI::Initials_GUI(bool transition)
{
	if (transition) {
		this->transition = true;
		transition_is_slide_vertical = true;
	}
	else {
		this->transition = false;
	}

	curr = 0;
	initials[0] = 'A';
	initials[1] = 'A';
	initials[2] = 'A';
}

Initials_GUI::~Initials_GUI()
{
}

void Initials_GUI::draw()
{
	draw_crystal(util::Point<float>(SQUARE_W*3, (float)shim::screen_size.h)-SQUARE_W*2, (float)shim::screen_size.h*0.75f, 0.0f, 6, 0.25f);

	std::string txt = game_t->translate(11/* Enter Initials: */);
	int txt_w = title_font->get_text_width(txt);

	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f+0});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+0, SQUARE_W*1.5f+1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f+1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f+1, SQUARE_W*1.5f-1});
	title_font->draw(YELLOW, txt, {shim::screen_size.w/2.0f-txt_w/2.0f-1, SQUARE_W*1.5f+1});

	for (int i = 0; i < 3; i++) {
		int w = SQUARE_W*3+(SQUARE_W/8.0f)*2;
		int a_w = shim::font->get_text_width("A");
		int a_h = shim::font->get_height();
		float rx = shim::screen_size.w/2.0f-w/2.0f + i * SQUARE_W + i * (SQUARE_W/8.0f);
		float x = rx + SQUARE_W/2 - a_w/2;
		float ry = SQUARE_W*3.5f;
		float y = ry + SQUARE_W/2 - a_h/2;
		gfx::draw_filled_rectangle(YELLOW, {rx, ry}, {(float)SQUARE_W, (float)SQUARE_W});
		if (i == curr) {
			gfx::draw_filled_rectangle(shim::black, {rx+SQUARE_W/8.0f, ry+SQUARE_W/8.0f}, {SQUARE_W*3.0f/4.0f, SQUARE_W*3.0f/4.0f});
		}
		char s[2];
		s[0] = initials[i];
		s[1] = 0;
		std::string in = s;
		SDL_Colour col;
		if (i == curr) {
			col = YELLOW;
		}
		else {
			col = shim::black;
		}
		shim::font->draw(col, in, {x+1, y});
		shim::font->draw(col, in, {x, y+1});
		shim::font->draw(col, in, {x+1, y+1});
		shim::font->draw(col, in, {x-1, y});
		shim::font->draw(col, in, {x, y-1});
		shim::font->draw(col, in, {x-1, y-1});
		shim::font->draw(col, in, {x+1, y-1});
		shim::font->draw(col, in, {x-1, y+1});
		shim::font->draw(col, in, {x, y});
	}

	GUI::draw();
}

void Initials_GUI::update()
{
	CRY_GUI::update();
}

void Initials_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == key_action) || (event->type == TGUI_JOY_DOWN && event->joystick.button == joy_action)) {
		button_mml->play(false);
		if (curr == 2) {
			char name[4];
			name[0] = initials[0];
			name[1] = initials[1];
			name[2] = initials[2];
			name[3] = 0;
			record_score(std::string(name));
			HighScore_GUI *t = new HighScore_GUI(false, 0);
			shim::guis.push_back(t);
			exit();
		}
		else {
			curr++;
		}
	}
	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == joy_back)) {
		button_mml->play(false);
		initials[curr] = 'A';
		if (curr > 0) {
			curr--;
		}
	}
	else if (event->type == TGUI_FOCUS && event->focus.type == TGUI_FOCUS_UP) {
		shim::widget_sfx->play(false);
		if (initials[curr] == 'Z') {
			initials[curr] = '0';
		}
		else if (initials[curr] == '9') {
			initials[curr] = 'A';
		}
		else {
			initials[curr]++;
		}
	}
	else if (event->type == TGUI_FOCUS && event->focus.type == TGUI_FOCUS_DOWN) {
		shim::widget_sfx->play(false);
		if (initials[curr] == 'A') {
			initials[curr] = '9';
		}
		else if (initials[curr] == '0') {
			initials[curr] = 'Z';
		}
		else {
			initials[curr]--;
		}
	}
	else
	{
		gui::GUI::handle_event(event);
	}
}
