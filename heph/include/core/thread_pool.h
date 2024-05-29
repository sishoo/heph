// #pragma once

// #include "./memory.hpp"


// #include <pthread.h>
// #include <stdint.h>
// #include <stdatomic.h>
// #include <signal.h>

// #include <stdbool.h>

// #define HEPH_THREAD_POOL_QUEUE_DEFAULT_CAP 25

// typedef void *(*Routine)(void *);

// typedef struct
// {
//         Routine fn;
//         void *args;
// } heph_thread_pool_job_t;

// typedef struct
// {       
//         pthread_mutex_t submit_mutex, checkout_mutex; // when reallocating, just get both
//         uint32_t submit, checkout, capacity, njobs;
//         heph_thread_pool_job_t * jobs;
// } heph_thread_pool_queue_t;

// typedef struct
// {
//         bool kill;
//         pthread_t handle;
//         heph_thread_pool_queue_t queue;
// } heph_thread_pool_thread_t;

// typedef struct
// {
//         bool accepting_work;
//         uint32_t nthreads;
//         heph_thread_pool_thread_t *threads;
// } heph_thread_pool_t;

// static bool heph_thread_pool_queue_checkout(heph_thread_pool_queue_t *const q, heph_thread_pool_job_t *const j)
// {
//         uint32_t i = q->checkout * !(q->checkout == q->capacity);

//         if (q->jobs[i].fn == NULL)
//         {
//                 return false;
//         }

//         j->fn = q->jobs[i].fn;
//         j->args = q->jobs[i].args;
//         q->jobs[i].fn = NULL;
//         q->checkout = i + 1;

//         return true;
// }

// static void *heph_thread_pool_thread_routine(void *args)
// {
//         heph_thread_pool_thread_t *t = (heph_thread_pool_thread_t *)args;
//         while (!t->kill)
//         {
//                 pthread_mutex_lock(&t->queue.checkout_mutex);
//                 heph_thread_pool_job_t j = {};
//                 while (heph_thread_pool_queue_checkout(&t->queue, &j))
//                 {
//                         j.fn(j.args);
//                 }
//                 pthread_mutex_lock(&t->queue.checkout_mutex);


//         }

//         return NULL;
// }

// static void heph_thread_pool_queue_init(heph_thread_pool_queue_t *const q)
// {
//         pthread_mutex_init(&q->submit_mutex, NULL);
//         pthread_mutex_init(&q->checkout_mutex, NULL);

//         pthread_mutex_lock(&q->submit_mutex);
//         pthread_mutex_lock(&q->checkout_mutex);

//         memset(q, 0, sizeof(heph_thread_pool_queue_t));
//         q->capacity = HEPH_THREAD_POOL_QUEUE_DEFAULT_CAP;
//         heph_thread_pool_job_t *jobs = (heph_thread_pool_job_t *)HCALLOC(HEPH_THREAD_POOL_QUEUE_DEFAULT_CAP, sizeof(heph_thread_pool_job_t));
//         q->jobs = jobs;

//         pthread_mutex_unlock(&q->submit_mutex);
//         pthread_mutex_unlock(&q->checkout_mutex);
// }

// static void heph_thread_pool_queue_destroy(heph_thread_pool_queue_t *const q)
// {
//         HFREE(q->jobs);
// }

// static void heph_thread_pool_queue_reallocate(heph_thread_pool_queue_t *const q)
// {
//         /*
//                 TODO 
//                 only one memcpy does not guarrentee the queue is fifo anymore 
//         */
//         pthread_mutex_lock(&q->checkout_mutex);

//         uint32_t new_capacity = q->capacity * 2;
//         heph_thread_pool_job_t *new_memory = (heph_thread_pool_job_t *)HCALLOC(new_capacity, sizeof(heph_thread_pool_job_t));

//         memcpy(new_memory, q->jobs, sizeof(heph_thread_pool_job_t) * q->capacity);

//         q->submit = q->capacity;
//         q->capacity = new_capacity;
//         q->checkout = 0;
//         HFREE(q->jobs);
//         q->jobs = new_memory;

//         pthread_mutex_unlock(&q->checkout_mutex);
// }

// static uint32_t heph_thread_pool_queue_submit(heph_thread_pool_queue_t *const q, heph_thread_pool_job_t job)
// {
//         if (q->njobs == q->capacity)
//         {
//                 heph_thread_pool_queue_reallocate(q);
//         }

//         uint32_t i = q->submit * !(q->submit == q->capacity);
//         q->jobs[i].fn = job.fn;
//         q->jobs[i].args = job.args;
//         q->submit = i + 1;
//         uint32_t temp = q->njobs++;

//         return temp;
// }

// void heph_thread_pool_init(heph_thread_pool_t *const tp)
// {
//         memset(tp, 0, sizeof(heph_thread_pool_t));

//         tp->nthreads = HEPH_CPU_INFO_NTHREADS;

//         const uint32_t threads_size = sizeof(pthread_t) * HEPH_CPU_INFO_NTHREADS;
//         const uint32_t queues_size = sizeof(heph_thread_pool_queue_t) * HEPH_CPU_INFO_NTHREADS * 2;

//         tp->threads = (heph_thread_pool_thread_t *)HCALLOC(threads_size + queues_size, 1);

//         for (uint32_t i = 0; i < HEPH_CPU_INFO_NTHREADS; i++)
//         {
//                 heph_thread_pool_queue_init(&tp->threads[i].queue);

//                 if (pthread_create(&tp->threads[i].handle, NULL, heph_thread_pool_thread_routine, (void *)&tp->threads[i]) != 0)
//                 {
//                         HEPH_ABORT("Unable to create thread for application thread pool.");
//                 }
//         }

//         tp->accepting_work = true;
// }

// void heph_thread_pool_destroy(heph_thread_pool_t *const tp)
// {
//         tp->accepting_work = false;

//         for (uint32_t i = 0; i < HEPH_CPU_INFO_NTHREADS; i++)
//         {
//                 tp->threads[i].kill = true;
//                 pthread_join(tp->threads[i].handle, NULL);
//         }

//         HFREE(tp->threads);
// }

// bool heph_thread_pool_submit(heph_thread_pool_t *const tp, heph_thread_pool_job_t job)
// {
//         if (!tp->accepting_work)
//         {
//                 return false;
//         }

//         uint32_t min_jobs = UINT32_MAX;
//         uint32_t id = 0;
//         for (uint32_t i = 0; i < HEPH_CPU_INFO_NTHREADS; i++)
//         {
//                 pthread_mutex_lock(&tp->threads[i].queue.submit_mutex);
//                 uint32_t njobs = tp->threads[i].queue.njobs;
//                 if (njobs == 0)
//                 {
//                         id = i;
//                         break;
//                 }
//                 else if (njobs < min_jobs)
//                 {
//                         min_jobs = njobs;
//                         id = i;
//                 }
//                 pthread_mutex_unlock(&tp->threads[i].queue.submit_mutex);
//         }

//         if (!heph_thread_pool_queue_submit(&tp->threads[id].queue, job))
//         {
//                 // wake the thread
//         }
//         pthread_mutex_unlock(&tp->threads[id].queue.submit_mutex);

//         return true;
// }
