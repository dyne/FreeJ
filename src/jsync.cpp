#include <jsync.h>
#include <jutils.h>

typedef void* (kickoff)(void*);

void JSyncThread::_thread_init() {
  if(pthread_mutex_init (&_mutex,NULL) == -1)
    error("error initializing POSIX thread mutex");
  if(pthread_cond_init (&_cond, NULL) == -1)
    error("error initializing POSIX thread condtition"); 
  if(pthread_attr_init (&_attr) == -1)
    error("error initializing POSIX thread attribute");

 if(pthread_mutex_init (&_mutex_feed,NULL) == -1)
    error("error initializing POSIX thread feed mutex");
  if(pthread_cond_init (&_cond_feed, NULL) == -1)
    error("error initializing POSIX thread feed condtition"); 

  /* sets the thread as detched
     see: man pthread_attr_init(3) */
  pthread_attr_setdetachstate(&_attr,PTHREAD_CREATE_DETACHED);

  _thread_initialized = true;
}

void JSyncThread::_thread_destroy() {
  if(!_thread_initialized) return;
  if(pthread_mutex_destroy(&_mutex) == -1)
    error("error destroying POSIX thread mutex");
  if(pthread_cond_destroy(&_cond) == -1)
    error("error destroying POSIX thread condition");
  if(pthread_attr_destroy(&_attr) == -1)
    error("error destroying POSIX thread attribute");

  if(pthread_mutex_destroy(&_mutex_feed) == -1)
    error("error destroying POSIX thread feed mutex");
  if(pthread_cond_destroy(&_cond_feed) == -1)
    error("error destroying POSIX thread feed attribute");

}

void JSyncThread::start() {
  if(!_thread_initialized)
    _thread_init();

  pthread_create(&_thread, &_attr, &kickoff, this);
}
