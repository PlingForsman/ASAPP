#include <iostream>
#include "asa/interfaces/teleportmap.h"
#include "../../include/asa/utility.h"
#include "asa/core/state.h"
#include "asa/game/resources.h"
#include "asa/interfaces/exceptions.h"

namespace asa::interfaces
{
    bool TeleportMap::is_open() const
    {
        return window::match_template(window::Rect(239, 126, 176, 51), resources::text::teleports);
    }

    void TeleportMap::close()
    {
        const auto start = std::chrono::system_clock::now();
        while (is_open()) {
            window::press("esc", true);
            if (util::await([this]() { return !is_open(); }, std::chrono::seconds(5))) {
                return;
            }

            // Increased timeout to 60 seconds
            if (util::timedout(start, std::chrono::seconds(60))) {
                throw failed_to_close(this);
            }
        }
    }

    void TeleportMap::set_selected_as_default()
    {
        if (!util::await([this]() -> bool { return can_confirm_travel(); },
                         std::chrono::seconds(10))) {
            throw std::exception("Travel confirmation button did not become available.");
        }

        set_default_button.press();
    }

    void TeleportMap::go_to(const std::string& destination, bool wait_ready)
    {
        searchbar.search_for(destination);
        core::sleep_for(std::chrono::milliseconds(400));

        destination_button button = get_ready_destination(destination, false);
        button.select();

        if (!util::await([this]() -> bool { return can_confirm_travel(); },
                         std::chrono::seconds(10))) {
            throw std::exception("Tp confirmation button did not become available.");
        }

        do { confirm_button.press(); } while (!util::await(
            [this]() -> bool { return !is_open(); },
            std::chrono::seconds(5)));
        searchbar.set_text_cleared();
    }

    std::vector<base_travel_map::destination_button> TeleportMap::get_destinations() const
    {
        std::vector<destination_button> ret;

        for (const auto& roi: destination_slots_) {
            // create an imaginary button for now.
            const destination_button button(roi.x, roi.y);
            // button doesnt exist, end of the list reached.
            if (!button.is_ready()) { break; }
            ret.push_back(button);
        }
        return ret;
    }
}
