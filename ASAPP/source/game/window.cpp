#include "asapp/game/window.h"
#include "../core/wrappers.h"
#include "../util/util.h"
#include "asapp/core/config.h"
#include "asapp/game/globals.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>


namespace asa::window
{
	namespace
	{
		BITMAPINFOHEADER get_bitmap_info_header(
			int width, int height, int bitCount, int compression)
		{
			BITMAPINFOHEADER bi;
			bi.biSize = sizeof(BITMAPINFOHEADER);
			bi.biWidth = width;
			bi.biHeight = -height;
			bi.biPlanes = 1;
			bi.biBitCount = bitCount;
			bi.biCompression = compression;
			bi.biSizeImage = 0;
			bi.biXPelsPerMeter = 1;
			bi.biYPelsPerMeter = 2;
			bi.biClrUsed = 3;
			bi.biClrImportant = 4;

			return bi;
		}
	}

	bool Init()
	{
		tessEngine = new tesseract::TessBaseAPI();

		if (tessEngine->Init(
				core::config::tessdata_path.string().c_str(), "eng")) {
			std::cerr << "[!] Failed to initialize tesseract!" << std::endl;
			return false;
		}
		std::cout << "[+] Tesseract engine initialized successfully."
				  << std::endl;
	}

	void Color::to_range(int v, cv::Scalar& low, cv::Scalar& high) const
	{
		low = cv::Scalar(
			std::max(0, r - v), std::max(0, g - v), std::max(0, b - v));
		high = cv::Scalar(
			std::min(255, r + v), std::min(255, g + v), std::min(255, b + v));
	}

	const Point Rect::get_random_location(int padding) const
	{
		const int x_min = padding;
		const int x_max = width - padding;

		const int y_min = padding;
		const int y_max = height - padding;

		std::random_device rd;
		std::mt19937 gen(rd());

		std::uniform_int_distribution<int> rand_x_range(x_min, x_max);
		std::uniform_int_distribution<int> rand_y_range(y_min, y_max);

		return Point{ x + rand_x_range(gen), y + rand_y_range(gen) };
	}

	std::optional<Rect> locate_template(const Rect& region,
		const cv::Mat& templ, float threshold, const cv::Mat& mask)
	{
		cv::Mat image = screenshot(region);
		return locate_template(image, templ, threshold, mask);
	}

	std::optional<Rect> locate_template(const cv::Mat& source,
		const cv::Mat& templ, float threshold, const cv::Mat& mask)
	{
		check_state();

		cv::Mat result;
		if (mask.empty()) {
			cv::matchTemplate(source, templ, result, cv::TM_CCOEFF_NORMED);
		}
		else {
			cv::matchTemplate(
				source, templ, result, cv::TM_CCOEFF_NORMED, mask);
		}

		double min_val, max_val;
		cv::Point min_loc, max_loc;
		cv::minMaxLoc(result, &min_val, &max_val, &min_loc, &max_loc);

		if (max_val < threshold) {
			return std::nullopt;
		}

		return Rect(max_loc.x, max_loc.y, templ.cols, templ.rows);
	}

	std::vector<Rect> locate_all_template(const Rect& region,
		const cv::Mat& templ, float threshold, const cv::Mat& mask)
	{
		cv::Mat image = screenshot(region);
		return locate_all_template(image, templ, threshold, mask);
	}

	std::vector<Rect> locate_all_template(const cv::Mat& source,
		const cv::Mat& templ, float threshold, const cv::Mat& mask)
	{
		cv::Mat match_result;
		cv::matchTemplate(
			source, templ, match_result, cv::TM_CCOEFF_NORMED, mask);

		double min_val, max_val;
		cv::Point min_loc, max_loc;
		std::vector<Rect> allMatches;

		while (true) {
			cv::minMaxLoc(match_result, &min_val, &max_val, &min_loc, &max_loc);
			if (max_val < threshold) {
				break;
			}

			Rect loc{ max_loc.x, max_loc.y, templ.cols, templ.rows };
			cv::rectangle(match_result, { loc.x - 5, loc.y - 5, 15, 15 },
				cv::Scalar(0, 0, 0), cv::FILLED);

			allMatches.push_back(loc);
		}
		return allMatches;
	}

	bool match_template(const Rect& region, const cv::Mat& templ,
		float threshold, const cv::Mat& mask)
	{
		return locate_template(region, templ, threshold, mask) != std::nullopt;
	}

	bool match_template(const cv::Mat& source, const cv::Mat& templ,
		float threshold, const cv::Mat& mask)
	{
		return locate_template(source, templ, threshold, mask) != std::nullopt;
	}

	cv::Mat get_mask(const Rect& region, const Color& color, float variance)
	{
		cv::Mat image = screenshot(region);
		return get_mask(image, color, variance);
	}

	cv::Mat get_mask(const cv::Mat& image, const Color& color, float variance)
	{
		cv::Scalar low;
		cv::Scalar high;
		color.to_bgr().to_range(variance, low, high);

		cv::Mat mask;
		cv::inRange(image, low, high, mask);
		return mask;
	}

	void set_tesseract_image(const cv::Mat& image)
	{
		tessEngine->SetImage(image.data, image.size().width,
			image.size().height, image.channels(), image.step1());
	}

	void get_handle(int timeout, bool verbose)
	{
		using seconds = std::chrono::seconds;

		auto start = std::chrono::system_clock::now();
		auto interval_start = start;
		bool first_grab = !hWnd;
		bool info = false;

		do {
			auto now = std::chrono::system_clock::now();
			auto time_passed = std::chrono::duration_cast<seconds>(now - start);
			auto interval_passed = std::chrono::duration_cast<seconds>(
				now - interval_start);

			hWnd = FindWindowA("UnrealWindow", "ArkAscended");

			if (time_passed.count() > timeout && timeout != 0) {
				if (verbose) {
					std::cout << "[!] Setting window handle failed."
							  << std::endl;
				}
				return;
			}

			if (verbose && ((!hWnd && interval_passed.count() > 10) || !info)) {
				std::cout << "[+] Trying to find the window..." << std::endl;
				interval_start = now;
				info = true;
			}
		} while (!hWnd);

		RECT rect;
		GetWindowRect(hWnd, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;

		if (first_grab && verbose) {
			std::cout << std::format(
							 "\t[-] Set window handle! Width: {}, height: {}",
							 width, height)
					  << std::endl;
		}
	}

	cv::Mat screenshot(const Rect& region)
	{
		SetProcessDPIAware();

		RECT rect;
		GetWindowRect(hWnd, &rect);
		int window_width = rect.right - rect.left;
		int window_height = rect.bottom - rect.top;

		HDC hwndDC = GetWindowDC(hWnd);
		HDC mDc = CreateCompatibleDC(hwndDC);
		HBITMAP bitmap = CreateCompatibleBitmap(
			hwndDC, window_width, window_height);

		SelectObject(mDc, bitmap);
		PrintWindow(hWnd, mDc, PW_RENDERFULLCONTENT);

		BITMAPINFOHEADER bi = get_bitmap_info_header(
			window_width, window_height, 32, BI_RGB);

		cv::Mat mat = cv::Mat(window_height, window_width, CV_8UC4);
		GetDIBits(mDc, bitmap, 0, window_height, mat.data,
			reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

		DeleteObject(bitmap);
		DeleteDC(mDc);
		ReleaseDC(hWnd, hwndDC);

		cv::Mat result;
		cv::cvtColor(mat, result, cv::COLOR_RGBA2RGB);

		if (!region.width && !region.height) {
			return result;
		}
		return result(cv::Rect(region.x, region.y, region.width, region.height))
			.clone();
	}

	bool set_foreground()
	{
		if (!hWnd) {
			std::cout << "[!] Cant focus window, get the hWnd first."
					  << std::endl;
			return false;
		}
		return SetForegroundWindow(hWnd) != NULL;
	}

	bool set_foreground_but_hidden()
	{
		return set_foreground() &&
			   SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0,
				   SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	bool has_crashed_popup()
	{
		return (
			FindWindowA(NULL,
				"The UE-ShooterGame Game has crashed and will close") != NULL ||
			FindWindowA(NULL, "Crash!") != NULL);
	}

	void set_mouse_pos(const Point& location)
	{
		SetCursorPos(location.x, location.y);
	}

	void set_mouse_pos(int x, int y) { SetCursorPos(x, y); }

	void click_at(const Point& position, controls::MouseButton button,
		std::chrono::milliseconds delay)
	{

		if (!globals::useWindowInput) {
			set_mouse_pos(position);
			sleep_for(delay);
			controls::mouse_press(button);
		}
		else {
			post_mouse_press_at(position, button);
		}
	}

	void down(
		const settings::ActionMapping& input, std::chrono::milliseconds delay)
	{
		globals::useWindowInput ? post_down(input, delay)
								: controls::down(input, delay);
	}

	void up(
		const settings::ActionMapping& input, std::chrono::milliseconds delay)
	{
		globals::useWindowInput ? post_up(input, delay)
								: controls::release(input, delay);
	}

	void press(const settings::ActionMapping& input, bool catchCursor,
		std::chrono::milliseconds delay)
	{
		globals::useWindowInput ? post_press(input, catchCursor, delay)
								: controls::press(input, delay);
	}

	void down(const std::string& key, std::chrono::milliseconds delay)
	{
		globals::useWindowInput ? post_key_down(key, delay)
								: controls::key_down(key, delay);
	}

	void up(const std::string& key, std::chrono::milliseconds delay)
	{
		globals::useWindowInput ? post_key_up(key, delay)
								: controls::key_up(key, delay);
	}

	void press(const std::string& key, bool catchCursor,
		std::chrono::milliseconds delay)
	{
		globals::useWindowInput ? post_key_press(key, catchCursor, delay)
								: controls::key_press(key, delay);
	}

	void post_down(
		const settings::ActionMapping& input, std::chrono::milliseconds delay)
	{
		controls::is_mouse_input(input)
			? post_mouse_down(controls::str_to_button.at(input.key), delay)
			: post_key_down(input.key, delay);
	}

	void post_up(
		const settings::ActionMapping& input, std::chrono::milliseconds delay)
	{
		controls::is_mouse_input(input)
			? post_mouse_up(controls::str_to_button.at(input.key), delay)
			: post_key_up(input.key, delay);
	}

	void PostPress(const settings::ActionMapping& input, bool catchCursor,
		std::chrono::milliseconds delay)
	{
		if (controls::is_mouse_input(input)) {
			post_mouse_press(
				controls::str_to_button.at(input.key), catchCursor, delay);
		}
		else {
			post_key_press(input.key, catchCursor, delay);
		}
	}

	void post_key_down(const std::string& key, std::chrono::milliseconds delay)
	{
		PostMessageW(
			hWnd, WM_KEYDOWN, controls::get_virtual_keycode(key), NULL);
		sleep_for(delay);
	}

	void post_key_up(const std::string& key, std::chrono::milliseconds delay)
	{
		PostMessageW(hWnd, WM_KEYUP, controls::get_virtual_keycode(key), NULL);
		sleep_for(delay);
	}

	void post_key_press(const std::string& key, bool catchCursor,
		std::chrono::milliseconds delay)
	{
		POINT prevPos;
		if (catchCursor) {
			GetCursorPos(&prevPos);
		}

		post_key_down(key);
		post_key_up(key);

		if (catchCursor) {
			reset_cursor(prevPos);
		}
	}

	void post_char(char c) { PostMessageW(hWnd, WM_CHAR, c, NULL); }

	void post_mouse_down(
		controls::MouseButton button, std::chrono::milliseconds delay)
	{
		using controls::MouseButton;
		switch (button) {
		case MouseButton::LEFT:
			PostMessageW(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, NULL);
			break;
		case MouseButton::RIGHT:
			PostMessageW(hWnd, WM_RBUTTONDOWN, MK_RBUTTON, NULL);
			break;
		case MouseButton::MIDDLE:
			PostMessageW(hWnd, WM_MBUTTONDOWN, MK_MBUTTON, NULL);
			break;
		case MouseButton::MOUSE4:
			PostMessageW(hWnd, WM_XBUTTONDOWN, MK_XBUTTON1, NULL);
			break;
		case MouseButton::MOUSE5:
			PostMessageW(hWnd, WM_XBUTTONDOWN, MK_XBUTTON2, NULL);
			break;
		}
		sleep_for(delay);
	}

	void post_mouse_up(
		controls::MouseButton button, std::chrono::milliseconds delay)
	{
		using controls::MouseButton;
		switch (button) {
		case MouseButton::LEFT:
			PostMessageW(hWnd, WM_LBUTTONUP, MK_LBUTTON, NULL);
			break;
		case MouseButton::RIGHT:
			PostMessageW(hWnd, WM_RBUTTONUP, MK_RBUTTON, NULL);
			break;
		case MouseButton::MIDDLE:
			PostMessageW(hWnd, WM_MBUTTONUP, MK_MBUTTON, NULL);
			break;
		case MouseButton::MOUSE4:
			PostMessageW(hWnd, WM_XBUTTONUP, MK_XBUTTON1, NULL);
			break;
		case MouseButton::MOUSE5:
			PostMessageW(hWnd, WM_XBUTTONUP, MK_XBUTTON2, NULL);
			break;
		}
		sleep_for(delay);
	}

	void post_mouse_press(controls::MouseButton button, bool catchCursor,
		std::chrono::milliseconds delay)
	{
		POINT prevPos;
		if (catchCursor) {
			GetCursorPos(&prevPos);
		}

		post_mouse_down(button, delay);
		post_mouse_up(button);

		if (catchCursor) {
			reset_cursor(prevPos);
		}
	}

	void post_mouse_press_at(
		const Point& position, controls::MouseButton button)
	{
		if (GetForegroundWindow() != hWnd) {
			set_foreground();
			sleep_for(std::chrono::milliseconds(100));
		}

		LPARAM lParam = MAKELPARAM(position.x, position.y);
		if (button == controls::MouseButton::LEFT) {
			PostMessageW(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
		}
		else {
			PostMessageW(hWnd, WM_RBUTTONDOWN, MK_RBUTTON, lParam);
		}

		if (button == controls::MouseButton::LEFT) {
			PostMessageW(hWnd, WM_LBUTTONUP, MK_LBUTTON, lParam);
		}
		else {
			PostMessageW(hWnd, WM_RBUTTONUP, MK_RBUTTON, lParam);
		}
	}

	void reset_cursor(POINT& previousPosition)
	{
		auto start = std::chrono::system_clock::now();
		POINT curr_pos;
		GetCursorPos(&curr_pos);

		while (!util::timedout(start, std::chrono::milliseconds(250))) {

			while (curr_pos.x != width / 2 && curr_pos.y != height / 2) {
				previousPosition = curr_pos;
				GetCursorPos(&curr_pos);
			}
			SetCursorPos(previousPosition.x, previousPosition.y);
		}
	}




}
