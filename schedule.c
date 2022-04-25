#include "schedule.h"

schedule periodic_task_schedule(list *stream)
{
    float utilization = 0.;
    unsigned int hyperperiod = 0;
    list p_list;
    list_init(&p_list);
    /*
     * get the node with lowest utilization
     */
    task *node = get_min(stream);
    while(node) {
        /*
         * if the total utilization is lower than 1,
         * check if the task can be add into the list.
         * else, end the scheduling
         */
        if(utilization + node->utilization <= 1.) {
            if(check_periodic_schedule(&p_list, &hyperperiod, node)) {
                en_list(&p_list.head, node, deadline);
                p_list.count++;
                utilization += node->utilization;
            } else {
                free(node->job);
                free(node);
            }
        } else {
            free(node->job);
            free(node);
            break;
        }
        node = get_min(stream);
    }
    schedule plan;
    schedule_init(&plan);
    plan.period = hyperperiod;
    plan.periodic_task.head = p_list.head;
    plan.periodic_task.count = p_list.count;
    expand_schedule(&plan, hyperperiod);
    return plan;
}

void schedule_init(schedule *plan)
{
    plan->hyperperiod = malloc(sizeof(period));
    plan->hyperperiod->using_time = plan->hyperperiod->total_time = 0;
    INIT_LIST_HEAD(&plan->hyperperiod->job_list);
    plan->count = 0;
}

void free_schedule(schedule *a) {
    if(!a)
        return;
    free_list(&a->periodic_task);
    free_list(&a->aperiodic_task);
    free_list(&a->sporadic_task);
    event *node, *safe;
    for(int i=0;i<a->count; i++)
        list_for_each_entry_safe(node, safe, &a->hyperperiod[i].job_list, list)
            free(node);
    free(a->hyperperiod);
}

int check_periodic_schedule(list *p_list, unsigned int *hyperperiod, task *node)
{
    unsigned int lhyperperiod = *hyperperiod;
    lhyperperiod = cal_hyperperiod(lhyperperiod, node->period);
    if(lhyperperiod < *hyperperiod)
        return 0;
    task *temp = p_list->head;
    status *head = NULL;
    /*
     * change the task in the list to the status list
     * use for check if the tasks in the list can be sheduled in a hyperperiod
     */
    for(int i=0; i < p_list->count; i++, temp = temp->next) {
        status *n_node = malloc(sizeof(*n_node));
        n_node->release_time = temp->phase;
        n_node->deadline = temp->deadline + temp->phase;
        n_node->remain_time = temp->exe_time;
        n_node->info = temp;
        n_node->next = NULL;
        en_status_list(&head, n_node, deadline);
    }
    status *n_node = malloc(sizeof(*n_node));
    n_node->release_time = node->phase;
    n_node->deadline = node->deadline + node->phase;
    n_node->remain_time = node->exe_time;
    n_node->info = node;
    n_node->next = NULL;
    en_status_list(&head, n_node, deadline);

    int time = 0;
    while(head) {
        /*
         * get the task status with the lowest deadline
         */
        status *now;
        remove_head(head, now);
        /*
         * check if the task is over the deadline
         */
        if(now->deadline - now->remain_time < time) {
            free(now);
            while(head) {
                status *del = head;
                head = head->next;
                free(del);
            }
            return 0;
        }
        /*
         * if the task with the lowest deadline has not reach its release time,
         * find another task to fill the intervel time
         */
        while(now->release_time > time) {
            status *comp = head;
            while(comp && comp->release_time > time)
                 comp = comp->next;
            if(comp) {
                int spend = update_status(comp, time,
                                          now->release_time - time);
                if(spend < 0) {
                    while(head) {
                        status *del = head;
                        head = head->next;
                        free(del);
                    }
                    return 0;
                }
                remove_node(&head, comp);
                if(comp->deadline > lhyperperiod + comp->info->phase) {
                    free(comp);
                } else {
                    en_status_list(&head, comp, deadline);
                }
                time += spend;
            } else
                time++;
                //time = now->release_time;
        }
        /*
         * the release time is reached
         */
        int spend = update_status(now, time, now->remain_time);

        time += spend;
        if(now->deadline > lhyperperiod + now->info->phase)
            free(now);
        else 
            en_status_list(&head, now, deadline);
    }
    *hyperperiod = lhyperperiod;
    return 1;
}

int update_status(status *a, unsigned int now_time, unsigned int cost)
{
    int spend;
    int check = (int) a->deadline - a->remain_time - now_time;
    if(check < 0)
        return -1;
    if(a->remain_time <= cost) {
        spend = a->remain_time;
        a->remain_time = a->info->exe_time;
        a->release_time += a->info->period;
        a->deadline += a->info->period;
    } else {
        spend = cost;
        a->remain_time -= cost;
    }
    return spend;
}

int update_status_job(status *a, unsigned int now_time, unsigned int cost)
{
    int spend;
    int check = (int) a->deadline - a->remain_time - now_time;
    if(check < 0)
        return -1;
    if(a->remain_time <= cost) {
        spend = a->remain_time;
        int num_of_period = (now_time + a->remain_time - a->info->phase) /
                            a->info->period;
        a->remain_time = a->info->job[num_of_period];
        a->release_time += a->info->period;
        a->deadline += a->info->period;
    } else {
        spend = cost;
        a->remain_time -= cost;
    }
    return spend;
}

unsigned int cal_hyperperiod(unsigned int a, unsigned int b)
{
    if(!a || !b)
        return a | b;
    if(b > a)
        swap(a, b);
    unsigned int hyperperiod = a * b;
    unsigned int gcd = b;
    while(a % gcd) {
        unsigned int temp = a % gcd;
        a = gcd;
        gcd = temp;
    }
    hyperperiod /= gcd;
    return hyperperiod;
}

void expand_schedule(schedule *plan, unsigned int hyperperiod)
{
    int now_period = 0;
    plan->count = stream_time / hyperperiod + !!(stream_time % hyperperiod);
    plan->hyperperiod = malloc(sizeof(period) * plan->count);
    for(int i = 0; i < plan->count;i++){
        plan->hyperperiod[i].using_time = 0;
        if(i != plan->count - 1)
            plan->hyperperiod[i].total_time = hyperperiod;
        else 
            plan->hyperperiod[i].total_time = stream_time % hyperperiod;
        INIT_LIST_HEAD(&plan->hyperperiod[i].job_list);
    }
    task *temp = plan->periodic_task.head;
    status *head = NULL;
    for(int i=0; i < plan->periodic_task.count; i++, temp = temp->next) {
        status *n_node = malloc(sizeof(*n_node));
        n_node->release_time = temp->phase;
        n_node->deadline = temp->deadline + temp->phase;
        n_node->remain_time = temp->exe_time;
        n_node->info = temp;
        n_node->next = NULL;
        en_status_list(&head, n_node, deadline);
    }
    int time = 0;
    while(head) {
        status *now;
        remove_head(head, now);
        while(now->release_time > time) {
            status *comp = head;
            while(comp && comp->release_time > time)
               comp = comp->next;
            if(comp) {
                event *node = malloc(sizeof(*node));
                node->type = PERIODIC;
                node->shift = comp->deadline;

                int spend = update_status_job(comp, time, 
                                              now->release_time - time);

                node->start_time = time;
                node->end_time = time + spend;
                node->id = comp->info->id;
                node->shift = node->shift - time - spend;

                if(time > (now_period + 1) * hyperperiod)
                    now_period++;

                list_add_tail(&node->list,
                              &plan->hyperperiod[now_period].job_list);
                plan->hyperperiod[now_period].using_time += spend;

                remove_node(&head, comp);
                if(comp->deadline > stream_time) {
                    free(comp);
                } else {
                    en_status_list(&head, comp, deadline);
                }
                time += spend;
            } else
                time++;
                //time = now->release_time;
        }
        event *node = malloc(sizeof(*node));
        node->type = PERIODIC;
        node->shift = now->deadline;
        int spend = update_status_job(now, time, now->remain_time);

        node->start_time = time;
        node->end_time = time + spend;
        node->id = now->info->id;
        node->shift = node->shift - time - spend;

        if(time > (now_period + 1) * hyperperiod)
            now_period++;

        list_add_tail(&node->list,
                      &plan->hyperperiod[now_period].job_list);
        plan->hyperperiod[now_period].using_time += spend;

        time += spend;
        if(now->deadline > stream_time)
            free(now);
        else
            en_status_list(&head, now, deadline);
    }
}

void print_schedule(list *p_list, unsigned int hyperperiod)
{
    int now_period = 0;
    printf("Period (0)\n");
    task *temp = p_list->head;
    status *head = NULL;
    for(int i=0; i < p_list->count; i++, temp = temp->next) {
        status *n_node = malloc(sizeof(*n_node));
        n_node->release_time = temp->phase;
        n_node->deadline = temp->deadline + temp->phase;
        n_node->remain_time = temp->exe_time;
        n_node->info = temp;
        n_node->next = NULL;
        en_status_list(&head, n_node, deadline);
    }
    unsigned int shift;
    int time = 0;
    while(head) {
        status *now;
        remove_head(head, now);
        if(time > (now_period + 1) * hyperperiod)
            printf("Period (%d)\n", ++now_period);
        while(now->release_time > time) {
            status *comp = head;
            while(comp && comp->release_time > time)
               comp = comp->next;
            if(comp) {
                shift = comp->deadline;
                printf("ID:%3d, start time: %5d ", comp->info->id,
                                                   time);
                int spend = update_status(comp, time,
                                          now->release_time - time);
                printf("end time: %5d, shift:%3d\n", time + spend,
                                                   shift - time - spend);

                remove_node(&head, comp);
                if(comp->deadline > hyperperiod * 3) {
                    free(comp);
                } else {
                    en_status_list(&head, comp, deadline);
                }
                time += spend;
                if(time > (now_period + 1) * hyperperiod)
                    printf("Period (%d)\n", ++now_period);
            } else
                time++;
                //time = now->release_time;

        }

        if(time > (now_period + 1) * hyperperiod)
            printf("Period (%d)\n", ++now_period);
        shift = now->deadline;
        printf("ID:%3d, start time: %5d ", now->info->id,
                                            time);
        int spend = update_status(now, time, now->remain_time);

        printf("end time: %5d, shift:%3d\n", time + spend,
                                           shift - time - spend);
        time += spend;
        if(now->deadline > hyperperiod * 3)
            free(now);
        else
            en_status_list(&head, now, deadline);
    }
}
