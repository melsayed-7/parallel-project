#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

// function that returns the largest 2 counts and their indecies
int find_max2(int* array, int n, int *largest1, int *largest2, int* idx1, int *idx2)
{
    int temp;
    int tempIdx;
    *largest1 = array[0];
    *largest2 = array[1];
    *idx1 = 0;
    *idx2 = 1;

    if (*largest1 < *largest2)
    {
        temp = *largest1;
        *largest1 = *largest2;
        *largest2 = temp;
        tempIdx = *idx1;
        *idx1 = *idx2;
        *idx2 = tempIdx;
    }
    for (int i = 2; i < n; i++)
    {
        if (array[i] >= *largest1)
        {
            *largest2 = *largest1;
            *largest1 = array[i];
            *idx2 = *idx1;
            *idx1 = i;
        }
        else if (array[i] > *largest2 && array[i] != *largest1)
        {
            *largest2 = array[i];
            *idx2 = i;
        }
    }
}



int main(int argc, char *argv[])
{
    // read file
    static const char filename[] = "text.txt";
    FILE *file = fopen(filename, "r");
    int num_candidates, num_voters;
    int numtasks, rank;
    int *results = malloc(num_candidates * sizeof(int));

    // read number of candidates and number of votes from the files
    fscanf(file, "%d", &num_candidates);
    fscanf(file, "%d", &num_voters);
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);


    // compute num_elements for each proces
    int num_ele = num_voters / numtasks;
    int begin  = rank * (num_ele) * num_candidates;
    int end = (rank+1) * num_ele * num_candidates;
    int remainder = num_voters % numtasks;

    // deal with votes number that are not divisble by number of processes
    if (rank < remainder) {
        begin = rank * (num_ele + 1);
        end = (rank+1) * (num_ele+1) - 1;
    }
    else
    {
        begin = rank * num_ele + remainder;
        end = begin + (num_ele - 1);
    }
    begin = begin * num_candidates;
    end = end * (num_candidates) + num_candidates;

    // initialize a counter to count if a candidate was the preferred for each voter
    int count = 0;
    int *counter = malloc(num_candidates * sizeof(int));
    for(int k=0; k<num_candidates; k++){
        counter[k] = 0;
    }

    // read numbers and check the preferred candidate and increase the counter of that candidate by one
    int x;
    for(int i = 0; i < num_candidates*num_voters; i++){
            if(count >= begin && count < end && count%num_candidates==0) { /* Read numbers one-by-one */
                fscanf(file, "%d", &x);
                counter[x-1] = counter[x-1] + 1;
                count++;
            }
            else{
                fscanf(file, "%d", &x);
                count++;
            }
    }


    // variables to get first count, second count, index of most preferred, index of second preferred
    int first;
    int second;
    int idx1, idx2;

    // All_reduce to aggreate counter
    MPI_Allreduce(counter, results, num_candidates, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if(rank == 0){
        for(int i=0; i<num_candidates; i++){
            printf("[%d]: %d",i+1, results[i]);
            printf("\n");
        }
        find_max2(results, num_candidates, &first, &second, &idx1, &idx2);
        printf("Percentage of the most preferred candidate in round 1: %f %%\n", (double)first*100/(double)num_voters);
    }

    MPI_Bcast(&first, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&second, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&idx1, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&idx2, 1, MPI_INT, 0, MPI_COMM_WORLD);


    // variables to get winner in round 2
    int c = 0;
    int flag = 0;
    int idx_1 = 0;
    int idx_2 = 0;

    if((double)first/(double)num_voters < 0.5){
        file = fopen(filename, "r");
        fscanf(file, "%d", &num_candidates);
        fscanf(file, "%d", &num_voters);

        count = 0;

        for(int k=0; k<num_candidates; k++){
            counter[k] = 0;
        }

        // execlude non-2-largest candidates and count the prefered candidate
        for(int i = 0; i < num_candidates*num_voters; i++){
            fscanf(file, "%d", &x);
            if(count >= begin && count < end) {
                if(count%num_candidates == 0){
                    flag = 0;
                }
                if(flag==0){
                    if(x == idx1+1){
                        flag = 1;
                        counter[x-1] += 1;
                    }
                    else if(x==idx2+1){
                        flag = 1;
                        counter[x-1] += 1;
                    }
                }
            }
            count++;
        }

        // reduce all counts to results again and print the winner
        MPI_Allreduce(counter, results, num_candidates, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        if(rank == 0){
            printf("\nBegin round 2\n");
            for(int i=0; i<num_candidates; i++){
                printf("[%d]: %d", i+1, results[i]);
                printf("\n");
            }
            find_max2(results, num_candidates, &first, &second, &idx1, &idx2);
            printf("Candidate %d wins\n", idx1+1);
        }
    }
    else{
        if(rank==0)
            printf("Candidate %d wins\n", idx1+1);
    }
    MPI_Finalize();
}
