#include "os2021_thread_api.h"

struct itimerval Signaltimer;
ucontext_t dispatch_context;
ucontext_t timer_context;

int thread_id = 0;
long time_past = 0;

/* use to matain running thread and queue*/
thread *running = NULL;
thread *wait_head = NULL;
thread *ready_head = NULL;
thread *terminate_head = NULL;

int OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode)
{
    thread *tmp = thread_create(job_name, priority, thread_id++, cancel_mode);

    if (!strcmp(p_function, "Function1"))
        CreateContext(&(tmp->ctx), &dispatch_context, &Function1);
    else if (!strcmp(p_function, "Function2"))
        CreateContext(&(tmp->ctx), &dispatch_context, &Function2);
    else if (!strcmp(p_function, "Function3"))
        CreateContext(&(tmp->ctx), &dispatch_context, &Function3);
    else if (!strcmp(p_function, "Function4"))
        CreateContext(&(tmp->ctx), &dispatch_context, &Function4);
    else if (!strcmp(p_function, "Function5"))
        CreateContext(&(tmp->ctx), &dispatch_context, &Function5);
    else
    {
        free(tmp);
        return -1;
    }

    inq(&ready_head, &tmp);
    return tmp->id;
}

void OS2021_ThreadCancel(char *job_name)
{
}

void OS2021_ThreadWaitEvent(int event_id)
{
}

void OS2021_ThreadSetEvent(int event_id)
{
}

void OS2021_ThreadWaitTime(int msec)
{
}

void OS2021_DeallocateThreadResource()
{
}

void OS2021_TestCancel()
{
}

void CreateContext(ucontext_t *context, ucontext_t *next_context, void *func)
{
    getcontext(context);
    context->uc_stack.ss_sp = malloc(STACK_SIZE);
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = next_context;
    makecontext(context, (void (*)(void))func, 0);
}

void ResetTimer()
{
    Signaltimer.it_value.tv_sec = 0;
    Signaltimer.it_value.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
    {
        printf("ERROR SETTING TIME SIGALRM!\n");
    }
}

void Dispatcher()
{
    running = deq(&ready_head), time_past = 0;
    ResetTimer();
    setcontext(&(running->ctx));
}

void StartSchedulingSimulation()
{
    /* Set Timer */
    Signaltimer.it_interval.tv_usec = 0;
    Signaltimer.it_interval.tv_sec = 0;

    /* create context and init thread*/
    CreateContext(&dispatch_context, NULL, &Dispatcher);
    OS2021_ThreadCreate("reclaimer", "ResourceReclaim", "L", 1);
    json_parse_and_init_thread();

    /* scheduling */
    setcontext(&dispatch_context);
}
