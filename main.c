#include  "schedule.h"
void print_periodic_info(schedule *plan, list *fail);

int main() {
    list *success, p_fail;
    success = malloc(sizeof(*success) * table_number);
    for(int i=0;i < table_number;i++)
        list_init(&success[i]);
    list_init(&p_fail);
    //task *node;
    build_periodic_task_hashtable(success, &p_fail);
    //while((node = get_min(success)))
    //    printf("%3d %3d %3d %f\n", node->id, node->period, node->deadline, node->utilization);
    schedule plan = periodic_task_schedule(success);
    for(int i=0;i < table_number;i++)
        free_list(&success[i]);
    print_periodic_info(&plan, &p_fail);
    /* 
       add aperiodic and sporadic implement here
       */
    free(success);
    return 0;
}

void print_periodic_info(schedule *plan, list *fail)
{
    printf("(1) ");
    for(task *node = fail->head; node; node = node->next) {
        printf("TASK %d", node->id);
        if(node->next)
            printf(", ");
    }
    printf(".\n(2) %d tasks: ", plan->periodic_task.count);
    for(task *node = plan->periodic_task.head; node; node = node->next) {
        printf("TASK %d", node->id);
        if(node->next)
            printf(", ");
    }
    printf(".\n(3) Hyper-period: %d\n", plan->period);
    int using_time = 0;
    for(int i = 0; i < plan->count; i++)
        using_time += plan->hyperperiod[i].using_time;
    printf("(4) %d%\n", (int)using_time * 100 / stream_time);
    printf("(5)\n");

    print_schedule(&plan->periodic_task, plan->period);
}
