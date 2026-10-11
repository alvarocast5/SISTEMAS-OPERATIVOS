#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

pid_t pid_ejec;
pid_t pid_A;
pid_t pid_B;
pid_t pid_X;
pid_t pid_Y;
pid_t pid_Z;

int segundos;
int aviso_recibido = 0;
char *nombre_hijo_B;

void leer_argumentos(int argc, char *argv[]) {
    if (argc != 2 || atoi(argv[1]) <= 0) {
        printf("Error. Uso: %s segundos (mayor que 0)\n", argv[0]);
        exit(1);
    }
    segundos = atoi(argv[1]);
}

void esperar_hijo(pid_t pid) {
    while (wait(NULL) != pid) {
    }
}

void terminar_hijo(pid_t pid) {
    kill(pid, SIGUSR2);
    esperar_hijo(pid);
}

void ejecutar_pstree(void) {
    pid_t pid = fork();

    if (pid == 0) {
        execlp("pstree", "pstree", NULL);
        perror("Error al ejecutar pstree");
        exit(1);
    }
    esperar_hijo(pid);
}

void manejador_tarea_A(int s) {
    ejecutar_pstree();
    kill(pid_ejec, SIGUSR2);
}

void manejador_destruir_A(int s) {
    terminar_hijo(pid_B);
    printf("Soy A (%d) y muero\n", pid_A);
    exit(0);
}

void manejador_aviso(int s) {
    signal(SIGUSR1, manejador_aviso);
    aviso_recibido = 1;
}

void esperar_aviso(void) {
    while (aviso_recibido == 0) {
        pause();
    }
    aviso_recibido = 0;
}

void manejador_destruir_hijo_B(int s) {
    printf("Soy %s (%d) y muero\n", nombre_hijo_B, getpid());
    exit(0);
}

void presentar_hijo_B(char *nombre) {
    printf("Soy el proceso %s: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n",
           nombre, getpid(), pid_B, pid_A, pid_ejec);
    kill(pid_B, SIGUSR1);
}

void proceso_hijo_B(char *nombre) {
    nombre_hijo_B = nombre;
    signal(SIGUSR2, manejador_destruir_hijo_B);
    presentar_hijo_B(nombre);

    while (1) {
        pause();
    }
}

pid_t crear_hijo_B(char *nombre) {
    pid_t pid = fork();

    if (pid == 0) {
        proceso_hijo_B(nombre);
        exit(0);
    }
    return pid;
}

void manejador_alarma_Z(int s) {
    kill(pid_A, SIGUSR1);
}

void proceso_Z(void) {
    nombre_hijo_B = "Z";
    signal(SIGUSR2, manejador_destruir_hijo_B);
    signal(SIGALRM, manejador_alarma_Z);
    presentar_hijo_B("Z");

    alarm(segundos);

    while (1) {
        pause();
    }
}

pid_t crear_Z(void) {
    pid_t pid = fork();

    if (pid == 0) {
        proceso_Z();
        exit(0);
    }
    return pid;
}

void manejador_destruir_B(int s) {
    terminar_hijo(pid_Z);
    terminar_hijo(pid_Y);
    terminar_hijo(pid_X);
    printf("Soy B (%d) y muero\n", pid_B);
    exit(0);
}

void proceso_B(void) {
    pid_B = getpid();
    printf("Soy el proceso B: mi pid es %d. Mi padre es %d. Mi abuelo es %d\n",
           pid_B, pid_A, pid_ejec);

    signal(SIGUSR1, manejador_aviso);
    signal(SIGUSR2, manejador_destruir_B);

    pid_X = crear_hijo_B("X");
    esperar_aviso();
    pid_Y = crear_hijo_B("Y");
    esperar_aviso();
    pid_Z = crear_Z();
    esperar_aviso();

    while (1) {
        pause();
    }
}

void crear_B(void) {
    pid_B = fork();

    if (pid_B == 0) {
        proceso_B();
        exit(0);
    }
    esperar_hijo(pid_B);
}

void proceso_A(void) {
    pid_A = getpid();
    printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n", pid_A, pid_ejec);

    signal(SIGUSR1, manejador_tarea_A);
    signal(SIGUSR2, manejador_destruir_A);

    crear_B();
}

void crear_A(void) {
    pid_A = fork();

    if (pid_A == 0) {
        proceso_A();
        exit(0);
    }
    esperar_hijo(pid_A);
}

void manejador_destruccion(int s) {
    kill(pid_A, SIGUSR2);
}

int main(int argc, char *argv[]) {
    leer_argumentos(argc, argv);

    pid_ejec = getpid();
    printf("Soy el proceso ejec: mi pid es %d\n", pid_ejec);

    signal(SIGUSR2, manejador_destruccion);
    crear_A();

    printf("Soy ejec (%d) y muero\n", pid_ejec);
    return 0;
}
