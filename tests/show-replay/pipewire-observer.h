// Preload helpers shared by the diagnostics. Publish the one observed input
// handle when FreeJ registers it, without locking on the data thread.
#ifndef FREEJ_PIPEWIRE_OBSERVER_H
#define FREEJ_PIPEWIRE_OBSERVER_H
#include <atomic>
#include <cstdlib>
#include <dlfcn.h>
#include <pipewire/filter.h>

static std::atomic<void *> observed_input{nullptr};

extern "C" void *pw_filter_add_port(pw_filter *filter, pw_direction direction,
    pw_filter_port_flags flags, size_t user_data_size, pw_properties *properties,
    const spa_pod **params, uint32_t n_params)
{
    static const auto add = reinterpret_cast<decltype(&pw_filter_add_port)>(
        dlsym(RTLD_NEXT, "pw_filter_add_port"));
    if (!add) std::abort();
    auto *port = add(filter, direction, flags, user_data_size, properties, params, n_params);
    if (direction == PW_DIRECTION_INPUT)
        observed_input.store(port, std::memory_order_release);
    return port;
}
#endif
