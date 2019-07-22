#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

struct client_info {
	int client_descriptor;
	socklen_t peer_addr_len;
	struct sockaddr_storage peer_addr;
};

int initialize_server(){
	int s, b, error;
	struct addrinfo hints, *res , *res0;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	error = getaddrinfo(NULL, "http", &hints, &res0);
	if(error != 0){
		errx(1, "%s", gai_strerror(error));
		exit(EXIT_FAILURE);
	}
	
	s = -1;
	for(res = res0; res != NULL; res = res->ai_next){
		s = socket(res->ai_family, res->ai_socktype, 0);
		printf("Funcion socket ejecutada con resultado: %i\n", s);
		if(s == -1)
			continue; //error getting descriptor, try another socket
		b = bind(s, (struct sockaddr *) res->ai_addr, res->ai_addrlen);
		if( b == 0){
			printf("Funcion bind ejecutada con exito\n");
			break;	//success
		}
		perror(strerror(errno));
		close(s);
	}
	
	if(res == NULL){		//no address succeeded
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}
	
	
	freeaddrinfo(res0);
	
	return s;
}

void handle_client(int client_descriptor, struct sockaddr_storage peer_addr, socklen_t peer_addr_len){
	const int BUF_SIZE = 5000;
	char *action, *path;
	char buf[BUF_SIZE];
	char host[NI_MAXHOST], service[NI_MAXSERV];
	int sinf;
	ssize_t nread;
	
	sinf = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len,
					   host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV|NI_NUMERICHOST);
	if(sinf == 0){
		printf("Connection accepted from %s:%s\n", host, service);
	}
	else
		fprintf(stderr, "getnameinfo: %s\n", gai_strerror(sinf));
	
	memset(buf, 0, 5000);
	nread = recv(client_descriptor, buf, BUF_SIZE, 0);
	if (nread == -1){
		fprintf(stderr, "Error receiving data\n");
		exit(EXIT_FAILURE);
	}
	else{
		buf[nread] = '\0';
		printf("Recibidos %i bytes: \n", nread);
		printf("%s", buf);
		
		printf("\n Comienzo del parseo: \n\n");
		action = strtok(buf, " ");
		printf("String 1: \n");
		printf("%s", action);
		path = strtok(NULL, " ");
		printf("String 2: %s", path);
	}
	exit(EXIT_SUCCESS);
}

int main(int *argc, char *argv[]){
	
	
	int s;
	struct sockaddr_storage peer_addr;
	socklen_t peer_addr_len;
	struct client_info client;
	
	s = initialize_server();
	
	if(listen(s, 5) == -1){
		fprintf(stderr, "Could not listen\n");
		exit(EXIT_FAILURE);
	}
	
	while (1){
	
		peer_addr_len = sizeof(struct sockaddr_storage);
		client.client_descriptor = accept(s, (struct sockaddr *) &client.peer_addr, &client.peer_addr_len);
		
		if(client.client_descriptor == -1){
			fprintf(stderr, "Could not accept\n");
		}
		else{
			
			if(fork() == 0){
				handle_client(client.client_descriptor, client.peer_addr, client.peer_addr_len);
			}
		}
	}
}