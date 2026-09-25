#include <stdio.h>
#include <fcntl.h>
#include<stdlib.h>
#include <string.h>
# include <unistd.h>
# include <sys/wait.h>


int main() {
    char ch[100] = "";
    char opt[100] = "";
    char *argv[100];
    char *argv2pipe[100];
    while ((strcmp(ch, "exit") != 0 )) {
        printf("$");

        if ( fgets(ch, 100, stdin) == NULL)
            return -1;
        if (ch[strlen(ch) - 1] == '\n') {
            ch[strlen(ch) - 1] = '\0';
        }
        printf("%s", ch);
        // size_t taille = strlen(ch);
        if (strcmp(ch, "")==0) {
            printf("commande ne peut pas etre null\n");
            continue;
        }
        argv[0] = strtok(ch, " ");
        printf("\n");
        int i = 0;
        do {
            i++;
            argv[i] = strtok(NULL, " ");
        }
        while (argv[i] != NULL);

        if (strcmp(argv[0], "exit") == 0)
            exit(0);

        // for (int i = 0; argv[i] != NULL; i++) {
        //     printf("%s\n", argv[i]);
        // }
        if (strcmp(argv[0], "cd") == 0) {
            if (argv[1] == NULL) {
                argv[1] = getenv("HOME");
            }

            if (chdir(argv[1]) == -1) {
                argv[1] = NULL;
                perror("chdir");
                continue;
            }
            argv[1] = NULL;

            continue;
        }
        int fichi = -2;
        int sortie = -2;
        int erreurredirect = 0;
        int pipeverif = 0;
        int fd[2];
        for (i = 0; argv[i] != NULL; i++) {
            if (strcmp(argv[i], ">") == 0) {
                i++;
                fichi = open(argv[i], O_RDWR | O_CREAT | O_TRUNC , 0644);
                if (fichi == -1) {
                    perror("open");
                    erreurredirect = -1 ;
                }
                i-=1;
                argv[i] = NULL;
                sortie = 1;
            }
            else if (strcmp(argv[i], "|") == 0) {
                int z = 0;
                for (int j = i+1; argv[j] != NULL; j++) {
                    argv2pipe[z] = argv[j];
                    z++;
                }
                argv2pipe[z] = NULL;
                argv[i] = NULL;
                pipeverif = 1;

            }
            else if (strcmp(argv[i], "<") == 0) {
                i++;
                fichi = open(argv[i], O_RDONLY);
                if (fichi == -1) {
                    perror("open");
                    erreurredirect = -1 ;
                }


                i-=1;
                argv[i] = NULL;
                sortie = 0;
            }
            else if (strcmp(argv[i], ">>") == 0) {
                i++;
                fichi = open(argv[i], O_RDWR | O_CREAT | O_APPEND, 0644);
                if (fichi == -1) {
                    perror("open");
                    erreurredirect = -1 ;
                }
                i-=1;
                argv[i] = NULL;
                sortie = 1;
            }
        }
        if (erreurredirect== -1)
            continue;
        if (pipeverif == 1) {
            if(pipe(fd)==-1) {
                perror("pipe");
            }else {
                pid_t enfant1 = fork();
                if (enfant1 == -1) {
                    perror("fork gosse 1");
                }
                if (enfant1 == 0) {
                    dup2(fd[1], STDOUT_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                    if (execvp(argv[0], argv) == -1) {
                        perror("execvp");
                        exit(1);
                    }
                }
                pid_t enfant2 = fork();
                if (enfant2 == -1) {
                    perror("fork gosse 2");
                }
                if (enfant2 == 0) {
                    dup2(fd[0], STDIN_FILENO);
                    close(fd[1]);
                    close(fd[0]);
                    if (execvp(argv2pipe[0], argv2pipe) == -1) {
                        perror("execvp");
                        exit(1);
                    }
                }
                if (enfant1 != -1 && enfant2 != -1) {
                    close(fd[1]);
                    close(fd[0]);
                    waitpid(enfant1, NULL, 0);
                    waitpid(enfant2, NULL, 0);
                }

            }

        }
        if (pipeverif != 1) {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
            }
            else if (pid == 0) {
                if (fichi!= -2 && sortie != -2) {
                    dup2(fichi, sortie);
                }

                if (execvp(argv[0], argv) == -1) {
                    perror("execvp");
                    exit(1);
                }

            }
            else {
                wait(NULL);
            }
        }

    }

}
