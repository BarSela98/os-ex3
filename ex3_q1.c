#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "ex3_q1_given.h"

int num_of_files=0;
void write_avg_grade(FILE *temp_file, char student_name[10], float avg_grade);
void calculate_grades(char students_grades_file_name[100] ,bool last);

int main(int argc, char *argv[])
{
    int total_students = 0;
    char line[100];
    bool first_word = true;
    char student_name[10];
    int pid_arr[100];
    int i;
    for(i=1;i<argc;i++){
        num_of_files++;
        int id =fork();
        if(!id) {
            calculate_grades(argv[i],i=(argc-1));//second var is to signal if the file is the last file
        }
        else{
            pid_arr[num_of_files]=id;//store to pids in arr to open temp files with them
        }
    }
    while(wait(NULL)!=-1);

    int parent_to_child[2];
    int child_to_parent[2];
    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        perror("pipe");
        return 1;
    }


    if(!fork()){//read from temp files in child procces
        char file_name[100];
        FILE *all_std_file;

        chdir("..");
        all_std_file = fopen("all_std.log", "w");

        // read the num_of_files from the parent
        read(parent_to_child[0], &num_of_files, sizeof(int));

        // read the array from the parent
        read(parent_to_child[0], pid_arr, 100 * sizeof(int));


        for(int file_number = 0 ; file_number < num_of_files; ++file_number){
            sprintf(file_name, "%d",pid_arr[file_number]);
            strcat(file_name,".temp");

            FILE *grade_file = fopen(file_name, "r"); // open the file for reading

            if (grade_file == NULL) { // check if the file was opened successfully
                printf("Error opening file\n");
                exit(0);
            }

            while (fgets(line, sizeof(line), grade_file) != NULL) { // read lines from the file until NULL is returned
                char *token = strtok(line, " ");
                float avg_grade = 0;
                while (token != NULL) {
                    if (!first_word) {// first is the name so we dont need tro convert to int
                        avg_grade = atoi(token);
                    } else {
                        first_word = false;
                        strcpy(student_name, token);
                    }
                    token = strtok(NULL, " ");
                }
                first_word = true;
                total_students++;

                write_avg_grade_to_file(all_std_file, student_name, avg_grade);
            }
            fclose(grade_file);
        }
        fclose(all_std_file);

        write(child_to_parent[1], &total_students, sizeof(total_students));

        close(parent_to_child[0]);
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        close(child_to_parent[1]);
    }
    else{
        // send the num_of_files to the child
        write(parent_to_child[1], &num_of_files, sizeof(int));

        // send the array to the child
        write(parent_to_child[1], &(pid_arr[1]), 99 * sizeof(int));

        while(wait(NULL)!=-1);

        total_students = child_to_parent[0];

        close(parent_to_child[0]);
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        close(child_to_parent[1]);

        report_data_summary(total_students);
    }
    return 0;
}

void calculate_grades(char students_grades_file_name[100]) {
    pid_t pid = getpid();
    chdir("..");
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
            if (!first_word) {// first is the name so we dont need tro convert to int
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
        write_avg_grade(student_name,avg_grade);
    }


    fclose(grade_file); // close the file

    if(last){
        print_student_arr();
    }
    exit(0);
}


void write_avg_grade(char student_name[10], float avg_grade) {
    //add wrtie to the all_stud

}


