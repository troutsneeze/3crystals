#include <shim4/shim4.h>

#include "general.h"
#include "gui.h"
#include "main.h"
#include "widgets.h"

void Widget::set_background_colour(SDL_Colour colour)
{
	background_colour = colour;
}

void Widget::start()
{
	background_colour.r = 0;
	background_colour.g = 0;
	background_colour.b = 0;
	background_colour.a = 0;
}

Widget::Widget(int w, int h) :
	TGUI_Widget(w, h)
{
	start();
}

Widget::Widget(float percent_w, float percent_h) :
	TGUI_Widget(percent_w, percent_h)
{
	start();
}

Widget::Widget(int w, float percent_h) :
	TGUI_Widget(w, percent_h)
{
	start();
}

Widget::Widget(float percent_w, int h) :
	TGUI_Widget(percent_w, h)
{
	start();
}

Widget::Widget(TGUI_Widget::Fit fit, int other) :
	TGUI_Widget(fit, other)
{
	start();
}

Widget::Widget(TGUI_Widget::Fit fit, float percent_other) :
	TGUI_Widget(fit, percent_other)
{
	start();
}

Widget::Widget() :
	TGUI_Widget()
{
	start();
}

Widget::~Widget()
{
}

void Widget::draw()
{
	// This is used to clear the background to a darker colour, so don't do it unless this widget is part of
	// the topmost gui because it could happen twice giving a darker colour

	if (shim::guis.size() > 0) {
		TGUI_Widget *root = shim::guis.back()->gui->get_main_widget();
		TGUI_Widget *w = this;
		TGUI_Widget *parent;
		while ((parent = w->get_parent()) != NULL) {
			w = parent;
		}
		if (root != w) {
			return;
		}
	}

	if (background_colour.a != 0) {
		// Need to clear transforms temporarily because it might be part of a transition
		glm::mat4 old_mv, old_p;
		gfx::get_matrices(old_mv, old_p);
		gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
		gfx::update_projection();
		gfx::draw_filled_rectangle(background_colour, util::Point<int>(calculated_x, calculated_y), util::Size<int>(calculated_w, calculated_h));
		gfx::set_matrices(old_mv, old_p);
		gfx::update_projection();
	}
}

bool Widget::is_focussed()
{
	if (shim::guis.size() == 0) {
		return false;
	}
	if (gui != shim::guis.back()->gui) {
		return false;
	}
	return gui->get_focus() == this;
}

//--

Widget_Window::Widget_Window(int w, int h) :
	Widget(w, h)
{
	start();
}

Widget_Window::Widget_Window(float percent_w, float percent_h) :
	Widget(percent_w, percent_h)
{
	start();
}

Widget_Window::Widget_Window(int w, float percent_h) :
	Widget(w, percent_h)
{
	start();
}

Widget_Window::Widget_Window(float percent_w, int h) :
	Widget(percent_w, h)
{
	start();
}

Widget_Window::Widget_Window(TGUI_Widget::Fit fit, int other) :
	Widget(fit, other)
{
	start();
}

Widget_Window::Widget_Window(TGUI_Widget::Fit fit, float percent_other) :
	Widget(fit, percent_other)
{
	start();
}

void Widget_Window::start()
{
	image = nullptr;
	alpha = 1.0f;
}

Widget_Window::~Widget_Window()
{
}

void Widget_Window::draw()
{
	if (image == nullptr) {
		return;
	}

	gfx::draw_9patch_tinted(make_translucent(shim::white, alpha), image, util::Point<int>(calculated_x, calculated_y), util::Size<int>(calculated_w, calculated_h));
}
	
void Widget_Window::set_image(gfx::Image *image)
{
	this->image = image;
}

void Widget_Window::set_alpha(float alpha)
{
	this->alpha = alpha;
}

//--

Widget_Button::Widget_Button(int w, int h) :
	Widget(w, h)
{
	start();
}

Widget_Button::Widget_Button(float w, float h) :
	Widget(w, h)
{
	start();
}

Widget_Button::Widget_Button(int w, float h) :
	Widget(w, h)
{
	start();
}

Widget_Button::Widget_Button(float w, int h) :
	Widget(w, h)
{
	start();
}

void Widget_Button::start()
{
	_pressed = false;
	_released = false;
	_hover = false;
	gotten = true;
	sound_enabled = true;
	accepts_focus = true;
	mouse_only = false;
}

Widget_Button::~Widget_Button()
{
}

void Widget_Button::handle_event(TGUI_Event *event)
{
	if (mouse_only && (event->type == TGUI_KEY_DOWN || event->type == TGUI_KEY_UP || event->type == TGUI_JOY_DOWN || event->type == TGUI_JOY_UP)) {
		return;
	}

	int x, y;

	if (use_relative_position) {
		x = relative_x;
		y = relative_y;
	}
	else {
		x = calculated_x;
		y = calculated_y;
	}

	if (event->type == TGUI_MOUSE_AXIS) {
		if (event->mouse.x >= x && event->mouse.x < x+calculated_w && event->mouse.y >= y && event->mouse.y < y+calculated_h) {
			_hover = true;
		}
		else {
			_hover = false;
		}
	}
	if (accepts_focus && gui->get_event_owner(event) == this) {
		if (event->type == TGUI_KEY_DOWN && event->keyboard.is_repeat == false) {
			if (event->keyboard.code == key_action) {
				if (gotten) {
					_pressed = true;
					_hover = true;
				}
			}
			else {
				_pressed = false;
				_hover = false;
			}
		}
		else if (event->type == TGUI_JOY_DOWN && event->joystick.is_repeat == false) {
			if (event->joystick.button == joy_action) {
				if (gotten) {
					_pressed = true;
					_hover = true;
				}
			}
			else {
				_pressed = false;
				_hover = false;
			}
		}
		else if (event->type == TGUI_MOUSE_DOWN && event->mouse.is_repeat == false) {
			if (event->mouse.button == 1) {
				if (gotten) {
					_pressed = true;
				}
			}
			else {
				_pressed = false;
			}
			_hover = true;
		}
		else if (event->type == TGUI_KEY_UP && event->keyboard.is_repeat == false) {
			if (_pressed && event->keyboard.code == key_action) {
				if (gotten) {
					gotten = false;
					_released = true;
					_hover = false;
					if (sound_enabled) {
						if (button_mml != 0) {
							button_mml->play(false);
						}
					}
				}
			}
			else {
				_pressed = false;
				_hover = false;
			}
		}
		else if (event->type == TGUI_JOY_UP && event->joystick.is_repeat == false) {
			if (_pressed && (event->joystick.button == joy_action)) {
				if (gotten) {
					gotten = false;
					_released = true;
					_hover = false;
					if (sound_enabled) {
						if (button_mml != 0) {
							button_mml->play(false);
						}
					}
				}
			}
			else {
				_pressed = false;
				_hover = false;
			}
		}
		else if (event->type == TGUI_MOUSE_UP && event->mouse.is_repeat == false) {
			if (_pressed && (event->mouse.button == 1)) {
				if (gotten) {
					gotten = false;
					_released = true;
					if (sound_enabled) {
						if (button_mml != 0) {
							button_mml->play(false);
						}
					}
				}
			}
			else {
				_pressed = false;
			}
		}
	}
	else {
		if (event->type == TGUI_KEY_UP) {
			_pressed = false;
			_hover = false;
		}
		else if (event->type == TGUI_JOY_UP) {
			_pressed = false;
			_hover = false;
		}
		else if (event->type == TGUI_MOUSE_UP && event->mouse.is_repeat == false) {
			_pressed = false;
			_hover = false;
		}
		else if ((event->type == TGUI_MOUSE_DOWN || event->type == TGUI_MOUSE_AXIS) && event->mouse.is_repeat == false) {
			_hover = false;
		}
	}
}

bool Widget_Button::pressed()
{
	bool r = _released;
	if (_released) {
		_pressed = _released = false;
	}
	gotten = true;
	return r;
}

void Widget_Button::set_sound_enabled(bool enabled)
{
	sound_enabled = enabled;
}

void Widget_Button::set_pressed(bool pressed)
{
	this->_pressed = this->_released = pressed;
}

void Widget_Button::set_mouse_only(bool mouse_only)
{
	this->mouse_only = mouse_only;
}

//--

Widget_Text_Button::Widget_Text_Button(std::string text) :
	Widget_Button(0, 0),
	text(text),
	force_w(0)
{
	enabled = true;
	set_size();
}

Widget_Text_Button::Widget_Text_Button(std::string text, int force_w) :
	Widget_Button(0, 0),
	text(text),
	force_w(force_w)
{
	enabled = true;
	set_size();
}

Widget_Text_Button::~Widget_Text_Button()
{
}

void Widget_Text_Button::draw()
{
	bool focussed = is_focussed();

	int x, y;

	if (use_relative_position) {
		x = relative_x;
		y = relative_y;
	}
	else {
		x = calculated_x;
		y = calculated_y;
	}

	int tw = shim::font->get_text_width(text);
	int th = shim::font->get_height();

	int w = calculated_w;
	int h = calculated_h;

	Uint32 t = SDL_GetTicks();
	float p = (t % 1000) / 1000.0f;
	if (p <= 0.5f) {
		p /= 0.5f;
	}
	else {
		p -= 0.5f;
		p /= 0.5f;
		p = 1.0f - p;
	}

	SDL_Colour c;
	if (focussed && gui == shim::guis.back()->gui) {
		c.r = 0;
		c.g = 216 * p;
		c.b = 255 * p;
		c.a = 255 * p;
	}
	else {
		c.r = 0;
		c.g = 0;
		c.b = 0;
		c.a = 0;
	}

	gfx::draw_filled_rectangle(c, util::Point<float>(x, y) + (_pressed && _hover ? util::Point<int>(1, 1) : util::Point<int>(0, 0)), util::Size<int>(w, h));
	shim::font->draw(YELLOW, text, util::Point<float>(x, y) + (_pressed && _hover ? util::Point<int>(1, 1) : util::Point<int>(0, 0)) + util::Point<int>(w/2-tw/2, h/2-th/2));
}

void Widget_Text_Button::set_size()
{
	w = force_w > 0 ? force_w : shim::font->get_text_width(text) + 20;
	h = SQUARE_W;
}

void Widget_Text_Button::set_enabled(bool enabled)
{
	this->enabled = enabled;
	if (enabled == true) {
		accepts_focus = true;
	}
	else {
		accepts_focus = false;
	}
}

bool Widget_Text_Button::is_enabled()
{
	return enabled;
}

void Widget_Text_Button::set_text(std::string text)
{
	this->text = text;
	set_size();
}

//--

Widget_Checkbox::Widget_Checkbox(bool checked, std::string text) :
	Widget(0, 0),
	text(text),
	force_w(0),
	checked(checked)
{
	enabled = true;
	accepts_focus = true;
	set_size();
}

Widget_Checkbox::Widget_Checkbox(bool checked, std::string text, int force_w) :
	Widget(0, 0),
	text(text),
	force_w(force_w),
	checked(checked)
{
	enabled = true;
	accepts_focus = true;
	set_size();
}

Widget_Checkbox::~Widget_Checkbox()
{
}

void Widget_Checkbox::draw()
{
	bool focussed = is_focussed();

	int x, y;

	if (use_relative_position) {
		x = relative_x;
		y = relative_y;
	}
	else {
		x = calculated_x;
		y = calculated_y;
	}

	int tw = shim::font->get_text_width(text);
	int th = shim::font->get_height();

	int w = calculated_w;
	int h = calculated_h;

	Uint32 t = SDL_GetTicks();
	float p = (t % 1000) / 1000.0f;
	if (p <= 0.5f) {
		p /= 0.5f;
	}
	else {
		p -= 0.5f;
		p /= 0.5f;
		p = 1.0f - p;
	}

	SDL_Colour c;
	if (focussed && gui == shim::guis.back()->gui) {
		c.r = 0;
		c.g = 216 * p;
		c.b = 255 * p;
		c.a = 255 * p;
	}
	else {
		c.r = 0;
		c.g = 0;
		c.b = 0;
		c.a = 0;
	}

	gfx::draw_filled_rectangle(c, util::Point<float>(x, y), util::Size<int>(w, h));
	shim::font->draw(YELLOW, text, util::Point<float>(x, y) + util::Point<int>(w/2-tw/2+(10+SQUARE_W)/2, h/2-th/2));

	gfx::draw_filled_rectangle(YELLOW, util::Point<float>(x+SQUARE_W/4+w/2-tw/2-(10+SQUARE_W)/2, y+SQUARE_W/4), util::Size<int>(SQUARE_W/2, SQUARE_W/2));
	if (checked) {
		gfx::draw_filled_rectangle(shim::black, util::Point<float>(x+SQUARE_W/4+SQUARE_W/6+w/2-tw/2-(10+SQUARE_W)/2, y+SQUARE_W/4+SQUARE_W/6), util::Size<int>(SQUARE_W/2/3, SQUARE_W/2/3));
	}
}

void Widget_Checkbox::set_size()
{
	w = force_w > 0 ? force_w : shim::font->get_text_width(text) + 10 + SQUARE_W;
	h = SQUARE_W;
}

void Widget_Checkbox::set_enabled(bool enabled)
{
	this->enabled = enabled;
	if (enabled == true) {
		accepts_focus = true;
	}
	else {
		accepts_focus = false;
	}
}

bool Widget_Checkbox::is_enabled()
{
	return enabled;
}

void Widget_Checkbox::set_text(std::string text)
{
	this->text = text;
	set_size();
}

void Widget_Checkbox::handle_event(TGUI_Event *event)
{
	if (is_focussed() == false) {
		return;
	}

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == key_action) || (event->type == TGUI_JOY_DOWN && event->joystick.button == joy_action)) {
		button_mml->play(false);
		checked = !checked;
	}
	else if (event->type == TGUI_MOUSE_DOWN && event->mouse.x >= calculated_x && event->mouse.y >= calculated_y && event->mouse.x < calculated_x+w && event->mouse.y < calculated_y+h) {
		button_mml->play(false);
		checked = !checked;
	}
}

bool Widget_Checkbox::is_checked()
{
	return checked;
}

void Widget_Checkbox::set_checked(bool checked)
{
	this->checked = checked;
}
