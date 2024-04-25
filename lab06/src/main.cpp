#include <stdio.h>
#include <string.h>
#include <mpi.h>

#include "./01-stats/main.cpp"
#include "./02-matrix/main.cpp"

void task1 () {
    printf("task 1\n");
    FILE* fp = fopen("./input/1.txt", "r");
    if (!fp) {
        printf("Can not open file\n");
        return;
    }
    int m_rows = 0, m_cols = 0;
    fscanf(fp, "%d %d", &m_rows, &m_cols);
    printf("%d rows, %d cols\n", m_rows, m_cols);
}

void task2 () {
    printf("task 2\n");
    printf("pass\n");
}

// split string into array of strings
// params: 
// str: source string pointer
// delimiter: delimiter pointer
// count: result size pointer
// result: result array pointer
void str_split (const char* str, const char* delimiter, int* count, char*** result) {
    count[0] = 0;
    bool word = false;
    int i;
    for (i=0; str[i]!=0; i++) {
        if (strchr(delimiter, str[i])) {
            word = false;
        } 
        else {
            if (!word) count[0]++;
            word = true;
        }
    }
    // printf("count: %d\n", count[0]);
    result[0] = new char*[count[0]];
    int length = i;
    char* str1 = new char[length+1];
    memcpy(str1, str, length+1);
    char* nexttok;
    for (i=0; i<count[0]; i++) {
        result[0][i] = strtok_r(str1, delimiter, &nexttok);
        str1 = NULL;
    }
}

// check if strings are equal
// params: 
// s1: first string
// s2: other string
// returns: true if strings are equal, otherwise false
bool str_equals (char* const s1, char* const s2) {
    return !strcmp(s1, s2);
}

void do_stats (int argc, char** argv) {
    _01_stats_main(argc, argv);
}

void do_matrix (int argc, char** argv) {
    _02_matrix_main(argc, argv);
}

void do_default (int argc, char** argv) {
    printf("default\n");
}


// expects argv to contain 
int main (int argc, char** argv) {
    MPI_Init(NULL, NULL);
    int node_id = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &node_id);
    char** argvs;
    int argcs;
    str_split((node_id < argc-1) ? argv[node_id+1] : "", " ", &argcs, &argvs);
    if (argcs > 0) {
        if (str_equals(argvs[0], "stats")) do_stats(argcs, argvs);
        else if (str_equals(argvs[0], "matrix")) do_matrix(argcs, argvs);
        else do_default(argcs, argvs);
    }
    else {
        do_default(argcs, argvs);
    }
    MPI_Finalize();
}

