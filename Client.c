#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TAM_BLOQUE 1024
#define PUERTO 9999

int abrirSocket(void){
	int sock;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("Error. No se pudo crear el socket");
	}
	return sock;
}

void rellenarDireccion(struct sockaddr_in *dir, char *ip){
	memset(dir, 0, sizeof(*dir));
	dir->sin_family = AF_INET;
	dir->sin_port = htons(PUERTO);
	dir->sin_addr.s_addr = inet_addr(ip);
}

int conectarConServidor(char *ip){
	int sock;
	struct sockaddr_in dirServidor;

	sock = abrirSocket();
	if(sock >= 0){
		rellenarDireccion(&dirServidor, ip);
		if(connect(sock, (struct sockaddr *) &dirServidor, sizeof(dirServidor)) < 0){
			perror("Error. No se pudo conectar con el servidor");
			close(sock);
			sock = -1;
		}
	}
	return sock;
}

void recibirFichero(int origen){
	char bloque[TAM_BLOQUE];
	ssize_t nRecibidos;

	nRecibidos = read(origen, bloque, TAM_BLOQUE);
	while(nRecibidos > 0){
		write(STDOUT_FILENO, bloque, nRecibidos);
		nRecibidos = read(origen, bloque, TAM_BLOQUE);
	}
	if(nRecibidos < 0){
		perror("Error al leer del socket");
	}
}

int main(int argc, char *argv[]){
	int sockServidor;

	if(argc != 2){
		fprintf(stderr, "Uso: %s <IP_del_servidor>\n", argv[0]);
	}
	else{
		sockServidor = conectarConServidor(argv[1]);
		if(sockServidor >= 0){
			recibirFichero(sockServidor);
			close(sockServidor);
		}
	}
	return 0;
}