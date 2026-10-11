#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int shmid_verticales, shmid_finales;
int *pids_verticales, *pids_finales;

void despierta(int senyal) {
}

void crear_memoria(int x, int y) {
    shmid_verticales = shmget(IPC_PRIVATE, sizeof(int) * x, IPC_CREAT | 0666);
    pids_verticales = (int *) shmat(shmid_verticales, 0, 0);

    shmid_finales = shmget(IPC_PRIVATE, sizeof(int) * y, IPC_CREAT | 0666);
    pids_finales = (int *) shmat(shmid_finales, 0, 0);
}

void liberar_memoria() {
    shmdt((char *) pids_verticales);
    shmdt((char *) pids_finales);
    shmctl(shmid_verticales, IPC_RMID, 0);
    shmctl(shmid_finales, IPC_RMID, 0);
}

void mostrar_pids(int *vector, int cantidad) {
    int i;

    for (i = 0; i < cantidad; i++) {
        printf("%d", vector[i]);
        if (i != cantidad - 1) {
            printf(", ");
        }
    }
    printf("\n");
}

void mostrar_subhijo(int x) {
    printf("Soy el subhijo %d, mis padres son: ", getpid());
    mostrar_pids(pids_verticales, x);
}

void mostrar_superpadre(int y) {
    printf("Soy el superpadre %d : mis hijos finales son: ", getpid());
    mostrar_pids(pids_finales, y);
}

void esperar_alarma() {
    signal(SIGALRM, despierta);
    alarm(10);
    pause();
}

void crear_finales(int x, int y) {
    int i;

    for (i = 0; i < y; i++) {
        if (fork() == 0) {
            pids_finales[i] = getpid();
            mostrar_subhijo(x);
            esperar_alarma();
            exit(0);
        }
    }

    for (i = 0; i < y; i++) {
        wait(NULL);
    }
}

void crear_verticales(int x, int y) {
    int i;

    for (i = 0; i < x; i++) {
        if (fork() != 0) {
            wait(NULL);
            if (i > 0) {
                exit(0);
            }
            return;
        }
        pids_verticales[i] = getpid();
    }

    crear_finales(x, y);
    exit(0);
}

int main(int argc, char *argv[]) {
    int x, y;

    if (argc != 3) {
        printf("Error. Uso: %s x y\n", argv[0]);
        return 1;
    }

    x = atoi(argv[1]);
    y = atoi(argv[2]);
    if (x < 1 || y < 1) {
        printf("Error. x e y deben ser mayores que 0\n");
        return 1;
    }

    crear_memoria(x, y);
    crear_verticales(x, y);
    mostrar_superpadre(y);
    liberar_memoria();
    return 0;
}
