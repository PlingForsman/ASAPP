#include "asa/ui/maps/basetravelmap.h"
#include "asa/ui/maps/exceptions.h"
#include "asa/core/state.h"
#include "asa/utility.h"

namespace asa
{
    bool base_travel_map::destination_button::is_ready() const
    {
        const cv::Mat mask = utility::mask(area, text_color, 30);
        mask |= utility::mask(area, text_selected_color, 30);
        return cv::countNonZero(mask) > 100;
    }

    bool base_travel_map::destination_button::is_on_cooldown() const
    {
        const cv::Mat mask = utility::mask(area, text_cooldown_color, 30);
        return cv::countNonZero(mask) > 100;
    }

    bool base_travel_map::destination_button::is_selected() const
    {
        const cv::Mat mask = utility::mask(area, selected_color, 30);
        mask |= utility::mask(area, hovered_selected_color, 30);

        return cv::countNonZero(mask) > 300;
    }

    void base_travel_map::destination_button::select()
    {
        do { press(); }
        while (!utility::await([this]() { return is_selected(); }, 3s));
    }

    base_travel_map::base_travel_map()
    {
        for (int i = 0; i < destination_slots_.max_size(); i++) {
            destination_slots_[i] = {89, 195 + (i * 55), 459, 55};
        }
    };

    bool base_travel_map::is_open() const
    {
        return match(embedded::interfaces::day, day_time_);
    }

    bool base_travel_map::can_confirm_travel() const
    {
        static cv::Vec3b ready_color{158, 88, 18};

        const auto mask = utility::mask(confirm_button.area, ready_color, 20);
        return cv::countNonZero(mask) > 50;
    }

    std::vector<base_travel_map::destination_button> base_travel_map::get_destinations() const
    {
        std::vector<destination_button> ret;

        for (const auto& roi : destination_slots_) {
            // create an imaginary button for now.
            const destination_button button(roi.x, roi.y);
            // button doesnt exist, end of the list reached.
            if (!(button.is_ready() || button.is_on_cooldown())) { break; }
            ret.push_back(button);
        }
        return ret;
    }

    base_travel_map::destination_button base_travel_map::get_ready_destination(
        const std::string& name, const bool wait_ready) const
    {
        auto results = get_destinations();
        // Give it 5 seconds for the destinations to pop up in case server is
        // lagging or something.
        const auto start = std::chrono::system_clock::now();
        while (results.empty()) {
            if (utility::timedout(start, std::chrono::seconds(5))) {
                throw destination_not_found(name);
            }
            results = get_destinations();
        }

        const auto it = std::ranges::find_if(
            results, [](const destination_button& b) -> bool { return b.is_ready(); });

        if (it == results.end()) {
            if (wait_ready) {
                checked_sleep(std::chrono::seconds(5));
                return get_ready_destination(name, wait_ready);
            }
            throw destination_not_ready(name);
        }
        return *it;
    }
}
