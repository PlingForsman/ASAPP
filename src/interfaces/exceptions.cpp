#include "asa/interfaces/exceptions.h"
#include <format>
#include "asa/interfaces/interface.h"

namespace asa
{
    interface_error::interface_error(const interface* t_interface, std::string t_info)
        : asapp_error(std::move(t_info)), _interface(t_interface) {};

    interface_error::interface_error(const interface* t_interface)
        : interface_error(t_interface, "Unknown interface_error!") {};

    const char* interface_error::what() const noexcept { return this->info.c_str(); }

    failed_to_open::failed_to_open(const interface* t_interface) : interface_error(
        t_interface, std::format("Failed to open interface")) {};

    failed_to_close::failed_to_close(const interface* t_interface) : interface_error(
        t_interface, std::format("Failed to close interface")) {};

    receiving_remote_inventory_timeout::receiving_remote_inventory_timeout(
        const interface* t_interface) : interface_error(
        t_interface, std::format("Timed out receing remote inventory")) {};

    no_interface_open::no_interface_open(std::string t_attempted_action,
                                         const interface* t_interface)
        : interface_error(t_interface, std::format(
                              "Attempted to execute '{}' on a closed interface",
                              std::move(t_attempted_action))) {}
}
