#include "asa/interfaces/console.h"

#include <iostream>
#include <../../include/asa/util.h>


namespace asa::interfaces
{
    bool console::is_open() const
    {
        static constexpr window::Color gray{140,140,140};

        return cv::countNonZero(window::get_mask(bar_lower_, gray, 5)) > 500; 
    }

    void console::open()
    {
        if (is_open()) { return; }

        do { window::press(settings::console); }
        while (!util::await([this]() -> bool { return is_open(); },
                            std::chrono::seconds(5)));
    }

    void console::close()
    {
        if (!is_open()) { return; }

        do { window::press("enter"); }
        while (!util::await([this]() -> bool { return !is_open(); },
                            std::chrono::seconds(5)));
    }

    void console::execute(const std::string& command)
    {
        open();
        util::set_clipboard(command);
        controls::key_combination_press("ctrl", "v");

        // Keep trying to press enter until console closed
        int counter = 0;
        do {
            // Only try to close the console 12 times (aka 1min)
            if(counter > 12) {
                break;
            }
            window::press("enter");
            counter++;
        } while (!util::await([this] { return !is_open(); }, std::chrono::seconds(5)));
    }



}
