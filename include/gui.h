#ifndef GUI_H
#define GUI_H

#include <shim4/shim4.h>

#include "widgets.h"

class CRY_GUI : public gui::GUI {
public:
	CRY_GUI();
	virtual ~CRY_GUI();
};

class Title_GUI : public CRY_GUI {
public:
	Title_GUI(bool transition);
	virtual ~Title_GUI();

	virtual void draw();
	virtual void update();
	virtual void resize(util::Size<int> new_size);
	void handle_event(TGUI_Event *event);

	void focus_button(int n);

	void play();
	
private:
	TGUI_Widget *container;
	Widget_Text_Button *play_button;
	Widget_Text_Button *scores_button;
	Widget_Text_Button *settings_button;
	Widget_Text_Button *quit_button;
};

class Settings_GUI : public CRY_GUI {
public:
	static void static_start();

	Settings_GUI(bool transition);
	virtual ~Settings_GUI();

	virtual void draw();
	virtual void update();
	virtual void resize(util::Size<int> new_size);
	void handle_event(TGUI_Event *event);

private:
	TGUI_Widget *container;
	Widget_Checkbox *music_checkbox;
	Widget_Checkbox *sfx_checkbox;
#ifdef ANDROID
	Widget_Text_Button *licenses_button;
#else
	Widget_Checkbox *fullscreen_checkbox;
	Widget_Checkbox *rumble_checkbox;
#endif
};

class Pause_GUI : public CRY_GUI {
public:
	Pause_GUI(bool transition);
	virtual ~Pause_GUI();

	virtual void draw();
	virtual void update();
	virtual void resize(util::Size<int> new_size);
	void handle_event(TGUI_Event *event);

private:
	TGUI_Widget *container;
	Widget_Text_Button *resume_button;
	Widget_Text_Button *quit_button;
	float save_vol;
};

class HighScore_GUI : public CRY_GUI {
public:
	HighScore_GUI(bool transition, int next_menu);
	virtual ~HighScore_GUI();

	virtual void draw();
	virtual void update();
	void handle_event(TGUI_Event *event);

private:
	int next_menu;
};

class Initials_GUI : public CRY_GUI {
public:
	Initials_GUI(bool transition);
	virtual ~Initials_GUI();

	virtual void draw();
	virtual void update();
	void handle_event(TGUI_Event *event);

private:
	int curr;
	char initials[3];
};

#endif // GUI_H
