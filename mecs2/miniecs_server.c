#include<stdio.h>
#include<stdlib.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<stdbool.h>
#include<pthread.h>
#include<sys/mman.h>
#include<wait.h>

#define BUFFER 1000

pthread_mutex_t fileLock1 = PTHREAD_MUTEX_INITIALIZER;

//Estructura para guardar los parametros que se van a pasar a los hilos
typedef struct __pthread_args{;
	char name[20];
	char image[20];
} pthread_args;

//Esta funcion se encarga de enviar al host la peticion de crear un contenedor
int create_container(void *arg){
	pthread_args * args = arg;
	FILE *fptr;
	char * token;
	char *contents = NULL;
    size_t len = 0;
	bool exists = false;
	pid_t pid;

	pthread_mutex_lock( &fileLock1 );
	fptr = fopen("containers.txt", "r");
	while(getline(&contents, &len, fptr) != -1){
		token = strtok(contents, " ");
		if(strcmp(token, args->name) == 0){
			exists = true;
			break;
		}
	}

	fclose(fptr);
	pthread_mutex_unlock( &fileLock1 );

	if(!exists){
		pid = fork();

		if(pid == 0){
			execlp("docker", "docker", "run", "-di", "--name", args->name, args->image, "/bin/bash", NULL);
		} else{
			wait(NULL);
			printf("Contenedor %s corriendo\n", args->name);

			pthread_mutex_lock( &fileLock1 );
			fptr = fopen("containers.txt", "a");
			strcat(args->name, " corriendo\n");
			fputs(args->name, fptr);
			fclose(fptr);
			pthread_mutex_unlock( &fileLock1 );
		}
	} else{
		printf("El contenedor %s ya existe\n", args->name);
	}
	free(args);
}

int stop_container(void *arg){
	pthread_args *args = arg;
	char replace[30], compare[30], buffer[BUFFER];
	FILE *fptr;
	FILE *fptr2;
	char *contents = NULL;
	char * token;
    size_t len = 0;
	int running = 0, line = 0;
	bool exists = false;
	pid_t pid;

	memset(compare, 0, 30);
	strcpy(compare, args->name);
	strcat(compare, " corriendo\n");

	pthread_mutex_lock( &fileLock1 );
	contents = NULL;
	fptr = fopen("containers.txt", "r");

	while(getline(&contents, &len, fptr) != -1){
		if(strcmp(contents, compare) == 0){
			exists = true;
			break;
		}
		running++;
	}
	fclose(fptr);
	pthread_mutex_unlock( &fileLock1 );

	if(exists){

		pid = fork();

		if(pid == 0){
			execlp("docker", "docker", "stop", args->name, NULL);
		} else{
			wait(NULL);
			printf("Contenedor %s detenido\n", args->name);

			memset(replace, 0, 30);
			strcpy(replace, args->name);
			strcat(replace, " detenido\n");

			pthread_mutex_lock( &fileLock1 );
			fptr = fopen("containers.txt", "r");
			fptr2 = fopen("temp.txt", "w");

			while((fgets(buffer, BUFFER, fptr)) != NULL){
				if(line != running){
					fputs(buffer, fptr2);
				} else{
					fputs(replace, fptr2);
				}
				line++;
			}

			fclose(fptr2);
			fclose(fptr);

			remove("containers.txt");
			rename("temp.txt", "containers.txt");
			pthread_mutex_unlock( &fileLock1 );
		}
	} else{
		printf("El contenedor %s no esta corriendo\n", args->name);
	}

	free(args);
}

//Esta funcion se encarga de enviar al host la peticion de detener o eliminar un contenedor
int delete_container(void *arg){
	pthread_args *args = arg;
	char buffer[BUFFER], compare[30];
	FILE *fptr;
	FILE *fptr2;
	char *contents = NULL;
	char * token;
    size_t len = 0;
	int del = 0, line = 0;
	bool exists = false;
	pid_t pid;

	memset(compare, 0, 30);
	strcpy(compare, args->name);
	strcat(compare, " detenido\n");

	pthread_mutex_lock( &fileLock1 );
	contents = NULL;
	fptr = fopen("containers.txt", "r");

	while(getline(&contents, &len, fptr) != -1){
		if(strcmp(contents, compare) == 0){
			exists = true;
			break;
		}
		del++;
	}
	fclose(fptr);

	pthread_mutex_unlock( &fileLock1 );

	if(exists){
		pid = fork();

		if(pid == 0){
			execlp("docker", "docker", "rm", args->name, NULL);
		} else{
			wait(NULL);
			printf("Contenedor %s eliminado\n", args->name);

			pthread_mutex_lock( &fileLock1 );

			fptr = fopen("containers.txt", "r");
			fptr2 = fopen("temp.txt", "w");

			while((fgets(buffer, BUFFER, fptr)) != NULL){
				if(line != del){
					fputs(buffer, fptr2);
				}
				line++;
			}

			fclose(fptr2);
			fclose(fptr);

			remove("containers.txt");
			rename("temp.txt", "containers.txt");
			pthread_mutex_unlock( &fileLock1 );
		}
	} else{
		printf("El contenedor %s no esta detenido\n", args->name);
	}

	free(args);
}

//Esta funcion se encarga de enviar al host la peticion de listar los contenedores
int list_containers(){
	pid_t pid;

	pid = fork();

	if(pid == 0){
		execlp("docker", "docker", "ps", "-a", NULL);
	} else{
		wait(NULL);
		printf("Contenedores listados\n");
	}
}

int main(int argc, char *argv[]) {
	int admin_socket, ecs_client, c;
	char client_message[30], petition[20], name[20], buffer[20], image[20];
	char * token;
	bool received = false;

	struct sockaddr_in server, client;

	admin_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (admin_socket == -1) {
		printf("Could not create socket");
	}
	puts("Miniecs server created");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 15600 );
	
	//Bind the socket to the address and port number specified
	if( bind(admin_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
		//print the error message
		perror("Bind failed. Error");
		return 1;
	}
	puts("Miniecs server bind done");

	while(1){
		//Listen
		listen(admin_socket, 10);
		
		//Accept and incoming connection
		puts("Waiting for incoming petitions...");
		c = sizeof(struct sockaddr_in);
		
		//accept connection from an incoming client
		ecs_client = accept(admin_socket, (struct sockaddr *)&client, (socklen_t*)&c);
		if (ecs_client < 0) {
			perror("Accept failed");
			return 1;
		}

		received = false;
		while(!received) {
			memset( client_message, 0, 30 );
			if (recv(ecs_client, client_message, 30, 0) > 0) {
				printf("Received from client: %s\n", client_message);
				token = strtok(client_message, " ");
				strcpy(petition, token);

				if(strcmp(petition, "list") != 0){
					token = strtok(NULL, " ");
					strcpy(name, token);
					
					if(strcmp(petition, "create") == 0){
						token = strtok(NULL, " ");
						strcpy(image, token);
						pthread_t create;
						pthread_args * args_c = malloc(sizeof(*args_c));
						strcpy(args_c->name, name);
						strcpy(args_c->image, image);
						pthread_create( &create, NULL, (void*) create_container, args_c);

					} else if(strcmp(petition, "delete") == 0){
						pthread_t delete;
						pthread_args * args_d = malloc(sizeof(*args_d));
						strcpy(args_d->name, name);
						pthread_create( &delete, NULL, (void*) delete_container, args_d);
					} else if(strcmp(petition, "stop") == 0){
						pthread_t stop;
						pthread_args * args_s = malloc(sizeof(*args_s));
						strcpy(args_s->name, name);
						pthread_create( &stop, NULL, (void*) stop_container, args_s);
					} 
					received = true;
					printf("Finalized\n");
				} else if(strcmp(petition, "list") == 0){
					pthread_t list;
					pthread_create( &list, NULL, (void*) list_containers, NULL);
					received = true;
				}	
			}
		}
		printf("\n");		
	}	
	return 0;
}