#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

void escribir_texto(int descriptor, char *texto) {
    write(descriptor, texto, strlen(texto));
}

void mostrar_uso(char *programa) {
    escribir_texto(1, "Error. Uso: ");
    escribir_texto(1, programa);
    escribir_texto(1, " fichero tam_trozo\n");
}

int calcular_trozos(char *nombre, int tamanyo) {
    struct stat propiedades;
    int trozos;

    stat(nombre, &propiedades);
    trozos = propiedades.st_size / tamanyo;
    if (propiedades.st_size % tamanyo != 0) {
        trozos++;
    }
    return trozos;
}

void crear_nombre_trozo(char *destino, char *nombre, int numero) {
    sprintf(destino, "%s.h%02d", nombre, numero);
}

void guardar_trozo(int tubo[2], char *nombre, int numero, int tamanyo) {
    char nombre_trozo[256];
    char *trozo;
    int salida, leidos;

    close(tubo[1]);
    trozo = (char *) malloc(tamanyo);
    crear_nombre_trozo(nombre_trozo, nombre, numero);
    salida = creat(nombre_trozo, 0666);

    leidos = read(tubo[0], trozo, tamanyo);
    while (leidos > 0) {
        write(salida, trozo, leidos);
        leidos = read(tubo[0], trozo, tamanyo);
    }

    close(salida);
    close(tubo[0]);
    free(trozo);
}

void enviar_trozo(int entrada, int tubo[2], char *buffer, int tamanyo) {
    int leidos;

    close(tubo[0]);
    leidos = read(entrada, buffer, tamanyo);
    if (leidos > 0) {
        write(tubo[1], buffer, leidos);
    }
    close(tubo[1]);
}

void partir(char *nombre, int tamanyo) {
    int entrada, trozos, i;
    int tubo[2];
    char *buffer;
    pid_t pid;

    entrada = open(nombre, O_RDONLY);
    if (entrada < 0) {
        escribir_texto(1, "Error. No existe el fichero\n");
        return;
    }

    trozos = calcular_trozos(nombre, tamanyo);
    buffer = (char *) malloc(tamanyo);

    for (i = 0; i < trozos; i++) {
        pipe(tubo);
        pid = fork();
        if (pid == 0) {
            close(entrada);
            free(buffer);
            guardar_trozo(tubo, nombre, i, tamanyo);
            exit(0);
        }
        enviar_trozo(entrada, tubo, buffer, tamanyo);
    }

    for (i = 0; i < trozos; i++) {
        wait(NULL);
    }

    close(entrada);
    free(buffer);
}

int main(int argc, char *argv[]) {
    int tamanyo;

    if (argc != 3) {
        mostrar_uso(argv[0]);
        return 1;
    }

    tamanyo = atoi(argv[2]);
    if (tamanyo <= 0) {
        escribir_texto(1, "Error. El tamanyo del trozo debe ser mayor que 0\n");
        return 1;
    }

    partir(argv[1], tamanyo);
    return 0;
}
