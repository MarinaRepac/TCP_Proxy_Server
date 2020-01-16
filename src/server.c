/* 
    ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2018/2019
    Semestar:       Zimski (V)
    
    Ime fajla:      server.c
    Opis:           TCP klijent
    
    Autori: 		Aleksandar Zagorac
    				Marina Repac
    				
    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
    ********************************************************************
*/
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <netinet/in.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   25
#define CLIENT_CONNECTIONS 4
#define NUM_PROXY 1

void* ConnectionHandler(void *socketDescPom);

int main(int argc , char *argv[])
{
    int socketDesc, socketDescToPorxy[CLIENT_CONNECTIONS] , clientSock[CLIENT_CONNECTIONS] , c , readSize;
    struct sockaddr_in server;
       
    socketDesc = socket(AF_INET , SOCK_STREAM , 0);
    
    server.sin_family = AF_UNSPEC; //AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);
     
    if( bind(socketDesc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        return 1;
    }

    listen(socketDesc , NUM_PROXY);
     
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    
    
    //accepting clients and creating thread to handle the function
    int *newSock;
    int j = 0;
    pthread_t snifferThread[CLIENT_CONNECTIONS];
    while( ( clientSock[j] = accept(socketDesc, (struct sockaddr *)&server, (socklen_t*)&c)) )
	{
		socketDescToPorxy[j] = socket(AF_INET , SOCK_STREAM , 0);
		if (socketDescToPorxy[j] == -1)
		{
		    printf("Could not create socket for server");
		    return 1;
		}
		newSock = malloc(1);
		*newSock = clientSock[j];
		
		if( pthread_create( &snifferThread[j] , NULL ,  ConnectionHandler , (void*)newSock) < 0)
		{
			perror("could not create thread");
			return 1;
		}
		
		//Now join the thread 
		
		pthread_join( snifferThread[j], NULL);

		if (clientSock[j] < 0)
		{
		
			perror("accept failed");
			return 1;
		}
		j++;
	
   	
	}
   
     
    return 0;
}
void *ConnectionHandler(void *socketDescPom)
{

	struct sockaddr_in client;
	char clientMessage[DEFAULT_BUFLEN];
	int c, readSize;
	c = sizeof(struct sockaddr_in);
		 
	int socketDesc = *(int*)socketDescPom;

    while( (readSize = recv(socketDesc , clientMessage , DEFAULT_BUFLEN , 0)) > 0 )
    {	
		//Send the message back to client
		puts("-----------------------------------");
		puts("--Message from client:");
		printf("--%s\n ",clientMessage);
		fflush(stdout);
		puts("-----------------------------------");
		if( send(socketDesc, clientMessage, strlen(clientMessage), 0) < 0)
		{
			perror("Send failed");
			return 1;
		}
		
		memset(clientMessage,'\0', DEFAULT_BUFLEN);	
		
    }
    
	
    if(readSize == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(readSize == -1)
    {
        perror("recv failed");
    }
}

