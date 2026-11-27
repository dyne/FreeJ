#include <frei0r/bundle.h>
#include <stdlib.h>

int freej_fixture_kind = F0R_PLUGIN_TYPE_FILTER;
#define KIND freej_fixture_kind
#define MODEL F0R_COLOR_MODEL_RGBA8888
#define ID "bound"
#define TITLE "Embedded clock"
#include "frei0r-fixture.c"

/* Only linked into the static host test. Shipping registry IDs come from frei0r. */
const f0r_plugin_descriptor_t freej_fixture_descriptor = {
  F0R_PLUGIN_DESCRIPTOR_SIZE, F0R_PLUGIN_DESCRIPTOR_VERSION, "test-clock",
  f0r_init, f0r_deinit, f0r_get_plugin_info, f0r_get_param_info, f0r_construct,
  f0r_destruct, f0r_set_param_value, f0r_get_param_value, f0r_update, f0r_update2
};
