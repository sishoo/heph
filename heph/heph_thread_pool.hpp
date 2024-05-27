#pragma once

#include "utils/heph_memory.hpp"
#include "heph_cpu_info.hpp"

#include <pthread.h>
#include <stdint.h>
#include <stdatomic.h>
#include <signal.h>

#define HEPH_THREAD_POOL_QUEUE_DEFAULT_CAP 25

typedef void *(*Routine)(void *);

typedef struct
{
        Routine fn;
        void *args;
} HephThreadPoolJob;

typedef struct
{       
        pthread_mutex_t submit_mutex, checkout_mutex; // when reallocating, just get both
        uint32_t submit, checkout, capacity, njobs;
        HephThreadPoolJob * jobs;
} HephThreadPoolQueue;

typedef struct
{
        bool kill;
        pthread_t handle;
        HephThreadPoolQueue queue;
} HephThreadPoolThread;

typedef struct
{
        bool accepting_work;
        uint32_t nthreads;
        HephThreadPoolThread *threads;
} HephThreadPool;

static bool heph_thread_pool_queue_checkout(HephThreadPoolQueue *const q, HephThreadPoolJob *const j)
{
        uint32_t i = q->checkout * !(q->checkout == q->capacity);

        if (q->jobs[i].fn == NULL)
        {
                return false;
        }

        j->fn = q->jobs[i].fn;
        j->args = q->jobs[i].args;
        q->jobs[i].fn = NULL;
        q->checkout = i + 1;

        return true;
}

static void *heph_thread_pool_thread_routine(void *args)
{
        HephThreadPoolThread *t = (HephThreadPoolThread *)args;
        while (!t->kill)
        {
                pthread_mutex_lock(&t->queue.checkout_mutex);
                HephThreadPoolJob j = {};
                while (heph_thread_pool_queue_checkout(&t->queue, &j))
                {
                        j.fn(j.args);
                }
                pthread_mutex_lock(&t->queue.checkout_mutex);


        }

        return NULL;
}

static void heph_thread_pool_queue_init(HephThreadPoolQueue *const q)
{
        pthread_mutex_init(&q->submit_mutex, NULL);
        pthread_mutex_init(&q->checkout_mutex, NULL);

        pthread_mutex_lock(&q->submit_mutex);
        pthread_mutex_lock(&q->checkout_mutex);

        memset(q, 0, sizeof(HephThreadPoolQueue));
        q->capacity = HEPH_THREAD_POOL_QUEUE_DEFAULT_CAP;
        HephThreadPoolJob *jobs = (HephThreadPoolJob *)HCALLOC(HEPH_THREAD_POOL_QUEUE_DEFAULT_CAP, sizeof(HephThreadPoolJob));
        q->jobs = jobs;

        pthread_mutex_unlock(&q->submit_mutex);
        pthread_mutex_unlock(&q->checkout_mutex);
}

static void heph_thread_pool_queue_destroy(HephThreadPoolQueue *const q)
{
        HFREE(q->jobs);
}

static void heph_thread_pool_queue_reallocate(HephThreadPoolQueue *const q)
{
        /*
                TODO 
                only one memcpy does not guarrentee the queue is fifo anymore 
        */
        pthread_mutex_lock(&q->checkout_mutex);

        uint32_t new_capacity = q->capacity * 2;
        HephThreadPoolJob *new_memory = (HephThreadPoolJob *)HCALLOC(new_capacity, sizeof(HephThreadPoolJob));

        memcpy(new_memory, q->jobs, sizeof(HephThreadPoolJob) * q->capacity);

        q->submit = q->capacity;
        q->capacity = new_capacity;
        q->checkout = 0;
        HFREE(q->jobs);
        q->jobs = new_memory;

        pthread_mutex_unlock(&q->checkout_mutex);
}

static uint32_t heph_thread_pool_queue_submit(HephThreadPoolQueue *const q, HephThreadPoolJob job)
{
        if (q->njobs == q->capacity)
        {
                heph_thread_pool_queue_reallocate(q);
        }

        uint32_t i = q->submit * !(q->submit == q->capacity);
        q->jobs[i].fn = job.fn;
        q->jobs[i].args = job.args;
        q->submit = i + 1;
        uint32_t temp = q->njobs++;

        return temp;
}

void heph_thread_pool_init(HephThreadPool *const tp)
{
        memset(tp, 0, sizeof(HephThreadPool));

        tp->nthreads = HEPH_CPU_INFO_NTHREADS;

        const uint32_t threads_size = sizeof(pthread_t) * HEPH_CPU_INFO_NTHREADS;
        const uint32_t queues_size = sizeof(HephThreadPoolQueue) * HEPH_CPU_INFO_NTHREADS * 2;

        tp->threads = (HephThreadPoolThread *)HCALLOC(threads_size + queues_size, 1);

        for (uint32_t i = 0; i < HEPH_CPU_INFO_NTHREADS; i++)
        {
                heph_thread_pool_queue_init(&tp->threads[i].queue);

                if (pthread_create(&tp->threads[i].handle, NULL, heph_thread_pool_thread_routine, (void *)&tp->threads[i]) != 0)
                {
                        HEPH_ABORT("Unable to create thread for application thread pool.");
                }
        }

        tp->accepting_work = true;
}

void heph_thread_pool_destroy(HephThreadPool *const tp)
{
        tp->accepting_work = false;

        for (uint32_t i = 0; i < HEPH_CPU_INFO_NTHREADS; i++)
        {
                tp->threads[i].kill = true;
                pthread_join(tp->threads[i].handle, NULL);
        }

        HFREE(tp->threads);
}

bool heph_thread_pool_submit(HephThreadPool *const tp, HephThreadPoolJob job)
{
        if (!tp->accepting_work)
        {
                return false;
        }

        uint32_t min_jobs = UINT32_MAX;
        uint32_t id = 0;
        for (uint32_t i = 0; i < HEPH_CPU_INFO_NTHREADS; i++)
        {
                pthread_mutex_lock(&tp->threads[i].queue.submit_mutex);
                uint32_t njobs = tp->threads[i].queue.njobs;
                if (njobs == 0)
                {
                        id = i;
                        break;
                }
                else if (njobs < min_jobs)
                {
                        min_jobs = njobs;
                        id = i;
                }
                pthread_mutex_unlock(&tp->threads[i].queue.submit_mutex);
        }

        if (!heph_thread_pool_queue_submit(&tp->threads[id].queue, job))
        {
                // wake the thread
        }
        pthread_mutex_unlock(&tp->threads[id].queue.submit_mutex);

        return true;
}
