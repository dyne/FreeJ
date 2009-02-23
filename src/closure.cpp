#include <closure.h>
#include <jutils.h>


Closing::Closing() {
  if(pthread_mutex_init(&_job_queue_mutex, NULL) == -1)
    error("error initializing POSIX thread job queue mutex");

}

Closing::~Closing() {
  if(pthread_mutex_destroy(&_job_queue_mutex) == -1)
    error("error destroying POSIX thread job queue mutex");
  while (!_job_queue.empty()) {
    delete _job_queue.front();
    _job_queue.pop();
  }
}

void Closing::add_job(Closure *job) {
	pthread_mutex_lock(&_job_queue_mutex);
	_job_queue.push(job);
	pthread_mutex_unlock(&_job_queue_mutex);
}

void Closing::do_jobs() {
	Closure *job;
	bool to_delete;
	if (pthread_mutex_trylock(&_job_queue_mutex))
		return; // don't wait we'll get it next time
	while (!_job_queue.empty()) {
		// TODO(shammash): maybe we have to consider fps here and exit
		// the loop even if queue is not empty
		job = _job_queue.front();
		_job_queue.pop();
		// convention: synchronized jobs are deleted by caller
		to_delete = !job->is_synchronized();
		job->run();
		if (to_delete) delete job;
	}
	pthread_mutex_unlock(&_job_queue_mutex);
}
