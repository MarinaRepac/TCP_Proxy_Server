/* 
    ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2018/2019
    Semestar:       Zimski (V)
    
    Ime fajla:      proxy.c
    Opis:           TCP klijent
    
    Autori:			Aleksandar Zagorac
    				Marina Reapac
    				
    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
    ********************************************************************
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>


#define NUM_BLOK 5
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015
#define SERVER_PORT   25
#define CLIENT_CONNECTIONS 4



char clientMessageRecv[DEFAULT_BUFLEN];
char serverMessageRecv[DEFAULT_BUFLEN];
char serverMessage[DEFAULT_BUFLEN];
const char* blocked[NUM_BLOK] = {"Pera", "Mika", "Zika", "Laza", "Marko"};
char serverToProxyReply[DEFAULT_BUFLEN];

int controle = 0;  
int controleForKillingThreads = 0;

pthread_mutex_t m,m1; 
pthread_cond_t cond; 
pthread_cond_t cond2, cond3; 

void *ConnectionHandler(void *);
void *ServerCommunication(void *);
void *ClientCommunication(void *);


void CopyMessageToGlobal(char* globalString, char* string);
char* CheckUser(char* user, int* pom);


int main(int argc , char *argv[])
{

    int socketDescClient, socketDescServer, clientSock[CLIENT_CONNECTIONS ], c , readSize, serverSock;
    struct sockaddr_in server , client;

    //create socket
    socketDescClient = socket(AF_INET , SOCK_STREAM , 0);
    if (socketDescClient == -1)
    {
        printf("Could not create socket for client");
    }
    puts("Socket created for client");
     
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( DEFAULT_PORT );
     
     
    //bind
    if( bind(socketDescClient,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind to client failed. Error");
        return 1;
    }
	
    puts("bind to client done");
     
    listen(socketDescClient , CLIENT_CONNECTIONS);
     
    puts("Waiting for incoming connections to client...");

    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = htons( SERVER_PORT );
  

    int i;
   	int j = 0; 
   
    int *newSock;
    int *newSock2;
	c = sizeof(struct sockaddr_in);
	pthread_t clientThread;
	pthread_t serverThread;
	pthread_t clientThreadSecond;
	

	
	while( ( clientSock[j] = accept(socketDescClient, (struct sockaddr *)&server, (socklen_t*)&c)) != NULL)
	{
		controleForKillingThreads = 0;
		if (pthread_mutex_init(&m, NULL) != 0)
		{
		    printf("\n mutex init failed\n");
		    return 1;
		}
		if (pthread_mutex_init(&m1, NULL) != 0)
		{
		    printf("\n mutex init failed\n");
		    return 1;
		}

		puts(" \n");
		puts("-----------------------------------");
		puts("----------Accept a client----------");
		socketDescServer = socket(AF_INET , SOCK_STREAM , 0);
		if (socketDescServer == -1)
		{
		    printf("Could not create socket for server");
		    return 1;
		}
		if ( connect(socketDescServer , (struct sockaddr *)&client , sizeof(client)) < 0)
	  	{
		    	perror("connect failed (to server). Error");
		  	 	return 1;
		}
	
		//threads for client
		
		if( pthread_create( &clientThread , NULL ,  ConnectionHandler , (void*)&clientSock[j]) < 0)
		{
			perror("could not create thread");
			return 1;
		}

		//threads for server
		
		if( pthread_create( &serverThread , NULL ,  ServerCommunication , (void*)&socketDescServer) < 0)
		{
			perror("could not create thread");
			return 1;
		}
		

		if (clientSock[j] < 0)
		{
		
			perror("accept failed");
			return 1;
		}
		
		
		//thread for client
		
		if( pthread_create( &clientThreadSecond , NULL ,  ClientCommunication , (void*)&clientSock[j])< 0)
		{
			perror("could not create thread");
			return 1;
		}
		
   		j++;
	
		pthread_join( clientThread, NULL);
		pthread_join( serverThread, NULL);
		pthread_join( clientThreadSecond, NULL);
		
		pthread_mutex_destroy(&m);
		pthread_mutex_destroy(&m1);
   	
	}
	

	
	
	
    return 0;
}

void *ConnectionHandler(void *socketDescPom)
{
	
	struct sockaddr_in client;
	char clientMessage[DEFAULT_BUFLEN];
	char* message;
	int c, readSize;
	c = sizeof(struct sockaddr_in);
	 
    int socketDesc = *(int*)socketDescPom;
    char logged[] ="Logged in!";

	int breakPom;
	int counter = 0; 
	while( (readSize = recv(socketDesc, clientMessage , DEFAULT_BUFLEN , 0)) > 0 )
	 {	
		CopyMessageToGlobal(clientMessageRecv ,  clientMessage);
		if(counter == 0)
		{
			counter++;
			message = CheckUser(clientMessageRecv, &breakPom); 
			write(socketDesc , message , strlen(message));
			puts("-----------------------------------");
			if(breakPom == 1){
				goto lab;
			}
			
		}else{
				  
			pthread_mutex_lock(&m); 
			controle = 1;
			pthread_cond_signal( &cond ); 
			pthread_mutex_unlock(&m);
		}
			
		memset(clientMessage,'\0', DEFAULT_BUFLEN);
		
     }

    if(readSize == 0)
    {
lab:    puts("Client disconnected");
       	controleForKillingThreads = 1;
        pthread_mutex_lock(&m);
       	pthread_cond_signal( &cond );
       	pthread_mutex_unlock(&m);
       	pthread_mutex_lock(&m);
       	pthread_cond_signal( &cond2 );
       	pthread_mutex_unlock(&m);
	
    }
    else if(readSize == -1)
    {
        perror("recv failed");

    }

    return 0;

}

void *ServerCommunication(void *socketDescPom)
{
	struct sockaddr_in client;
	int c, readSize;
	c = sizeof(struct sockaddr_in);
	 
    int socketDesc = *(int*)socketDescPom;

	pthread_mutex_lock(&m);
	while(controle == 0){
		pthread_cond_wait(&cond, &m);
		
		if(controleForKillingThreads == 1){
			controle = 0;
			close(socketDesc);
	 		return 0;
	 	}
		
		if( send(socketDesc, clientMessageRecv, strlen(clientMessageRecv), 0) < 0)
		{
					perror("Send failed");
					return 1;
		}
		puts("-----------------------------------");
		puts("--Sending to server:");	  
		printf("--%s\n",clientMessageRecv);
		memset(clientMessageRecv,'\0', DEFAULT_BUFLEN);

		if(( readSize = recv(socketDesc, serverMessageRecv , DEFAULT_BUFLEN , 0)) < 0)
		{
			puts("recv failed ");
			return 1;
		}
		puts("-----------------------------------");
		puts("--Recived from server and sent back to client :");	  
		printf("--%s\n",serverMessageRecv);
		puts("-----------------------------------");
		
		
		CopyMessageToGlobal(serverMessage , serverMessageRecv);	
			
		pthread_cond_signal( &cond2 );
		controle = 0;
		pthread_mutex_unlock(&m);
	
	
 	}
	
	close(socketDesc);
	return 0;
			
}

void *ClientCommunication(void *socketDescPom)
{
	int socketDesc = *(int*)socketDescPom;

	int i;
	int pom;
	pthread_mutex_lock(&m1);
	while(controle == 0){

		pthread_cond_wait(&cond2, &m1);

		if(controleForKillingThreads == 1)
		{
			close(socketDesc);
			return 0;
		}
	
		if( (pom = send(socketDesc , serverMessage , strlen(serverMessage) , 0)) < 0)
		{
			puts("Send failed");
			return 1;
		}
		
		pthread_mutex_unlock(&m1);
		
	
	}
}


void CopyMessageToGlobal(char* globalString, char* string){

	memset(globalString,'\0', DEFAULT_BUFLEN);
	strcpy(globalString, string); 
	memset(string,'\0', DEFAULT_BUFLEN);
}

char* CheckUser(char* user, int* pom){
	puts("-----------------------------------");
	puts("-----------------------------------");
	puts("--Checking user: ");
	printf("%s", "     ");
	puts(clientMessageRecv);
	int i;
	for (i = 0; i < NUM_BLOK; i++)
	{
		if (strcmp(user, blocked[i]) == 0)
		{	
			*pom = 1;
			puts( "The user is blocked!");
			puts(user);
			return "The user is blocked!";
			
		}
			
	}
	*pom = 0;
	puts( "--Logged in!");
	return "Logged in!";
	

}


































