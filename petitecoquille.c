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
        int nbpipe = 0;
        int emplacementpipe[100];
        int emplac = 0;
        for (int i = 0; argv[i] != NULL; i++) {
            if (strcmp(argv[i], "|") == 0) {
                nbpipe+=1;
                int plaq = i +1;
                emplacementpipe[emplac] = plaq;
                emplac++;

            }

        }
        if (nbpipe != 0) {
            nbpipe += 1;
        }
        char **argv2pipe[nbpipe];
        argv2pipe[0] = &argv[0];
        // if (nbpipe!=0)
        //     nbpipe -= 1; a verif si on garde ou pas car risque de pb de memoire 

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
            else if (strcmp(argv[i], "|") == 0) {
                // int z = 0;
                // for (int j = i+1; argv[j] != NULL; j++) {
                //     argv2pipe[z] = argv[j];
                //     z++;
                // }


                // argv2pipe[z] = NULL;
                argv[i] = NULL;
                pipeverif = 1;

            }
        }
        int increpipe = 0;
        for (int j =1; j<nbpipe; j++) {
            argv2pipe[j] = &argv[emplacementpipe[increpipe]];
            increpipe++;
        }
        if (erreurredirect== -1)
            continue;
        if (pipeverif == 1) {
            if(pipe(fd)==-1) {
                perror("pipe");
            }else {
                int entreeprecedante = -1;
                pipe(fd);
                for (int k = 0; k < nbpipe; k++) {
                    pid_t enfant1 = fork();
                    if (enfant1 == -1) {
                        perror("fork gosse 1");
                    }
                    if (enfant1 == 0) {
                        if (entreeprecedante == -1) { // debut
                            if (fichi!= -2 && sortie != -2) {
                                dup2(fichi, sortie);
                            }
                            dup2(fd[1], STDOUT_FILENO);
                            entreeprecedante = fd[0];
                            close(fd[0]);
                            close(fd[1]);
                            if (execvp(argv2pipe[k][0], argv2pipe[k]) == -1) {
                                perror("execvp");
                                exit(1);
                            }
                        }
                        else if (k == nbpipe-1) { // fin
                            if (fichi!= -2 && sortie != -2) {
                                dup2(fichi, sortie);
                            }
                            dup2(entreeprecedante, STDIN_FILENO);
                            entreeprecedante = fd[0];
                            //close(fd[0]);
                            close(fd[1]);
                            close(entreeprecedante);
                            if (execvp(argv2pipe[k][0], argv2pipe[k]) == -1) {
                                perror("execvp");
                                exit(1);
                            }
                        }
                        else { // milieu
                            if (fichi!= -2 && sortie != -2) {
                                dup2(fichi, sortie);
                            }
                            dup2(entreeprecedante, STDIN_FILENO);
                            dup2(fd[1], STDOUT_FILENO);
                            entreeprecedante = fd[0];
                            //close(fd[0]);
                            close(fd[1]);
                            close(entreeprecedante);
                            if (execvp(argv2pipe[k][0], argv2pipe[k]) == -1) {
                                perror("execvp");
                                exit(1);
                            }
                        }


                    }
                    // pid_t enfant2 = fork();
                    // if (enfant2 == -1) {
                    //     perror("fork gosse 2");
                    // }
                    // if (enfant2 == 0) {
                    //     dup2(entreeprecedante, STDIN_FILENO);
                    //     close(fd[1]);
                    //     close(fd[0]);
                    //     if (execvp(argv2pipe[0][0], argv2pipe[0]) == -1) {
                    //         perror("execvp");
                    //         exit(1);
                    //     }
                    // }
                    if (enfant1 != -1) {
                        close(entreeprecedante);
                        entreeprecedante = fd[0];
                        close(fd[1]);
                        //close(fd[0]);
                        waitpid(enfant1, NULL, 0);
                        // waitpid(enfant2, NULL, 0);
                    }
                    if (k+1 != nbpipe) {
                        pipe(fd);
                    }
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
