#ifndef __JSYNC_H__
#define __JSYNC_H__

#include <pthread.h>

class JSyncThread {
 private:
  
  void _thread_init();
  void _thread_destroy();
  bool _thread_initialized;

  pthread_t _thread;
  pthread_attr_t _attr;

  pthread_mutex_t _mutex;
  pthread_cond_t _cond;
  
  /* mutex and conditional for the feed */
  pthread_mutex_t _mutex_feed;
  pthread_cond_t _cond_feed;
  
 public:
  
  JSyncThread() { _thread_initialized = false; };
  virtual ~JSyncThread() { if(_thread_initialized) _thread_destroy(); };

  void start();
  virtual void run() {};

  void lock() { pthread_mutex_lock(&_mutex); };
  void unlock() { pthread_mutex_unlock(&_mutex); };

  void lock_feed() { pthread_mutex_lock(&_mutex_feed); };
  void unlock_feed() { pthread_mutex_unlock(&_mutex_feed); };
  
  /* MUTEX MUST BE LOCKED AND UNLOCKED WHILE USING WAIT */
  void wait() { pthread_cond_wait(&_cond,&_mutex); };
  void signal() { pthread_cond_signal(&_cond); };

  void wait_feed() { pthread_cond_wait(&_cond_feed,&_mutex_feed); };
  void signal_feed() { pthread_cond_signal(&_cond_feed); };

 protected:

  static void* kickoff(void *arg) { ((JSyncThread *) arg)->run(); return NULL; };

};

#endif
