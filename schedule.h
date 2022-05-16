#include "stream.h"
#include "list.h"

#define swap(a, b) \
    do {           \
        b ^= a;    \
        a ^= b;    \
        b ^= a;    \
    } while(0)

typedef enum task_type {
    PERIODIC = 1,
    APERIODIC = 2,
    SPORADIC = 4
} task_type;

/*
 * a node of the detailed timetable
 * store the start time and end time and TASK ID
 * can know this node is belong to which task in the stream
 */
typedef struct event {
    task_type type;
    unsigned int start_time,
                 end_time,
                 id,
                 shift; // knowing if we can delay this part of job
    list_head list;
} event;

/*
 * the detailed timetable of the jobs in the tasks
 * store the total time and how much time we have used before
 */
typedef struct period {
    unsigned int using_time, 
                 total_time;
    list_head job_list;
} period;

/*
 * store the result of the sheduler
 * can know the clear sheduling information
 */
typedef struct schedule {
    unsigned int count, period;
    unsigned int aperiodic_response_time;
    list periodic_task,
         aperiodic_task,
         sporadic_task;
    period *hyperperiod;
} schedule;

/*
 * node of a task situation
 * used for checking if a list of tasks can be sheduling in a hyperperiod time
 */
typedef struct status {
    unsigned int remain_time,
                 release_time,
                 deadline;
    task *info;
    struct status *next;
} status;

/*
 * remove head from the status list
 */
#define remove_head(head, node) \
    do{                         \
        if(head) {              \
            node = head;        \
            head = head->next;  \
        }                       \
    } while(0)

/*
 * remove node from the status list
 */
#define remove_node(head, node)    \
    do {                           \
        status **temp = head;      \
        while(*temp != node)       \
            temp = &(*temp)->next; \
        *temp = (*temp)->next;     \
    } while(0)

/*
 * insert node into the status list by compare member size
 */
#define en_status_list(head, node, member)             \
    do {                                               \
        status **temp = head;                          \
        while(*temp && (*temp)->member < node->member) \
            temp = &(*temp)->next;                     \
        node->next = *temp;                            \
        *temp = node;                                  \
    } while(0)

schedule periodic_task_schedule(list *stream);

void schedule_init(schedule *plan);

void free_schedule(schedule *a);

int check_periodic_schedule(list *p_list, unsigned int *hyperperiod, task *node);

void delay_schedule(schedule *plan);

int aperiodic_task_schedule(schedule *plan);

void expand_schedule(schedule *plan, unsigned int hyperperiod);

void print_schedule(list *p_list, unsigned int hyperperiod);

unsigned int cal_hyperperiod(unsigned int a, unsigned int b);

int update_status(status *a, unsigned int now_time, unsigned int cost);

int update_status_job(status *a, unsigned int now_time, unsigned int cost);
