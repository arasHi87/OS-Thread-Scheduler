#include "os2021_thread_api.h"

struct itimerval Signaltimer;
ucontext_t dispatch_context;
ucontext_t timer_context;

int thread_id = 0;
long time_past = 0;
int time_quantum[3] = {300, 200, 100};
char _priority_map[3] = {'L', 'M', 'H'};

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
    else if (strcmp(p_function, "ResourceReclaim") == 0)
        CreateContext(&(tmp->ctx), NULL, &ResourceReclaim);
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
    thread *tmp = ready_head, *target = NULL, *prev = NULL;
    if (strcmp(job_name, "reclaimer"))
    {
        /* try to find target job in ready queue or wait queue and cancel it */

        // find in ready queue
        while (tmp != NULL)
        {
            if (!strcmp(job_name, tmp->name))
            {
                target = tmp;
                break;
            }
            else
                prev = tmp, tmp = tmp->next;
        }

        // try to find in wait queue if have no result in ready queue
        if (!target)
        {
            tmp = wait_head, prev = NULL;
            while (tmp)
            {
                if (!strcmp(job_name, tmp->name))
                {
                    target = tmp;
                    break;
                }
                else
                    prev = tmp, tmp = tmp->next;
            }
        }

        // if still not found in both queue, check if is running thred
        if (!target)
        {
            if (!strcmp(job_name, running->name))
            {
                target = running, target->cancel_mark = 1;
                if (target->cancel_mode == 0)
                {
                    printf("%s wants to cancel %s\n", running->name, target->name);
                    return;
                }
                else
                    inq(&terminate_head, &target);
                swapcontext(&(target->ctx), &dispatch_context);
            }
        }

        // if find target want to be terminate, then deal its cancel
        if (target)
        {
            target->cancel_mark = 1;
            if (target->cancel_mode)
                printf("%s wants to cancel %s\n", running->name, target->name);
            else
            {
                // dequeue from queue
                if (target == wait_head)
                    wait_head = wait_head->next;
                else if (target == ready_head)
                    ready_head = ready_head->next;
                else
                    prev->next = target->next;

                // cancel thread immediately
                inq(&terminate_head, &target);
                printf("%s cancel %s", running->name, target->name);
            }
        }
    }
}

void OS2021_ThreadWaitEvent(int event_id)
{
    thread *tmp = running;
    tmp->wait_id = event_id;

    printf("%s wants to wait event %d\n", tmp->name, event_id);
    change_priority(&tmp, time_past, time_quantum[tmp->c_priority]);
    inq(&wait_head, &tmp);
    swapcontext(&(tmp->ctx), &dispatch_context);
}

void OS2021_ThreadSetEvent(int event_id)
{
    thread *tmp = wait_head, *prev = NULL;
    while (tmp)
    {
        if (tmp->wait_id != event_id)
            prev = tmp, tmp = tmp->next;
        else
        {
            tmp->wait_id = -1;
            if (tmp == wait_head)
                wait_head = wait_head->next;
            else
                prev->next = tmp->next, tmp->next = NULL;
            printf("%s changed %s state to ready\n", running->name, tmp->name);
            inq(&ready_head, &tmp);
            return;
        }
    }
    return;
}

void OS2021_ThreadWaitTime(int msec)
{
    thread *tmp = running;

    change_priority(&tmp, time_past, time_quantum[tmp->c_priority]);
    tmp->need_wait = msec;
    tmp->next = NULL;
    inq(&wait_head, &tmp);
    swapcontext(&(tmp->ctx), &dispatch_context);
}

void OS2021_DeallocateThreadResource()
{
    thread *tmp = terminate_head;
    while (tmp)
    {
        printf("The memory space of %s had been release\n", tmp->name);
        terminate_head = terminate_head->next;
        free(tmp);
        tmp = terminate_head;
    }
}

void OS2021_TestCancel()
{
    if (running->cancel_mark)
    {
        inq(&terminate_head, &running);
        setcontext(&dispatch_context);
    }
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
    Signaltimer.it_value.tv_usec = 10000;
    if (setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
    {
        printf("ERROR SETTING TIME SIGALRM!\n");
    }
}

void TimerHandler()
{
    time_past += 10;
    thread *tmp = ready_head, *prev = NULL;

    // add ready queue time
    while (tmp != NULL)
        tmp->r_qtime += 10, tmp = tmp->next;

    // add wait queue time
    tmp = wait_head;
    while (tmp != NULL)
    {
        tmp->w_qtime += 10;
        if (tmp->need_wait != 0)
        {
            tmp->already_wait += 1;
            if (tmp->already_wait >= tmp->need_wait)
            {
                tmp->need_wait = tmp->already_wait = 0;
                if (tmp == wait_head)
                    wait_head = wait_head->next;
                else
                    prev->next = tmp->next;
                inq(&ready_head, &tmp);
            }
        }
        prev = tmp, tmp = tmp->next;
    }

    // chack time quantum
    if (time_past >= time_quantum[running->c_priority])
    {
        if (running->cancel_mark == 1)
            inq(&terminate_head, &running);
        else
        {
            if (running->c_priority != 0)
            {
                running->c_priority -= 1;
                printf("The priority of %s is change from %c to %c\n", running->name,
                       _priority_map[running->c_priority + 1], _priority_map[running->c_priority]);
            }
            inq(&ready_head, &running);
        }
        swapcontext(&(running->ctx), &dispatch_context);
    }

    ResetTimer();
    return;
}

void Dispatcher()
{
    running = deq(&ready_head), time_past = 0;
    ResetTimer();
    setcontext(&(running->ctx));
}

void Report()
{
    thread *tmp = ready_head;
    puts("\n**************************************************************************************************");
    printf("*\tTID\tName\t\tState\t\tB_Priority\tC_Priority\tQ_Time\tW_time\t *\n");
    printf("*\t%d\t%-10s\tRUNNING\t\t%c\t\t%c\t\t%ld\t%ld\t *\n", running->id, running->name,
           _priority_map[running->b_priority], _priority_map[running->c_priority], running->r_qtime, running->w_qtime);

    // print ready queue
    while (tmp)
    {
        printf("*\t%d\t%-10s\tREADY\t\t%c\t\t%c\t\t%ld\t%ld\t *\n", tmp->id, tmp->name, _priority_map[tmp->b_priority],
               _priority_map[tmp->c_priority], tmp->r_qtime, tmp->w_qtime);
        tmp = tmp->next;
    }

    // print wait queue
    while (tmp)
    {
        printf("*\t%d\t%-10s\tWAITTING\t\t%c\t\t%c\t\t%ld\t%ld\t *\n", tmp->id, tmp->name,
               _priority_map[tmp->b_priority], _priority_map[tmp->c_priority], tmp->r_qtime, tmp->w_qtime);
        tmp = tmp->next;
    }
    puts("**************************************************************************************************");
}

void StartSchedulingSimulation()
{
    /* Set Timer */
    Signaltimer.it_interval.tv_usec = 0;
    Signaltimer.it_interval.tv_sec = 0;
    signal(SIGALRM, TimerHandler);
    signal(SIGTSTP, Report);

    /* create context and init thread*/
    CreateContext(&dispatch_context, NULL, &Dispatcher);
    OS2021_ThreadCreate("reclaimer", "ResourceReclaim", "L", 1);
    json_parse_and_init_thread();

    /* scheduling */
    setcontext(&dispatch_context);
}
