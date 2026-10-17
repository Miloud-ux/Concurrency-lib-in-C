#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>

#define unlikely(x) __builtin_expect(!!(x), 0)

// the struct must be 0 initialized (use calloc)
typedef struct mutex_ {
  atomic_int_least8_t lock;
  atomic_int_least8_t state;
  pthread_t owner;
} mutex_t;

#define MUTEX_INITIALIZER_HEAP (calloc(1, sizeof(mutex_t)))
#define MUTEX_INITIALIZER_STACK (mutex_t{0})

typedef enum {
  UNASSIGNED_OWNER = 0,
  LOCKED,
  UNLOCKED,
} mu_locking_state_t;

typedef enum { MUTEX_UNINIT = 0, MUTEX_INIT_IN_PROGRESS, MUTEX_READY } mu_state_t;

int mutex_init(mutex_t *mu);
int mutex_lock(mutex_t *mu);
int mutex_unlock(mutex_t *mu);
int mutex_destroy(mutex_t *mu);
