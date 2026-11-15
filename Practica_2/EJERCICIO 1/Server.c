#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>

#define TAM_BLOQUE 1024
#define PUERTO 9999
#define FICHERO "Google.html"

int abrirSocket(void){
	int sock;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("Error. No se pudo abrir el socket");
	}
	return sock;
}

void rellenarDireccion(struct sockaddr_in *dir){
	memset(dir, 0, sizeof(*dir));
	dir->sin_family = AF_INET;
	dir->sin_port = htons(PUERTO);
	dir->sin_addr.s_addr = htonl(INADDR_ANY);
}

int vincularYEscuchar(int sock, struct sockaddr_in *dir){
	int correcto = 0;
	int reuse = 1;

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if(bind(sock, (struct sockaddr *) dir, sizeof(*dir)) < 0){
		perror("Error. No se pudo vincular el socket");
	}
	else if(listen(sock, 5) < 0){
		perror("Error en listen");
	}
	else{
		correcto = 1;
	}
	return correcto;
}

int prepararServidor(void){
	int sock;
	struct sockaddr_in dirServidor;

	sock = abrirSocket();
	if(sock >= 0){
		rellenarDireccion(&dirServidor);
		if(!vincularYEscuchar(sock, &dirServidor)){
			close(sock);
			sock = -1;
		}
	}
	return sock;
}

int enviarBloque(int destino, char *bloque, ssize_t total){
	int correcto = 1;
	ssize_t enviado = 0;
	ssize_t nEscritos;

	while(correcto && enviado < total){
		nEscritos = write(destino, bloque + enviado, total - enviado);
		if(nEscritos <= 0){
			perror("Error al enviar al cliente");
			correcto = 0;
		}
		else{
			enviado += nEscritos;
		}
	}
	return correcto;
}

void transmitirFichero(int fd, int destino){
	int seguir = 1;
	char bloque[TAM_BLOQUE];
	ssize_t nLeidos;

	nLeidos = read(fd, bloque, TAM_BLOQUE);
	while(seguir && nLeidos > 0){
		seguir = enviarBloque(destino, bloque, nLeidos);
		if(seguir){
			nLeidos = read(fd, bloque, TAM_BLOQUE);
		}
	}
	if(nLeidos < 0){
		perror("Error al leer el fichero");
	}
}

void enviarFichero(int destino){
	int fd;

	fd = open(FICHERO, O_RDONLY);
	if(fd < 0){
		perror("Error al abrir el fichero");
	}
	else{
		transmitirFichero(fd, destino);
		close(fd);
	}
}

void atenderCliente(int sockEscucha, int sockCliente){
	close(sockEscucha);
	enviarFichero(sockCliente);
	close(sockCliente);
	_exit(0);
}

void esperarClientes(int sockEscucha){
	int sockCliente;
	struct sockaddr_in dirCliente;
	socklen_t longDirCliente;
	pid_t hijo;

	while(1){
		longDirCliente = sizeof(dirCliente);
		sockCliente = accept(sockEscucha, (struct sockaddr *) &dirCliente, &longDirCliente);
		if(sockCliente < 0){
			perror("Error al aceptar la conexion");
		}
		else{
			hijo = fork();
			if(hijo == 0){
				atenderCliente(sockEscucha, sockCliente);
			}
			if(hijo < 0){
				perror("Error en fork");
			}
			close(sockCliente);
		}
	}
}

int main(void){
	int sockEscucha;

	sockEscucha = prepararServidor();
	if(sockEscucha >= 0){
		signal(SIGCHLD, SIG_IGN);
		signal(SIGPIPE, SIG_IGN);
		esperarClientes(sockEscucha);
	}
	return 0;
}