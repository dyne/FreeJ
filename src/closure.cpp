#include <closure.h>
#include <jutils.h>


Closing::Closing() {
  if(pthread_mutex_init(&job_queue_mutex_, NULL) == -1)
    error("error initializing POSIX thread job queue mutex");
}

Closing::~Closing() {
  do_jobs(); // flush queue
  if(pthread_mutex_destroy(&job_queue_mutex_) == -1)
    error("error destroying POSIX thread job queue mutex");
}

void Closing::add_job(Closure *job) {
  pthread_mutex_lock(&job_queue_mutex_);
  job_queue_.push(job);
  pthread_mutex_unlock(&job_queue_mutex_);
}

Closure *Closing::get_job_() {
  Closure *job = NULL;
  pthread_mutex_lock(&job_queue_mutex_);
  if (!job_queue_.empty()) {
    job = job_queue_.front();
    job_queue_.pop();
  }
  pthread_mutex_unlock(&job_queue_mutex_);
  return job;
}

Closure *Closing::tryget_job_() {
  Closure *job = NULL;
  if (pthread_mutex_trylock(&job_queue_mutex_))
    return NULL; // don't wait we'll get it next time
  if (!job_queue_.empty()) {
    job = job_queue_.front();
    job_queue_.pop();
  }
  pthread_mutex_unlock(&job_queue_mutex_);
  return job;
}

void Closing::do_jobs() {
  Closure *job;
  bool to_delete;
  // TODO(shammash): maybe we'll need a timed condition to exit the loop
  while ((job = tryget_job_()) != NULL) {
    // convention: synchronized jobs are deleted by caller
    to_delete = !job->is_synchronized();
    job->run();
    if (to_delete) delete job;
  }
}



ThreadedClosing::ThreadedClosing() {
  running_ = true;
  pthread_cond_init(&loop_cond_, NULL);
  pthread_mutex_init(&loop_mutex_, NULL);
  pthread_mutex_lock(&loop_mutex_);
  pthread_create(&thread_, &attr_, &ThreadedClosing::jobs_loop_, this);
}

ThreadedClosing::~ThreadedClosing() {
  running_ = false;
  pthread_cond_signal(&loop_cond_);
  pthread_join(thread_, NULL);

  pthread_mutex_unlock(&loop_mutex_);
  pthread_mutex_destroy(&loop_mutex_);
  pthread_cond_destroy(&loop_cond_);
}

void ThreadedClosing::add_job(Closure *job) {
  Closing::add_job(job);
  pthread_cond_signal(&loop_cond_);
}

void ThreadedClosing::do_jobs() {
  Closure *job;
  bool to_delete;
  while ((job = get_job_()) != NULL) {
    // convention: synchronized jobs are deleted by caller
    to_delete = !job->is_synchronized();
    job->run();
    if (to_delete) delete job;
  }
}

void *ThreadedClosing::jobs_loop_(void *arg) {
  ThreadedClosing *me = (ThreadedClosing *)arg;
  while (me->running_) {
    me->do_jobs();
    pthread_cond_wait(&me->loop_cond_, &me->loop_mutex_);
  }
}

