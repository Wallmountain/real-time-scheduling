#include "stream.h"

void list_init(list *p)
{
    p->head = NULL;
    p->count = 0;
}

bool task_selection(task *node)
{
    unsigned int period = node->period;
    for(int i = 0;i < pf_limit;i++) {
        while(period % prime_factor[i] == 0)
            period /= prime_factor[i];
    }
    return !(period ^ 1);
}

int read_aperiodic_task(list *success)
{
    FILE *fd = fopen(aperiodic_job_input, "r");
    int n;
    for(int task_num = 0;task_num < periodic_task_num;task_num++) {
        char buffer[10000];
        if(!fgets(buffer, 10000, fd)) {
            fclose(fd);
            return 1;
        }
        task *node = malloc(sizeof(*node));
        node->next = NULL;
        /*
         * tokenize the buffer and assign to the corresponding vaiable in struct
         */
        char *token;
        if(!(token = strtok(buffer, " "))) {
            printf("ERROR : wrong number of parameter in periodic task\n");
            fclose(fd);
            free(node);
            return -1;
        }
        node->job = NULL;
        node->deadline = node->period = node->utilization = 0;
        for (n = 0;n < 3;n++) {
            switch(n) {
                case 0:
                    node->id = atoi(token);
                    break;
                case 1:
                    node->phase = atoi(token);
                    break;
                case 2:
                    node->exe_time = atoi(token);
                    break;
            }
            token = strtok(NULL, " ");
        }
        en_list(&success->head, node, exe_time);
        success->count++;
    }
    return 1;
}

int build_periodic_task_hashtable(list *success, list *fail)
{
    FILE *fd = fopen(periodic_job_input, "r");
    int n;
    for(int task_num = 0;task_num != periodic_task_num;task_num++) {
        char buffer[10000];
        if(!fgets(buffer, 10000, fd)) {
            fclose(fd);
            return 1;
        }
        task *node = malloc(sizeof(*node));
        node->next = NULL;
        int min_exe = 0;
        /*
         * tokenize the buffer and assign to the corresponding vaiable in struct
         */
        char *token;
        if(!(token = strtok(buffer, " "))) {
            printf("ERROR : wrong number of parameter in periodic task\n");
            fclose(fd);
            free(node);
            return -1;
        }
        for (n = 0;n < 6;n++) {
            switch(n) {
                case 0:
                    node->id = atoi(token);
                    break;
                case 1:
                    node->phase = atoi(token);
                    break;
                case 2:
                    node->period = atoi(token);
                    break;
                case 3:
                    min_exe = atoi(token);
                    break;
                case 4:
                    node->exe_time = atoi(token);
                    break;
                case 5:
                    node->deadline = node->period;//atoi(token);
                    break;
            }
            token = strtok(NULL, " ");
        }
        if(node->deadline > node->period)
            node->deadline = node->period;

        node->utilization = (float) node->exe_time / node->period;

        /*
         * detect if the input file information has something wrong
         */
        memset(buffer, '\0', 10000);
        fgets(buffer, 10000, fd);
        if(min_exe > node->exe_time || 
           node->exe_time > node->deadline ||
           !*buffer) {
            insert_head(&fail->head, node);
            fail->count++;
            fgets(buffer, 10000, fd);
            fgets(buffer, 10000, fd);
            continue;
        }
        int job_num = stream_time / node->period;
        node->job = malloc(sizeof(*node->job) * job_num);

        if(!(token = strtok(buffer, " "))) {
            printf("ERROR : wrong number of parameter in periodic task\n");
            fclose(fd);
            free(node);
            return -1;
        }
        /*
         * fill the job information
         */
        for(int i = 0;i < job_num;i++) {
            node->job[i] = atoi(token);
            if(node->job[i] > node->exe_time ||
               node->job[i] < min_exe) {
                free(node->job);
                insert_head(&fail->head, node);
                fail->count++;
                fgets(buffer, 10000, fd);
                fgets(buffer, 10000, fd);
                continue;
            }
            token = strtok(NULL, " ");
        }
        if(!task_selection(node)) {
            free(node->job);
            free(node);
        } else {
            node->next = NULL;
            success[(int)(node->utilization * 10)].count++;
            en_list(&success[(int)(node->utilization * 10)].head, node, utilization);
        }
        fgets(buffer, 10000, fd);
        fgets(buffer, 10000, fd);
    }
    fclose(fd);
    return 1;
}



void free_list(list *a)
{
    if(!a || !a->count)
        return;
    task *temp = a->head;
    while(temp) {
        a->head = temp->next;
        free(temp->job);
        free(temp);
        temp = a->head;
    }
    a->count = 0;
}

void insert_head(task **head, task *node)
{
    node->next = *head;
    *head = node;
}

/*
 * find first element in the hash table
 * if no element, return NULL
 */
task *get_min(list *a)
{
    for(int i=0;i < table_number;i++)
        if(a[i].head) {
            task *temp = a[i].head;
            a[i].head = temp->next;
            temp->next = NULL;
            a[i].count--;
            return temp;
        }
    return NULL;
}
