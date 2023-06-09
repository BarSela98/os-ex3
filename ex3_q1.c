#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ex3_q1_given.h"

// vars
#define NUM_THREADS 100
struct all_students all_stud;
pthread_mutex_t mutex2;
pthread_t threads[NUM_THREADS];
pthread_mutex_t mutex;
int current_index = 0;
int num_of_files = 0;


// functions
void initializeMutexes();

char get_character_from_grade(double grade);

void write_avg_grade(char student_name[10], double avg_grade);

void calculate_grades(char students_grades_file_name[100]);

void *thread_function(void *arg);

void *thread_function_last(void *arg);

void *print_grade_thread(void *grade);

void print_grades_by_thread();

struct student create_student(const char *name, double avg_grade);

void calculate_students_grades_by_thread(int number_of_files, char *files_names[]);

// main
int main(int argc, char *argv[]) {
    initializeMutexes();

    calculate_students_grades_by_thread(argc, argv);

    print_grades_by_thread();

    return 0;
}

void initializeMutexes() {
    // Initialize the mutexes
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex2, NULL);
}


void calculate_students_grades_by_thread(int number_of_files, char *files_names[]) {
    int threadArgs[NUM_THREADS];
    int num_threads = 0;
    int i;
    for (i = 1; i < number_of_files; ++i) {
        num_of_files++;

        threadArgs[i] = i;
        if (i == number_of_files - 1) {
            ++num_threads;
            int result = pthread_create(&threads[i], NULL, thread_function_last, files_names[i]);
            if (result != 0) {
                printf("Error creating a thread\n");
                exit(0);
            }
        } else {
            ++num_threads;
            int result = pthread_create(&threads[i], NULL, thread_function, files_names[i]);
            if (result != 0) {
                printf("Error creating a thread\n");
                exit(0);
            }
        }
    }

    // Wait for all threads to finish
    for (i = 1; i < number_of_files; ++i) {
        int joinResult = pthread_join(threads[i], NULL);
        if (joinResult != 0) {
            printf("Error - pthread_join fails\n");
            exit(0);
        }
    }

    fprintf(stderr, "all %d threads terminated\n", num_threads);
}

void print_grades_by_thread() {
    pthread_t finish_threads[5];
    char grades[] = {'A', 'B', 'C', 'D', 'F'};

    for (int i = 0; i < 5; i++)
        pthread_create(&finish_threads[i], NULL, print_grade_thread, &grades[i]);

    // Wait for all threads to finish
    for (int i = 0; i < 5; i++)
        pthread_join(finish_threads[i], NULL);

    fprintf(stderr, "all printer-threads terminated\n");
}

void *print_grade_thread(void *grade) {
    char grade_letter = *(char *) grade;

    printer_thread_msg(grade_letter);


    while (current_index < all_stud.count) {
        // Check if the grade matches the current index
        char grade_character = get_character_from_grade(all_stud.stud_arr[current_index].avg_grade);
        if (grade_letter == grade_character) {
            switch (grade_letter) {
                case 'A':
                    pthread_mutex_lock(&mutex);
                    print_grade_A(current_index);
                    break;
                case 'B':
                    pthread_mutex_lock(&mutex);
                    print_grade_B(current_index);
                    break;
                case 'C':
                    pthread_mutex_lock(&mutex);
                    print_grade_C(current_index);
                    break;
                case 'D':
                    pthread_mutex_lock(&mutex);
                    print_grade_D(current_index);
                    break;
                case 'F':
                    pthread_mutex_lock(&mutex);
                    print_grade_F(current_index);
                    break;
                default:
                    printf("Error - unknown letter\n");
                    exit(0);
            }

            // Increment the current index
            current_index++;
            pthread_mutex_unlock(&mutex);
        }
    }

    // Exit the thread
    pthread_exit(NULL);
}


void *thread_function_last(void *arg) {
    char *fileName = (char *) arg;
    // Call the calculate_grades function with the given file name
    calculate_grades(fileName);

    sleep(2);
    print_student_arr();
    return NULL;
}

void *thread_function(void *arg) {
    char *fileName = (char *) arg;
    // Call the calculate_grades function with the given file name
    calculate_grades(fileName);

    return NULL;
}

void calculate_grades(char students_grades_file_name[100]) {
    FILE *grade_file;
    char line[100];
    bool first_word = true;
    int students = 0;
    char student_name[10];
    double count = 0;
    double sum = 0;
    double avg_grade;

    grade_file = fopen(students_grades_file_name, "r"); // open the file for reading

    if (grade_file == NULL) { // check if the file was opened successfully
        printf("Error opening file\n");
        exit(0);
    }

    while (fgets(line, sizeof(line), grade_file) != NULL) { // read lines from the file until NULL is returned
        char *token = strtok(line, " ");
        while (token != NULL) {
            if (!first_word) {// first is the name, so we don't need tro convert to int
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
}

struct student create_student(const char *name, double avg_grade) {
    struct student new_student;
    strncpy(new_student.name, name, MAX_NAME_LEN);
    new_student.name[MAX_NAME_LEN] = '\0'; // Ensure null-terminated string
    new_student.avg_grade = avg_grade;

    return new_student;
}

void write_avg_grade(char student_name[10], double avg_grade) {
    // Lock the mutex to ensure synchronization
    pthread_mutex_lock(&mutex2);

    struct student new_student = create_student(student_name, avg_grade);
    add_to_student_arr(&new_student);

    // Unlock the mutex
    pthread_mutex_unlock(&mutex2);
}

char get_character_from_grade(double grade) {
    if (grade >= 90 && grade <= 100) {
        return 'A';
    } else if (grade >= 80 && grade < 90) {
        return 'B';
    } else if (grade >= 70 && grade < 80) {
        return 'C';
    } else if (grade >= 60 && grade < 70) {
        return 'D';
    } else if (grade >= 0 && grade < 60) {
        return 'F';
    } else {
        return 'X'; // Invalid grade
    }
}

