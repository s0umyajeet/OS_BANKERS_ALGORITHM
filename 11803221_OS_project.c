#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

int num_of_resources;
int num_of_processes;
int *resources;
int **allocated_matrix;
int **max_required;
int *safe_sequence;
int **need;
int random_process_cnt = 0;

pthread_mutex_t lock;
pthread_cond_t condition;

bool get_safe_sequence();
void *process_code_segment(void *);
void calculate_need_matrix(int **, int **, int **);
void free_all_resources();
void run_threads();

int main(int argc, char **argv)
{
        srand(time(NULL));

        printf("\n\n");
        printf("\t\t\t\t\t WELCOME\n");
        printf("\tINFO:\n");
        printf("\t----------------------------------------------------------------------\n");
        printf("\tThe Banker's algorithm, sometimes referred to as the detection algorithm,\n"); 
        printf("\tis a resource allocation and deadlock avoidance algorithm developed by\n");
        printf("\tEdsger Dijkstra that tests for safety by simulating the allocation of \n");
        printf("\tpredetermined maximum possible amounts of all resources, and then makes \n");
        printf("\tan \"s-state\" check to test for possible deadlock conditions for all\n");
        printf("\tother pending activities, before deciding whether allocation should be\n");
        printf("\tallowed to continue.\n\n");
        printf("\tSource: Wikipedia\n");
        printf("\t----------------------------------------------------------------------");
        printf("\n\t================================================\n");
        printf("\t| IMPLEMENTATION OF BANKER'S ALGORITHM IN C LANG |\n");
        printf("\t------------------------------------------------\n");
        printf("\tAuthor: Soumyajeet Mandal\n");
        printf("\t------------------------------------------------\n");
        printf("\n\tEnter the number of processes: ");
        scanf("%d", &num_of_processes);

        printf("\n\tEnter the number of resources: ");
        scanf("%d", &num_of_resources);

        resources = (int *)malloc(num_of_resources * sizeof(*resources));
        printf("\n\tCurrently available resources input in the form R1 R2 ...: ");
        for (int i = 0; i < num_of_resources; i++)
                scanf("%d", &resources[i]);

        allocated_matrix = (int **)malloc(num_of_processes * sizeof(*allocated_matrix));
        for (int i = 0; i < num_of_processes; i++)
                allocated_matrix[i] = (int *)malloc(num_of_resources * sizeof(**allocated_matrix));

        max_required = (int **)malloc(num_of_processes * sizeof(*max_required));
        for (int i = 0; i < num_of_processes; i++)
                max_required[i] = (int *)malloc(num_of_resources * sizeof(**max_required));

        printf("\n");
        for (int i = 0; i < num_of_processes; i++)
        {
                printf("\n\tResource allocated to process %d in the form R1 R2 ...: ", i + 1);
                for (int j = 0; j < num_of_resources; j++)
                        scanf("%d", &allocated_matrix[i][j]);
        }
        printf("\n");

        for (int i = 0; i < num_of_processes; i++)
        {
                printf("\n\tMaximum resource required by process %d in the form R1 R2 ...: ", i + 1);
                for (int j = 0; j < num_of_resources; j++)
                        scanf("%d", &max_required[i][j]);
        }
        printf("\n");


        need = (int **)malloc(num_of_processes * sizeof(*need));
        calculate_need_matrix(need, max_required, allocated_matrix);
        
        
        safe_sequence = (int *)malloc(num_of_processes * sizeof(*safe_sequence));
        for (int i = 0; i < num_of_processes; i++)
                safe_sequence[i] = -1;

        if (!get_safe_sequence())
        {
                printf("\n\tUnsafe State Warning! The processes lead the system to an unsafe state.\n\n");
                exit(-1);
        }

        printf("\n\n\tSafe Sequence Found : ");
        for (int i = 0; i < num_of_processes; i++)
        {
                printf("%-3d", safe_sequence[i] + 1);
        }

        printf("\n\tExecuting Processes...\n\n");
        sleep(1);

        run_threads();
        free_all_resources(); 
}

bool get_safe_sequence()
{
        int  temp_res_store[num_of_resources];
        int  num_of_completed = 0;
        bool Completed[num_of_processes];

        for (int i = 0; i < num_of_resources; i++)
                temp_res_store[i] = resources[i];
        for (int i = 0; i < num_of_processes; i++)
                Completed[i] = false;
        
        while (num_of_completed < num_of_processes)
        {
                bool safe = false;
                for (int i = 0; i < num_of_processes; i++)
                {
                        if (!Completed[i])
                        {
                                bool possible = true;
                                for (int j = 0; j < num_of_resources; j++)
                                        if (need[i][j] > temp_res_store[j])
                                        {
                                                possible = false;
                                                break;
                                        }

                                if (possible)
                                {
                                        for (int j = 0; j < num_of_resources; j++)
                                                temp_res_store[j] += allocated_matrix[i][j];
                                        safe_sequence[num_of_completed] = i;
                                        Completed[i] = true;
                                        ++num_of_completed;
                                        safe = true;
                                }
                        }
                }

                if (!safe)
                {
                        for (int k = 0; k < num_of_processes; k++)
                                safe_sequence[k] = -1;
                        return false; 
                }
        }
        return true; 
}

void *process_code_segment(void *arg)
{
        int p = *((int *)arg);

        pthread_mutex_lock(&lock);

        while (p != safe_sequence[random_process_cnt])
                pthread_cond_wait(&condition, &lock);

        printf("\n***********   Process no. %d   ***********\n", p + 1);
        printf("\n\tAllocated : ");
        for (int i = 0; i < num_of_resources; i++)
                printf("%3d", allocated_matrix[p][i]);

        printf("\n\tNeeded    : ");
        for (int i = 0; i < num_of_resources; i++)
                printf("%3d", need[p][i]);

        printf("\n\tAvailable : ");
        for (int i = 0; i < num_of_resources; i++)
                printf("%3d", resources[i]);

        printf("\n");
        sleep(1);
        printf("\t-----------------------------------\n");
        printf("\tResource allocated successfully!");
        printf("\n");
        sleep(1);
        printf("\tProcess Code Running...");
        printf("\n");
        sleep(rand() % 3 + 2); // process code
        printf("\tProcess Code Completed...");
        printf("\n");
        sleep(1);
        printf("\tProcess now releasing Resource...");
        printf("\n");
        sleep(1);
        printf("\tResource Released successfully!");

        for (int i = 0; i < num_of_resources; i++)
                resources[i] += allocated_matrix[p][i];

        printf("\n\tNow Available : ");
        for (int i = 0; i < num_of_resources; i++)
                printf("%3d", resources[i]);
        printf("\n");
        printf("\t-----------------------------------\n");
        sleep(1);

        random_process_cnt++;
        pthread_cond_broadcast(&condition);
        pthread_mutex_unlock(&lock);
        pthread_exit(NULL);
}

void calculate_need_matrix(int **need, int **max_required, int **allocated_matrix)
{
        for (int i = 0; i < num_of_processes; i++)
                need[i] = (int *)malloc(num_of_resources * sizeof(**need));

        for (int i = 0; i < num_of_processes; i++)
                for (int j = 0; j < num_of_resources; j++)
                        need[i][j] = max_required[i][j] - allocated_matrix[i][j];
}

void run_threads()
{
        pthread_t processes[num_of_processes];
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        int processNumber[num_of_processes];
        for (int i = 0; i < num_of_processes; i++)
                processNumber[i] = i;

        for (int i = 0; i < num_of_processes; i++)
                pthread_create(&processes[i], &attr, process_code_segment, (void *)(&processNumber[i]));

        for (int i = 0; i < num_of_processes; i++)
                pthread_join(processes[i], NULL);

        printf("\nAll Processes Completed\n");

}

void free_all_resources()
{
        free(resources);
        for (int i = 0; i < num_of_processes; i++)
        {
                free(allocated_matrix[i]);
                free(max_required[i]);
                free(need[i]);
        }
        free(allocated_matrix);
        free(max_required);
        free(need);
        free(safe_sequence);
}