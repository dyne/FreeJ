#ifndef FREEJ_SURFACE_LOCK_H
#define FREEJ_SURFACE_LOCK_H

#include <screen.h>

namespace freej_engine {

class SurfaceLock {
public:
  explicit SurfaceLock(ViewPort &target) : target_(target), locked_(target.lock()) {}
  ~SurfaceLock() {
    if (locked_)
      target_.unlock();
  }
  SurfaceLock(const SurfaceLock &) = delete;
  SurfaceLock &operator=(const SurfaceLock &) = delete;
  explicit operator bool() const { return locked_; }

private:
  ViewPort &target_;
  bool locked_;
};

}

#endif
