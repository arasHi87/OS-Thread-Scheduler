#ifndef _THREAD_H_
#define _THREAD_H_
#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

typedef struct thread
{
    /* thread base infomation */
    char *name;
    int id;            // thread id
    int cancel_mode;   // 0=asynchronous, 1=defer
    int cancel_mark;   // mark if need cancel
    int wait_id;       // event id which event want to wait for
    long r_qtime;      // queueing time in ready queue
    long w_qtime;      // waitting time in wait queue
    long need_wait;    // time need to wait
    long already_wait; // time already wait

    /* priority, H=2, M=1, L=0*/
    int b_priority; // base prioriity
    int c_priority; // current priority

    /* context about */
    ucontext_t ctx; // context record
    struct thread *next;
} thread;

thread *thread_create(char *name, char *priority, int id, int cancel_mode);
void inq(thread **head, thread **node);
thread *deq(thread **head);
void change_priority(thread **target, int time_past, int time_quantum);

#endif