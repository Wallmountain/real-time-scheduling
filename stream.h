#ifndef STREAM_H
#define STREAM_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define pf_limit 4
#define prime_factor_len 23

static int prime_factor[] = {2, 3, 5, 7, 11, 13, 17, 19, \
                           23, 29, 31, 37, 41, 43, 47, \
                           53, 59, 61, 67, 71, 73, 79, 97};

#define stream_time 10000 // total time unit
#define periodic_job_input "OnlinejobsOfHRT.txt"
#define periodic_task_input "TaskModel.txt"
#define table_number 10


/*
 * periodic task with all jobs' time and some task info
 */
typedef struct task {
    float utilization;
    unsigned int id,
                 phase,
                 period,
                 exe_time,
                 deadline;
    unsigned int *job;
    struct task *next;
} task;

/* 
 * task list for sorting and build a hash table
 */
typedef struct list {
    unsigned int count;
    task *head;
} list;

/*
 * traversal the list 
 * and decide the node position by compare member
 */
#define en_list(head, node, member)                    \
    do {                                               \
        task **temp = head;                            \
        while(*temp && (*temp)->member < node->member) \
            temp = &(*temp)->next;                     \
        node->next = *temp;                            \
        *temp = node;                                  \
    } while(0)

void list_init(list *p);

bool task_selection(task *node);

int build_periodic_task_hashtable(list *success, list *fail);

void free_list(list *a);

void insert_head(task **head, task *node);

task *get_min(list *a);

#endif
