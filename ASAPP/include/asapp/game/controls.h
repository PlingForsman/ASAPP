#pragma once
#include "settings.h"
#include <chrono>
#include <string>

namespace asa::controls
{
	using KeyboardMapping = std::unordered_map<std::string, int>;

	enum MouseButton
	{
		LEFT,
		RIGHT,
		MIDDLE,
		MOUSE4,
		MOUSE5,
	};

	inline const auto str_to_button =
		std::unordered_map<std::string, MouseButton>{
			{ "LeftMouseButton", MouseButton::LEFT },
			{ "RightMouseButton", MouseButton::RIGHT },
			{ "MiddleMouseButton", MouseButton::MIDDLE },
			{ "ThumbMouseButton", MouseButton::MOUSE4 },
			{ "ThumbMouseButton2", MouseButton::MOUSE5 },
		};

	int get_virtual_keycode(std::string key);
	bool is_mouse_input(const settings::ActionMapping& input);
	bool is_key_input(const settings::ActionMapping& input);

	void down(const settings::ActionMapping&,
		std::chrono::milliseconds delay = std::chrono::milliseconds(10));
	void release(const settings::ActionMapping&,
		std::chrono::milliseconds delay = std::chrono::milliseconds(10));
	void press(const settings::ActionMapping&,
		std::chrono::milliseconds delay = std::chrono::milliseconds(150));

	void mouse_down(MouseButton,
		std::chrono::milliseconds delay = std::chrono::milliseconds(10));
	void mouse_up(MouseButton,
		std::chrono::milliseconds delay = std::chrono::milliseconds(10));
	void mouse_press(MouseButton,
		std::chrono::milliseconds delay = std::chrono::milliseconds(150));

	void mouse_combination_press(MouseButton, std::string key);

	void turn_degrees(int x, int y);
	void turn_position(int x, int y);
	void turn_to(int x, int y);

	void key_down(std::string key,
		std::chrono::milliseconds delay = std::chrono::milliseconds(10));
	void key_up(std::string key,
		std::chrono::milliseconds delay = std::chrono::milliseconds(10));
	void key_press(std::string key,
		std::chrono::milliseconds delay = std::chrono::milliseconds(150));
	void key_combination_press(std::string holdKey, std::string pressKey);
}