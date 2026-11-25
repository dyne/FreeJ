/*  FreeJ - Frei0r wrapper
 *
 *  Copyright (C) 2007-2010 Denis Roio <jaromil@dyne.org>
 *  Copyright (C) 2010    Andrea Guzzo   <xant@xant.net>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * "$Id: $"
 *
 */

#include <config.h>

#ifdef WITH_FREI0R

#ifndef FREEJ_STRICT_STATIC
#include <dlfcn.h>
#endif
#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>
#include <vector>

#include <frei0r_freej.h>
#ifdef FREEJ_STRICT_STATIC
#include <frei0r/bundle.h>
#endif
#include <layer.h>
#include <jutils.h>

FACTORY_REGISTER_INSTANTIATOR(Filter, Freior, Frei0rFilter, core);

namespace {
struct Frei0rCallbacks {
  decltype(&::f0r_init) init;
  decltype(&::f0r_deinit) deinit;
  decltype(&::f0r_get_plugin_info) info;
  decltype(&::f0r_get_param_info) parameter;
  decltype(&::f0r_construct) construct;
  decltype(&::f0r_destruct) destruct;
  decltype(&::f0r_set_param_value) set;
  decltype(&::f0r_get_param_value) get;
  decltype(&::f0r_update) update;
  decltype(&::f0r_update2) update2;
};

#ifndef FREEJ_STRICT_STATIC
template<class Function>
Function symbol(void *handle, const char *name) {
  return reinterpret_cast<Function>(dlsym(handle, name));
}

Frei0rCallbacks directory_callbacks(void *handle) {
  return {
    symbol<decltype(&::f0r_init)>(handle, "f0r_init"),
    symbol<decltype(&::f0r_deinit)>(handle, "f0r_deinit"),
    symbol<decltype(&::f0r_get_plugin_info)>(handle, "f0r_get_plugin_info"),
    symbol<decltype(&::f0r_get_param_info)>(handle, "f0r_get_param_info"),
    symbol<decltype(&::f0r_construct)>(handle, "f0r_construct"),
    symbol<decltype(&::f0r_destruct)>(handle, "f0r_destruct"),
    symbol<decltype(&::f0r_set_param_value)>(handle, "f0r_set_param_value"),
    symbol<decltype(&::f0r_get_param_value)>(handle, "f0r_get_param_value"),
    symbol<decltype(&::f0r_update)>(handle, "f0r_update"),
    symbol<decltype(&::f0r_update2)>(handle, "f0r_update2")
  };
}
#endif
}

// dlopen can return one library to multiple contexts or via multiple paths.
// Its global init/deinit pair spans all those registrations.
struct Freior::Module {
  Frei0rCallbacks abi;
  const void *identity;
  unsigned users = 0;
  bool initialized = false;
  f0r_plugin_info_t info{};
  std::vector<f0r_param_info_t> parameters;

  Module(Frei0rCallbacks callbacks, const void *key) : abi(callbacks), identity(key) {}
  ~Module() {
    if (initialized) abi.deinit();
#ifndef FREEJ_STRICT_STATIC
    dlclose(const_cast<void *>(identity));
#endif
  }

  struct Catalog {
    std::mutex mutex;
    std::map<const void *, Module *> entries;
  };

  // CLI scripted quit uses std::exit with live layers. The lookup has process
  // lifetime; only releasing the last prototype may run plugin teardown.
  static Catalog &catalog() {
    static Catalog *const instance = new Catalog;
    return *instance;
  }

  bool start() {
    if (!abi.init || !abi.deinit || !abi.info || !abi.construct || !abi.destruct)
      return false;
    if (!abi.init()) return false;
    initialized = true;
    abi.info(&info);
    if (!info.name || !*info.name || info.frei0r_version != FREI0R_MAJOR_VERSION ||
        info.num_params < 0 || info.plugin_type < F0R_PLUGIN_TYPE_FILTER ||
        info.plugin_type > F0R_PLUGIN_TYPE_MIXER3 ||
        info.color_model < F0R_COLOR_MODEL_BGRA8888 || info.color_model > F0R_COLOR_MODEL_PACKED32)
      return false;
    bool mixer = info.plugin_type == F0R_PLUGIN_TYPE_MIXER2 || info.plugin_type == F0R_PLUGIN_TYPE_MIXER3;
    if ((mixer && !abi.update2) || (!mixer && !abi.update)) return false;
    if (info.plugin_type == F0R_PLUGIN_TYPE_SOURCE && info.color_model == F0R_COLOR_MODEL_PACKED32) {
      warning("frei0r source %s has no defined channel order (PACKED32)", info.name);
      return false;
    }
    if (info.num_params && (!abi.parameter || !abi.set || !abi.get)) return false;
    parameters.resize(info.num_params);
    for (int i = 0; i < info.num_params; ++i) {
      auto &p = parameters[i];
      abi.parameter(&p, i);
      if (!p.name || p.type < F0R_PARAM_BOOL || p.type > F0R_PARAM_STRING) return false;
      if (p.type == F0R_PARAM_STRING)
        warning("frei0r string parameter unsupported: %s / %s", info.name, p.name);
    }
    return true;
  }

  static Module *acquire(Catalog &loaded, const void *key, const Frei0rCallbacks &abi) {
    auto found = loaded.entries.find(key);
    if (found != loaded.entries.end()) {
#ifndef FREEJ_STRICT_STATIC
      dlclose(const_cast<void *>(key)); // balance this open of an already registered DSO
#endif
      ++found->second->users;
      return found->second;
    }
    std::unique_ptr<Module> candidate(new Module(abi, key));
    if (!candidate->start()) return nullptr;
    candidate->users = 1;
    loaded.entries.emplace(key, candidate.get());
    return candidate.release();
  }
#ifndef FREEJ_STRICT_STATIC
  static Module *open(const char *file) {
    Catalog &loaded = catalog();
    std::lock_guard<std::mutex> lock(loaded.mutex);
    void *handle = dlopen(file, RTLD_LAZY | RTLD_LOCAL);
    return handle ? acquire(loaded, handle, directory_callbacks(handle)) : nullptr;
  }
#else
  static Module *open(const f0r_plugin_descriptor_t *d) {
    if (!d->get_param_info || !d->set_param_value || !d->get_param_value) return nullptr;
    Frei0rCallbacks abi{d->init, d->deinit, d->get_plugin_info, d->get_param_info,
                        d->construct, d->destruct, d->set_param_value, d->get_param_value,
                        d->update, d->update2};
    Catalog &loaded = catalog();
    std::lock_guard<std::mutex> lock(loaded.mutex);
    return acquire(loaded, d, abi);
  }
#endif

  void release() {
    Catalog &loaded = catalog();
    std::lock_guard<std::mutex> lock(loaded.mutex);
    if (!--users) {
      loaded.entries.erase(identity);
      delete this;
    }
  }
};

Freior::Freior() : info{}, opened(false), module(nullptr) {
  set_name("Unknown");
}

Freior::~Freior() {
  if (module) module->release();
}

int Freior::open(char *file) {
  if (opened || !file) return 0;
#ifdef FREEJ_STRICT_STATIC
  return 0;
#else
  return finish_open(Module::open(file));
#endif
}

#ifdef FREEJ_STRICT_STATIC
bool Freior::open_descriptor(const f0r_plugin_descriptor_t *descriptor) {
  return !opened && finish_open(Module::open(descriptor));
}
#endif

int Freior::finish_open(Module *candidate) {
  module = candidate;
  if (!module) return 0;
  opened = true;
  info = module->info;
  param_infos = module->parameters;
  f0r_set_param_value = module->abi.set;
  f0r_get_param_value = module->abi.get;
  f0r_construct = module->abi.construct;
  set_name(info.name);
  if (get_debug() > 2) print_info();
  return 1;
}

void Freior::print_info() {
  notice("frei0r %s by %s, type %d, color model %d, %d parameters",
         info.name, author(), info.plugin_type, info.color_model, info.num_params);
  for (const auto &p : module->parameters)
    act("%s (type %d) %s", p.name, p.type, p.explanation ? p.explanation : "");
}

void Freior::init_parameters(Linklist<Parameter> &parameters) {
  for (const auto &p : module->parameters) {
    auto *param = new Parameter(static_cast<Parameter::Type>(p.type));
    param->set_name(p.name);
    snprintf(param->description, sizeof(param->description), "%s", p.explanation ? p.explanation : "");
    if (p.type != F0R_PARAM_STRING) {
      param->filter_set_f = set_parameter;
      param->filter_get_f = get_parameter;
    }
    parameters.append(param);
  }
}

void Freior::get_parameter(FilterInstance *instance, Parameter *param, int index) {
  if (!instance->core) return;
  const auto get = static_cast<Freior *>(instance->proto)->module->abi.get;
  const int i = index - 1;
  switch (param->type) {
  case Parameter::BOOL: {
    f0r_param_bool v{};
    get(instance->core, &v, i);
    *static_cast<bool *>(param->value) = v >= 0.5;
    break;
  }
  case Parameter::NUMBER:
    get(instance->core, param->value, i);
    break;
  case Parameter::COLOR: {
    f0r_param_color_t v{};
    get(instance->core, &v, i);
    auto *color = static_cast<double *>(param->value);
    color[0] = v.r; color[1] = v.g; color[2] = v.b;
    break;
  }
  case Parameter::POSITION: {
    f0r_param_position_t v{};
    get(instance->core, &v, i);
    auto *point = static_cast<double *>(param->value);
    point[0] = v.x; point[1] = v.y;
    break;
  }
  default: break;
  }
}

void Freior::set_parameter(FilterInstance *instance, Parameter *param, int index) {
  if (!instance->core) return;
  const auto set = static_cast<Freior *>(instance->proto)->module->abi.set;
  const int i = index - 1;
  const auto *values = static_cast<const double *>(param->value);
  switch (param->type) {
  case Parameter::BOOL: {
    f0r_param_bool v = *static_cast<bool *>(param->value) ? 1.0 : 0.0;
    set(instance->core, &v, i);
    break;
  }
  case Parameter::NUMBER: {
    f0r_param_double v = values[0];
    set(instance->core, &v, i);
    break;
  }
  case Parameter::COLOR: {
    f0r_param_color_t v{static_cast<float>(values[0]), static_cast<float>(values[1]), static_cast<float>(values[2])};
    set(instance->core, &v, i);
    break;
  }
  case Parameter::POSITION: {
    f0r_param_position_t v{values[0], values[1]};
    set(instance->core, &v, i);
    break;
  }
  default: break;
  }
}

bool Freior::construct(FilterInstance *inst, unsigned width, unsigned height) {
  if (!module || !width || !height || inst->core) return false;
  inst->core = module->abi.construct(width, height);
  if (!inst->core) return false;
  free(inst->frei0r_input_);
  inst->frei0r_input_ = nullptr;
  inst->frame_width_ = width;
  inst->frame_height_ = height;
  return true;
}

bool Freior::apply(Layer *layer, FilterInstance *instance) {
  if (info.plugin_type != F0R_PLUGIN_TYPE_FILTER || !construct(instance, layer->geo.w, layer->geo.h))
    return false;
  if (Filter::apply(layer, instance)) return true;
  destruct(instance);
  return false;
}

void Freior::destruct(FilterInstance *inst) {
  if (inst->core) {
    module->abi.destruct(inst->core);
    inst->core = nullptr;
  }
}

void Freior::update(FilterInstance *inst, double time, uint32_t *in, uint32_t *out) {
  if (!inst->core || !out) return;
  Filter::update(inst, time, in, out);
  if (in && reinterpret_cast<uintptr_t>(in) % 16) {
    const size_t bytes = size_t(inst->frame_width_) * inst->frame_height_ * sizeof(*in);
    if (!inst->frei0r_input_) {
      void *aligned = nullptr;
      if (posix_memalign(&aligned, 16, bytes) != 0) return;
      inst->frei0r_input_ = static_cast<uint32_t *>(aligned);
    }
    memcpy(inst->frei0r_input_, in, bytes);
    in = inst->frei0r_input_;
  }
  module->abi.update(inst->core, time, in, out);
}

const char *Freior::description()
{
    return info.explanation ? info.explanation : "";
}

const char *Freior::author()
{
    return info.author ? info.author : "";
}

int Freior::get_parameter_type(int i)
{
    return module && i >= 0 && i < info.num_params ? module->parameters[i].type : -1;
}

char *Freior::get_parameter_description(int i)
{
    return module && i >= 0 && i < info.num_params ? const_cast<char *>(module->parameters[i].explanation) : nullptr;
}

int Freior::type()
{
    return Filter::FREIOR;
}

#endif // WITH_FREI0R

