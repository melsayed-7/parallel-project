#include <iostream>
#include <time.h>
#include <bits/stdc++.h>
#include <chrono>
#include <omp.h>

using namespace std;

// Shuffle array
void shuffle_array(int arr[], int n)
{
    // Shuffling our array
    shuffle(arr, arr + n, default_random_engine(chrono::steady_clock::now().time_since_epoch().count()));
}

int main(){
    int i, num_candidates, num_voters;
    num_candidates = 7;
    num_voters = 200;

    int *arr = new int[num_candidates];
    #pragma omp parallel
    {
    #pragma omp parallel for ordered schedule(dynamic)
    for(int k=0; k<num_candidates; k++){
        arr[k] = k+1;
    }

    FILE *file;
    file = fopen("text.txt","w");
    if(file == NULL){
        printf("Error!");
        exit(1);
    }

    fprintf(file ,"%d\n" , num_candidates);
    fprintf(file ,"%d\n" , num_voters);
    /* % parallize the suffling and the writing */
    #pragma omp parallel for
    for (int i = 0; i < num_voters; i++){

        /* %suffle one array at a time */
        #pragma omp critical
        shuffle_array(arr, num_candidates);

        for (size_t i = 0; i <num_candidates; i++){
            fprintf(file,"%d\t",arr[i]);
        }

        fprintf(file,"\n" );
    }
    }
    /* fclose(file); */
    printf("File generated\n");
    return 0;
}
