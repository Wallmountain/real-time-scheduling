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
    //    printf("%3d %3d %f\n", node->id, node->period, node->utilization);
    schedule plan = periodic_task_schedule(success);
    for(int i=0;i < table_number;i++)
        free_list(&success[i]);

    read_sporadic_task(&plan.sporadic_task);
    read_aperiodic_task(&plan.aperiodic_task);
    delay_schedule(&plan);
    sporadic_task_schedule(&plan);
    aperiodic_task_schedule(&plan);
    print_periodic_info(&plan, &p_fail);
    free(success);
    free_schedule(&plan);
    return 0;
}

void print_periodic_info(schedule *plan, list *fail)
{
    printf("(1) %d tasks: ", plan->periodic_task.count);
    for(task *node = plan->periodic_task.head; node; node = node->next) {
        printf("TASK %d", node->id);
        if(node->next)
            printf(", ");
    }
    printf(".\n(2) Hyper-period: %d\n", plan->period);
    int using_time = 0;
    for(int i = 0; i < plan->count; i++)
        using_time += plan->hyperperiod[i].using_time;
    printf("(3) %d%\n", (int)using_time * 100 / stream_time);

    printf("(5) %5.2f\n", (double)plan->aperiodic_response_time / periodic_task_num);

    printf("(6) %d%\n", periodic_task_num - plan->sporadic_task.count);

    printf("(7) %d tasks: ", plan->sporadic_task.count);

    for(task *node = plan->sporadic_task.head; node; node = node->next) {
        printf("STask %d", node->id);
        if(node->next)
            printf(", ");
    }

    printf("\n(4)\n");

    int now_period = 0;
    event *entry;

    for(int i = 0;i < plan->count; i++) {
        printf("Period (%d)\n", i);
        list_for_each_entry(entry, &plan->hyperperiod[i].job_list, list) {
            printf("type:");
            if(entry->type & 0x1)
                printf("P, ");
            else if(entry->type & 0x2) 
                printf("A, ");
            else
                printf("S, ");
            printf("ID:%3d, start time: %5d, end time: %5d, shift:%3d\n", entry->id, entry->start_time, entry->end_time, entry->shift);
        }
    }
}
