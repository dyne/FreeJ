/* Instrumented directory module. Test builds supply an ID and plugin type. */
#include <frei0r.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct effect {
  unsigned width, height;
  double enabled, amount;
  f0r_param_color_t color;
  f0r_param_position_t point;
};

static unsigned live, initialized;
static void event(const char *name) {
  FILE *file = fopen(getenv("FREEJ_EFFECT_TRACE"), "a");
  if (!file) abort();
  fprintf(file, "%s %s\n", ID, name);
  fclose(file);
}

int f0r_init(void) {
  if (initialized++) abort();
#ifdef FAIL_INIT
  event("init-failed");
  return 0;
#else
  event("init");
  return 1;
#endif
}

#ifndef MISSING_DEINIT
void f0r_deinit(void) {
  if (live || initialized != 1) abort();
  initialized = 0;
  event("deinit");
}
#endif

void f0r_get_plugin_info(f0r_plugin_info_t *info) {
  if (!initialized) abort();
  *info = (f0r_plugin_info_t){TITLE, "FreeJ tests", KIND, MODEL, 1, 1, 0, 5, ID};
}

void f0r_get_param_info(f0r_param_info_t *param, int i) {
  const char *names[] = {"toggle", "amount", "color", "position", "text"};
  if (i < 0 || i > 4) abort();
  *param = (f0r_param_info_t){names[i], i, names[i]};
}

f0r_instance_t f0r_construct(unsigned w, unsigned h) {
  if (!initialized) abort();
  if (w == 88) { event("construct-failed"); return NULL; }
  struct effect *fx = calloc(1, sizeof(*fx));
  if (!fx) return NULL;
  fx->width = w;
  fx->height = h;
  fx->enabled = 0.75;
  live++;
  event("construct");
  return fx;
}

void f0r_destruct(f0r_instance_t instance) {
  if (!instance || !live--) abort();
  free(instance);
  event("destruct");
}

void f0r_set_param_value(f0r_instance_t inst, f0r_param_t value, int i) {
  struct effect *fx = inst;
  if (getenv("FREEJ_EFFECT_TRACE_SETS")) {
    if (i == 0) event(*(double *)value == 1.0 ? "set-true" : "set-false");
    if (i == 1) event(*(double *)value == 0.25 ? "quarter" : "other-amount");
    if (i == 3) {
      f0r_param_position_t *point = value;
      event(point->x == 0.25 && point->y == 0.75 ? "position" : "other-position");
    }
  }
  switch (i) {
    case 0: fx->enabled = *(f0r_param_bool *)value; break;
    case 1: fx->amount = *(f0r_param_double *)value; break;
    case 2: fx->color = *(f0r_param_color_t *)value; break;
    case 3: fx->point = *(f0r_param_position_t *)value; break;
    default: abort();
  }
}

void f0r_get_param_value(f0r_instance_t inst, f0r_param_t value, int i) {
  struct effect *fx = inst;
  switch (i) {
    case 0: *(f0r_param_bool *)value = fx->enabled; break;
    case 1: *(f0r_param_double *)value = fx->amount; break;
    case 2: *(f0r_param_color_t *)value = fx->color; break;
    case 3: *(f0r_param_position_t *)value = fx->point; break;
    default: abort();
  }
}

void f0r_update(f0r_instance_t inst, double time, const uint32_t *input, uint32_t *output) {
  struct effect *fx = inst;
  if ((uintptr_t)input % 16 || (uintptr_t)output % 16 ||
      (!!input != (KIND == F0R_PLUGIN_TYPE_FILTER))) abort();
  for (unsigned i = 0; i < fx->width * fx->height; ++i)
    output[i] = input ? input[i] : UINT32_C(0xef563412);
  if (input) ((unsigned char *)(output + 1))[0] ^= 0xff;
  output[0] = (uint32_t)(100 * time);
  memcpy(output + 2, &time, sizeof(time));
}

void f0r_update2(f0r_instance_t inst, double time, const uint32_t *a,
                 const uint32_t *b, const uint32_t *c, uint32_t *out) { abort(); }

