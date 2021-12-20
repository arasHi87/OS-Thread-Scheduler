#include "thread.h"

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