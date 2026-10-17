#include "concurrency.h"

// thread-safe mutex_t initialization
int mutex_init(mutex_t *mu) {
  if (!mu) {
    return 0;
  }

  int_least8_t expected = MUTEX_UNINIT;
  if (atomic_compare_exchange_strong_explicit(&mu->state, &expected, MUTEX_INIT_IN_PROGRESS,
                                              memory_order_seq_cst, memory_order_seq_cst)) {
    atomic_store(&mu->lock, UNLOCKED);
    mu->owner = UNASSIGNED_OWNER;
    atomic_store(&mu->state, MUTEX_READY);
    return 1;
  }

  // A thread is initializing/initialized the mutex_t
  // yield until he completes his work and return
  // sucess that the mutex_t is init;
  while (atomic_load(&mu->state) != MUTEX_READY) {
    sched_yield();
  }

  return 1;
}

int mutex_lock(mutex_t *mu) {
  if (!mu) {
    return 0;
  }

  int_least8_t expected = UNLOCKED;

  while (!atomic_compare_exchange_weak_explicit(&mu->lock, &expected, LOCKED, memory_order_seq_cst,
                                                memory_order_seq_cst)) {
    expected = UNLOCKED;
    sched_yield(); // instead of busy wait
  }

  // we have the lock
  mu->owner = pthread_self();
  return 1;
}

int mutex_unlock(mutex_t *mu) {
  if (unlikely(mu->owner != pthread_self())) {
    return 0; // can't unlock the mutex_t cuz you don't own it. you gotta earn it first!
  }

  int_least8_t expected = LOCKED;
  if (!atomic_compare_exchange_strong_explicit(&mu->lock, &expected, UNLOCKED, memory_order_seq_cst,
                                               memory_order_seq_cst)) {
    // the mutex_t wasn't locked
    return 0;
  }
  return 1;
}

int mutex_destroy(mutex_t *mu) {
  if (!mu) {
    return 0;
  }

  if (atomic_load(&mu->state) != MUTEX_READY) {
    return 0;
  }

  if (atomic_load(&mu->lock) != UNLOCKED) {
    return 0; // err mutex_t is still locked
  }

  atomic_store(&mu->state, MUTEX_UNINIT);
  mu->owner = UNASSIGNED_OWNER;

  return 1;
}
