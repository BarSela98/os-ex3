#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "ex3_q1_given.h"
#define NUM_THREADS 100
struct all_students all_stud;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int current_index = 0;
int num_of_files=0;

void write_avg_grade(FILE *temp_file, char student_name[10], float avg_grade);
void calculate_grades(char students_grades_file_name[100] ,bool last);
void* thread_function(void* arg);
void* thread_function_last(void* arg);
void print_grades();

int main(int argc, char *argv[])
{
    int total_students = 0;
    char line[100];
    bool first_word = true;
    char student_name[10];
    // int pid_arr[100];
    pthread_t threads[NUM_THREADS];
    int threadArgs[NUM_THREADS];
    int num_threads = 0;
    int i;
    for(i=1;i<argc;++i){
        num_of_files++;

        threadArgs[i] = i;
        if(i==argc-1){
            ++num_threads;
            int result = pthread_create(&threads[i], NULL, thread_function_last, fileNames[i]);
            if (result != 0){
                printf("Error creating a thread\n");
                exit(0);
            }
        }
        else{
            ++num_threads;
            int result = pthread_create(&threads[i], NULL, thread_function, fileNames[i]);
            if (result != 0){
                printf("Error creating a thread\n");
                exit(0);
            }
        }
    }

    // Wait for all threads to finish
    for(i=1;i<argc;++i) {
        int joinResult = pthread_join(threads[i], NULL);
        if (joinResult != 0) {
            printf("Error - pthread_join fails\n");
            exit(0);
        }
    }

    fprintf(stderr, "all %d threads terminated\n", num_threads);

    print_grades()

    return 0;
}

void print_grades(){
    pthread_t finish_threads[5];
    char grades[] = {'A', 'B', 'C', 'D', 'F'};

    for (int i = 0; i < 5; i++)
    {
        pthread_create(&finish_threads[i], NULL, print_grade_thread, &grades[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < 5; i++)
    {
        pthread_join(finish_threads[i], NULL);
    }

    fprintf(stderr, "all printer-threads terminated\n");
}

void* print_grade_thread(void* grade)
{
    char grade_letter = *(char*)grade;

    // Lock the mutex to ensure synchronization
    pthread_mutex_lock(&mutex);

    // Print the thread's ID and the grade it's responsible for
    printer_thread_msg(grade_letter)

    // Check if it's this thread's turn to print the grade
    while (current_index < all_stud.count)
    {
        // Check if the grade matches the current index
        if (grade_letter == all_stud.stud_arr[current_index].avg_grade)
        {
            // Call the appropriate print_grade_X function
            switch (grade_letter)
            {
                case 'A':
                    print_grade_A(current_index);
                    break;
                case 'B':
                    print_grade_B(current_index);
                    break;
                case 'C':
                    print_grade_C(current_index);
                    break;
                case 'D':
                    print_grade_D(current_index);
                    break;
                case 'F':
                    print_grade_F(current_index);
                    break;
            }

            // Increment the current index
            current_index++;

            // Break the loop to allow other threads to check their grades
            break;
        }
    }

    // Unlock the mutex
    pthread_mutex_unlock(&mutex);

    // Exit the thread
    pthread_exit(NULL);
}


void* thread_function_last(void* arg)
{
char* fileName = (char*)arg;
// Call the calculate_grades function with the given file name
calculate_grades(fileName, true);  // Set 'last' parameter as needed
return NULL;
}

void* thread_function(void* arg)
{
    char* fileName = (char*)arg;
    // Call the calculate_grades function with the given file name
    calculate_grades(fileName, false);  // Set 'last' parameter as needed
    return NULL;
}

void calculate_grades(char students_grades_file_name[100], bool last) {
    FILE *grade_file;
    char line[100];
    bool first_word = true;
    int students = 0;
    char student_name[10];
    float count = 0;
    float sum = 0;
    float avg_grade;
    grade_file = fopen(students_grades_file_name, "r"); // open the file for reading

    if (grade_file == NULL) { // check if the file was opened successfully
        printf("Error opening file\n");
        exit(0);
    }

    while (fgets(line, sizeof(line), grade_file) != NULL) { // read lines from the file until NULL is returned
        char *token = strtok(line, " ");
        while (token != NULL) {
            if (!first_word) {// first is the name so we don't need tro convert to int
                int num = atoi(token);
                sum += num;
                count++;
            } else {
                first_word = false;
                strcpy(student_name, token);
            }
            token = strtok(NULL, " ");
        }

        avg_grade = sum / count;

        write_avg_grade(student_name, avg_grade);

        sum = 0;
        count = 0;
        first_word = true;
        ++students;
    }


    fclose(grade_file); // close the file

    if(last){
        print_student_arr();
    }
    exit(0);
}


void write_avg_grade(char student_name[10], float avg_grade) {
    struct student student = {student_name, avg_grade};
    add_to_student_arr(&student);
}


