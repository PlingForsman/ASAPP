#include <iostream>
#include "asa/interfaces/components/search_bar.h"
#include "../../../include/asa/utility.h"
#include "asa/core/state.h"
#include "asa/game/controls.h"

namespace asa
{
    namespace
    {
        cv::Vec3b text_color{134, 234, 255};
    }

    bool search_bar::has_text_entered()
    {

        // keep track of a high and low count over the course of 500ms
        // to then evaluate the differences between them.
        const auto start = std::chrono::system_clock::now();
        int lowest = -1;
        bool has_changed = false;
        while (!utility::timedout(start, 800ms)) {
            const int pixcount = utility::count_matches(area, text_color, 30);

            if (pixcount > 100) { return true; }

            if (lowest == -1) {
                lowest = pixcount;
                continue;
            }
            has_changed = abs(lowest - pixcount) > 3;
            lowest = std::min(lowest, pixcount);

            if (has_changed) { break; }
        }
        return lowest > 15;
    }

    bool search_bar::has_blinking_cursor() const
    {
        return utility::count_matches(area, text_color, 30) > 10;
    }

    void search_bar::search_for(const std::string term)
    {
        if (has_text_entered()) { delete_search(); }
        checked_sleep(50ms);

        press();
        checked_sleep(20ms);
        searching = true;

        utility::set_clipboard(term);
        controls::key_combination_press("ctrl", "v");

        if (!utility::await([this] { return has_text_entered(); }, 5s)) {
            std::cerr << "[!] Failed to search, trying again... searching for " << term <<
                    std::endl;
            return search_for(term);
        }

        checked_sleep(50ms);
        window::press("enter");
        checked_sleep(50ms);
        window::post_mouse_press_at({955, 344}, controls::LEFT);

        searching = false;
        last_searched_term = term;
        text_entered = true;
    }

    void search_bar::press() const
    {
        const auto start = std::chrono::system_clock::now();

        do {
            window::post_mouse_press_at(utility::center_of(area), controls::LEFT);
        } while (!utility::await([this] { return has_blinking_cursor(); }, 50ms)
                 && !utility::timedout(start, 10s));
    }

    void search_bar::delete_search()
    {
        press();

        controls::key_combination_press("Ctrl", "a");
        checked_sleep(40ms);
        controls::key_press("Delete");

        checked_sleep(50ms);
        window::press("enter");

        set_text_cleared();
    }
}