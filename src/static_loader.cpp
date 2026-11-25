#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" {
void *__real_dlopen(const char *name, int flags);
void *__real_dlmopen(Lmid_t space, const char *name, int flags);

static void check_shared_object(const char *name)
{
  if (name) {
    fprintf(stderr, "Static FreeJ cannot load a shared object: %s\n", name);
    abort();
  }
}

void *__wrap_dlopen(const char *name, int flags)
{
  check_shared_object(name);
  return __real_dlopen(name, flags);
}

void *__wrap_dlmopen(Lmid_t space, const char *name, int flags)
{
  check_shared_object(name);
  return __real_dlmopen(space, name, flags);
}
}
