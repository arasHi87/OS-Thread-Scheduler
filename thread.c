#include "thread.h"

char priority_map[3] = {'L', 'M', 'H'};

thread *thread_create(char *name, char *priority, int id, int cancel_mode)
{
    int pri;
    if (*priority == 'H')
        pri = 2;
    if (*priority == 'M')
        pri = 1;
    if (*priority == 'L')
        pri = 0;

    thread *tmp = (thread *)malloc(sizeof(thread));

    tmp->name = name;
    tmp->id = id;
    tmp->cancel_mode = cancel_mode;
    tmp->cancel_mark = 0;
    tmp->wait_id = -1;
    tmp->r_qtime = 0;
    tmp->w_qtime = 0;
    tmp->need_wait = 0;
    tmp->already_wait = 0;
    tmp->b_priority = tmp->c_priority = pri;
    tmp->next = NULL;

    return tmp;
}

void inq(thread **head, thread **node)
{
    thread *tmp = (*head), *prev = NULL;
    if (!tmp)
        (*head) = (*node), (*node)->next = NULL;
    else
    {
        while (tmp)
        {
            if (tmp->c_priority >= (*node)->c_priority)
                prev = tmp, tmp = tmp->next;
            else
            {
                (*node)->next = tmp;
                if (tmp != (*head))
                    prev->next = (*node);
                else
                    (*head) = (*node);
                break;
            }
        }
        if (!tmp)
            prev->next = (*node);
    }
    return;
}

thread *deq(thread **head)
{
    if (!(*head))
        return NULL;
    else
    {
        thread *tmp = (*head);
        (*head) = (*head)->next;
        tmp->next = NULL;
        return tmp;
    }
}

void change_priority(thread **target, int time_past, int time_quantum)
{
    if (time_past < time_quantum)
    {
        if ((*target)->c_priority != 2)
        {
            (*target)->c_priority++;
            printf("The priority of %s was changed from %c to %c\n", (*target)->name,
                   priority_map[(*target)->c_priority - 1], priority_map[(*target)->c_priority]);
        }
    }
    else
    {
        if ((*target)->c_priority != 0)
        {
            (*target)->c_priority--;
            printf("The priority of %s was changed fomr %c to %c\n", (*target)->name,
                   priority_map[(*target)->c_priority + 1], priority_map[(*target)->c_priority]);
        }
    }
}