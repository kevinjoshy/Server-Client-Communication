#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>

#define  BUF_LEN 1024
#define  OPTION 3


int main(int argc, char ** argv)
{

    // Define OPTION to invoke emacs or vim instead of nano
#if   (OPTION == 1)
    char  *cmd_editor = "emacs";
#elif (OPTION == 2)
    char  *cmd_editor = "vim";
#else
    char  *cmd_editor = "nano";
#endif

    char    * cmd_make = "make";
    char    * cmd_find_error = "./find-error.py";
    char    * make_output = "make.out";   // output of make

    // Do not specify a line number first time
    int     linenumber = -1;

    // buffers
    char    c_file[BUF_LEN];
    char    make_target[BUF_LEN];

    if (argc >= 2) {
        int len = strlen(argv[1]);
        if (len > BUF_LEN - 10) {
            fprintf(stderr, "C file name or make target is too long.\n");
        }
        if (len > 2 && argv[1][len-1] == 'c' && argv[1][len-2] == '.') {
            strcpy(c_file, argv[1]);
            strcpy(make_target, argv[1]);
            make_target[len-2] = 0;
        }
        else {
            strcpy(make_target, argv[1]);
            strcpy(c_file, argv[1]);
            strcat(c_file, ".c");
        }
    }
    else {
        strcpy(c_file, "bugs.c");     // default C file to work on
        strcpy(make_target, "bugs");
    }

    fprintf(stderr, "Working on %s using %s, %s, and %s\n", c_file, cmd_editor, cmd_make, cmd_find_error);
    fprintf(stderr, "Initial line number: %d; make output saved in %s\n", linenumber,  make_output);

    // If you'd like to prepare more before getting into the loop,
    // do it here.
    // TODO

    int     done = 0;
    
    if (argc > 2) {
        char temp[5]; 
        strcpy(temp, (argv[2] ));
        linenumber = atoi(temp);
    }
        
    do {
        // invoke editor to edit c_file
        // TODO
        char* command;
        command = (char*)malloc(sizeof(char) * (strlen(cmd_editor) + strlen(c_file) + 6));
        strcpy(command, cmd_editor );
        strcat(command, " " );
        if (linenumber > 0) {
            char inttostr[5];
            sprintf(inttostr, "%d", linenumber);
            
            strcat(command, "+" );
            strcat(command, inttostr );
            strcat(command, " " );
            strcat(command, c_file );
        }
        else {
            strcat(command, c_file );
        }
        system(command);
        wait(NULL);
        free(command);
        
        // invoke make to build make_target and save stdout and stderr to make_output
        // TODO
        
        char* makeCommand; // cc -g -Wall -Werror -std=c99 -o 
        makeCommand = (char*)malloc(sizeof(char) * (strlen("make ") + strlen(make_target) + strlen(" 2> make.out") + 2));
        strcpy(makeCommand, "make " );
        strcat(makeCommand, make_target );
        strcat(makeCommand, " 2> make.out" );
        
        system(makeCommand);
        
        free(makeCommand);

        // invoke ./find-error.py to find first error/warning in make_output
        // open a pipe to get back the line wuth the first error/warning
        // TODO
        char* firstError;
        firstError = (char*)malloc(sizeof(char) * strlen("./find-error.py < make.out") + 1);
        strcpy(firstError, "./find-error.py < make.out" ); 
        
        FILE* errorMSG;
        errorMSG = popen(firstError,"r");
        free(firstError);
        
        int notEmpty = 0;
        int update =0;
        char c;
        while (1) {
            if ((c = fgetc(errorMSG)) == '\n') {
                if (notEmpty==0) {
                    printf("No error or warning found.");
                }
                break;
            }
            else {
                notEmpty=1;
                
                if (update==0) {
                    char temp[50];
                    if (c == ':') {
                        fputc(c,stdout);
                        int i=0; 
                        while ((c = fgetc(errorMSG)) != ':') {
                            fputc(c,stdout);
                            temp[i] = c;
                            i++;
                        }
                        temp[i] = '\0';
                        // printf("\nTemp: %s\n",temp);
                        linenumber = atoi(temp);
                        update = 1;
                        fputc(c,stdout);
                        // printf("\nlinenumchar: %s\n",temp);
                        // printf("linenum: %d\n",linenumber);
                    }
                    else {
                        fputc(c,stdout);
                    }
                }
                else {
                    fputc(c,stdout);
                }
                // fputc(c,stdout);
            }
        }
        
        pclose(errorMSG);
        printf("\n");
        // read the line with first error/warning and get the line number
        // TODO


        // ask if the user wants to continue
        char answer;
        printf("Would you like to continue?(y/n)\n");
        answer = getchar();
        if (answer == EOF || (answer != '\n'  && tolower(answer) != 'y'))
            done = 1;
        else {
            // skip to the end of the line
            while (answer != EOF && answer != '\n')
                answer = getchar();
        }
    } while (! done);

    // If you'd like to clean up after the loop,
    // do it here.
    // TODO

    return 0;
}
