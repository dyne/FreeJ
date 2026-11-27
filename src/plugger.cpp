/*  FreeJ
 *  (c) Copyright 2001-2002 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 * "$Id$"
 *
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <sys/types.h>
#include <dirent.h>

#ifdef WITH_FREI0R
#include <frei0r.h>
#endif

#include <plugger.h>
#include <context.h>
#include <jutils.h>
#include <frei0r_freej.h>
#ifdef FREEJ_STRICT_STATIC
#include <frei0r/bundle.h>
#endif
#include <freeframe_freej.h>

#ifdef HAVE_DARWIN
#include <Carbon/Carbon.h>
#endif

#ifdef WITH_COCOA
#include <CVFilter.h>
#endif

struct Plugger::State {
  std::vector<std::string> directories;
  struct Registration {
    std::shared_ptr<Filter> effect;
    bool source;
  };
  std::map<std::string, Registration> effects;
};

Plugger::Plugger() : state(new State) {
#ifndef FREEJ_STRICT_STATIC
  addsearchdir(PACKAGE_LIB_DIR);

  const char *home = getenv("HOME");
  if (home && *home)
    addsearchdir((std::string(home) + "/.freej/plugins").c_str());

  addsearchdir("/usr/lib/FreeFrame");
  addsearchdir("/usr/local/lib/FreeFrame");
#ifdef WITH_FREI0R
  addsearchdir("/usr/lib/frei0r-1");
  addsearchdir("/usr/lib64/frei0r-1");
  addsearchdir("/usr/local/lib/frei0r-1");
  addsearchdir("/opt/local/lib/frei0r-1");
#endif

//   addsearchdir("/usr/lib/freej");
//   addsearchdir("/usr/local/lib/freej");
//   addsearchdir("/opt/video/lib/freej");
#ifdef HAVE_DARWIN
    char temp[256];
    ProcessSerialNumber psn;
    GetProcessForPID(getpid(), &psn);
    FSRef location;
    GetProcessBundleLocation(&psn, &location);
    // 238 == 256 - strlen("/Contents/Plugins") - 1
    FSRefMakePath(&location, (UInt8 *)temp, 238);
    strcat(temp, "/Contents/Plugins");
    addsearchdir(temp);
    if (home && *home) {
      addsearchdir((std::string(home) + "/Library/Application Support/FreeJ").c_str());
      addsearchdir((std::string(home) + "/Library/Application Support/FreeFrame").c_str());
    }
    addsearchdir("/Library/Application Support/FreeJ");
    addsearchdir("/Library/Application Support/FreeFrame");
#endif
#endif
}

Plugger::~Plugger() {

  func("Plugger::~Plugger()");
  for (auto &effect : state->effects) effect.second.effect->rem();
  delete state;

}

#ifndef FREEJ_STRICT_STATIC
static int selector(const struct dirent *dir)
{
  if(strstr(dir->d_name,".so")) return(1);
  else if(strstr(dir->d_name,".frf")) return(1);
#ifdef HAVE_DARWIN
  else if(strstr(dir->d_name,".dylib")) return(1);
#endif
  return(0);
}
#else
static const char *descriptor_id(const f0r_plugin_descriptor_t *descriptor) {
  if (!descriptor || descriptor->descriptor_size < F0R_PLUGIN_DESCRIPTOR_SIZE ||
      descriptor->descriptor_version != F0R_PLUGIN_DESCRIPTOR_VERSION ||
      !descriptor->id || !*descriptor->id)
    return nullptr;
  return descriptor->id;
}

bool Plugger::register_builtin(Context *env, const f0r_plugin_descriptor_t *descriptor) {
  const char *id = descriptor_id(descriptor);
  if (!id || state->effects.count(id)) return false;

  std::shared_ptr<Freior> fr(static_cast<Freior *>(Factory<Filter>::new_instance("Frei0rFilter")));
  if (!fr || !fr->open_descriptor(descriptor)) return false;
  if (fr->info.plugin_type > F0R_PLUGIN_TYPE_SOURCE) {
    func("frei0r mixer %s (type %d) unsupported", fr->name, fr->info.plugin_type);
    return false;
  }
  const bool source = fr->info.plugin_type == F0R_PLUGIN_TYPE_SOURCE;
  state->effects.emplace(id, State::Registration{fr, source});
  fr->registry_owner_ = fr;
  (source ? env->generators : env->filters).append(fr.get());
  return true;
}
#endif

int Plugger::refresh(Context *env) {
#ifdef FREEJ_STRICT_STATIC
  for (size_t i = 0; i < f0r_bundle_plugin_count(); ++i) {
    const auto *descriptor = f0r_bundle_plugin_by_index(i);
    const char *id = descriptor_id(descriptor);
    if (!id || f0r_bundle_plugin_by_id(id) != descriptor) {
      warning("invalid frei0r descriptor at index %zu", i);
      continue;
    }
    if (!state->effects.count(id) && !register_builtin(env, descriptor))
      warning("cannot register built-in frei0r effect %s", id);
  }
  return 0;
#else

#ifdef WITH_COCOA
  CVFilter::listFilters(env->filters);
#endif
  // Keep prior registrations alive while scanning. Surviving instances also
  // retain their prototype if a path disappears or the context is destroyed.
  auto previous = std::move(state->effects);
  for (auto &entry : previous) entry.second.effect->rem();
  for (const auto &dir : state->directories) {
    struct dirent **files = nullptr;
    int count = scandir(dir.c_str(), &files, selector, alphasort);
    if (count < 0) {
      warning("cannot scan plugin directory %s", dir.c_str());
      continue;
    }
    // Keep the original reverse-alphabetical order within each directory.
    while (count--) {
      std::string file = dir + "/" + files[count]->d_name;
      free(files[count]);
      State::Registration reg{};
      auto old = previous.find(file);
      if (old != previous.end()) {
        reg = old->second;
      } else {
#ifdef WITH_FREI0R
        std::shared_ptr<Freior> fr(static_cast<Freior *>(Factory<Filter>::new_instance("Frei0rFilter")));
        if (fr && fr->open(&file[0])) {
          if (fr->info.plugin_type > F0R_PLUGIN_TYPE_SOURCE) {
            func("frei0r mixer %s (type %d) unsupported", fr->name, fr->info.plugin_type);
            continue;
          }
          reg = {fr, fr->info.plugin_type == F0R_PLUGIN_TYPE_SOURCE};
        }
#endif
#ifdef WITH_FREEFRAME
        if (!reg.effect) {
          std::shared_ptr<Freeframe> ff(static_cast<Freeframe *>(Factory<Filter>::new_instance("FreeframeFilter")));
          if (ff && ff->open(&file[0]) && ff->info->pluginType <= FF_SOURCE)
            reg = {ff, ff->info->pluginType == FF_SOURCE};
        }
#endif
      }
      if (reg.effect) {
        reg.effect->registry_owner_ = reg.effect;
        auto &list = reg.source ? env->generators : env->filters;
        list.append(reg.effect.get());
        state->effects.emplace(file, reg);
      }
    }
    free(files);
  }
  act("filters found: %u", env->filters.len());
  act("generators found: %u", env->generators.len());

  return 0;
#endif
}


void Plugger::addsearchdir(const char *dir) {
  if (dircheck(dir)) state->directories.emplace_back(dir);
}
