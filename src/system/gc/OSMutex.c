#include "dolphin.h"
#include "dolphin/os.h"

#include "__os.h"

#define DEQUEUE_HEAD(mutex, queue, link)            \
    do {                                            \
        struct OSMutex * __next = mutex->link.next; \
        if (__next == NULL) {                       \
            (queue)->tail = 0;                      \
        } else {                                    \
            __next->link.prev = 0;                  \
        }                                           \
        (queue)->head = __next;                     \
    } while(0);

void __OSUnlockAllMutex(struct OSThread * thread) {
    struct OSMutex * mutex;

    while(thread->queueMutex.head) {
        mutex = thread->queueMutex.head;
        DEQUEUE_HEAD(mutex, &thread->queueMutex, link);
        ASSERTLINE(0xE5, mutex->thread == thread);
        mutex->count = 0;
        mutex->thread = 0;
        OSWakeupThread(&mutex->queue);
    }
}
