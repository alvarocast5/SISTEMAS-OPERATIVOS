#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

static void al_despertar(int sig){
    (void)sig;
}

static void rama(int nivel, int filas){
    pid_t pid;

    if(nivel == filas){
        signal(SIGALRM, al_despertar);
        alarm(10);
        pause();
        exit(0);
    }

    pid = fork();
    if(pid < 0){
        perror("Error en fork");
        exit(1);
    }
    if(pid == 0){
        rama(nivel + 1, filas);
    }
    wait(NULL);
    exit(0);
}

int main(int argc, char *argv[]){
    int filas, columnas, c;
    pid_t pid;

    if(argc == 3 && atoi(argv[1]) > 0 && atoi(argv[2]) > 0){
        filas = atoi(argv[1]);
        columnas = atoi(argv[2]);

        for(c = 0; c < columnas; c++){
            pid = fork();
            if(pid < 0){
                perror("Error en fork");
                exit(1);
            }
            if(pid == 0){
                rama(1, filas);
            }
        }

        for(c = 0; c < columnas; c++){
            wait(NULL);
        }
    }
    else{
        printf("Uso: %s x(filas) y(columnas), ambos mayores que 0\n", argv[0]);
    }
    return 0;
}
