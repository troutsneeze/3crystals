#ifndef WIDGETS_H
#define WIDGETS_H

class Widget : public TGUI_Widget {
public:
	Widget(int w, int h);
	Widget(float percent_w, float percent_h);
	Widget(int w, float percent_h);
	Widget(float percent_w, int h);
	Widget(TGUI_Widget::Fit fit, int other);
	Widget(TGUI_Widget::Fit fit, float percent_other);
	Widget(); // Fit both
	virtual ~Widget();

	virtual void draw();

	bool is_focussed();

	void set_background_colour(SDL_Colour colour);

protected:
	void start();

	SDL_Colour background_colour;
};

class Widget_Window : public Widget
{
public:
	Widget_Window(int w, int h);
	Widget_Window(float percent_w, float percent_h);
	Widget_Window(int w, float percent_h);
	Widget_Window(float percent_w, int h);
	Widget_Window(TGUI_Widget::Fit fit, int other);
	Widget_Window(TGUI_Widget::Fit fit, float percent_other);
	virtual ~Widget_Window();

	void draw();

	void set_image(gfx::Image *image);
	void set_alpha(float alpha);

protected:
	void start();

	gfx::Image *image;
	float alpha;
};

class Widget_Button : public Widget {
public:
	Widget_Button(int w, int h);
	Widget_Button(float w, float h);
	Widget_Button(int w, float h);
	Widget_Button(float w, int h);
	virtual ~Widget_Button();

	virtual void handle_event(TGUI_Event *event);

	virtual bool pressed();

	void set_sound_enabled(bool enabled);

	void set_pressed(bool pressed);

	void set_mouse_only(bool mouse_only);

protected:
	void start();

	bool _pressed;
	bool _released;
	bool _hover;
	bool gotten;
	bool sound_enabled;
	bool mouse_only;
};

class Widget_Text_Button : public Widget_Button
{
public:
	Widget_Text_Button(std::string text);
	Widget_Text_Button(std::string text, int force_w = 0);
	virtual ~Widget_Text_Button();

	void draw();

	void set_enabled(bool enabled);
	bool is_enabled();
	void set_text(std::string text);

	void set_size();

protected:
	std::string text;

	bool enabled;

	int force_w;
};

class Widget_Checkbox : public Widget
{
public:
	Widget_Checkbox(bool checked, std::string text);
	Widget_Checkbox(bool checked, std::string text, int force_w = 0);
	virtual ~Widget_Checkbox();

	void draw();
	virtual void handle_event(TGUI_Event *event);

	void set_enabled(bool enabled);
	bool is_enabled();
	void set_text(std::string text);

	void set_size();

	bool is_checked();
	void set_checked(bool checked);

protected:
	std::string text;

	bool enabled;

	int force_w;

	bool checked;
};

#endif // WIDGETS_H
