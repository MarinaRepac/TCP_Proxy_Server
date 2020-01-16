/* 
    ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2018/2019
    Semestar:       Zimski (V)
    
    Ime fajla:      client.c
    Opis:           TCP klijent
    
    Autori:	    	Marina Repac
		     	    Aleksandar Zagorac 

    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
    ********************************************************************
*/
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015

int main(int argc , char *argv[])
{

    int sock;
    struct sockaddr_in server;
    char message1[DEFAULT_BUFLEN] , message2[DEFAULT_BUFLEN ], serverReply1[DEFAULT_BUFLEN], serverReply2[DEFAULT_BUFLEN];
    int counter;
	int read_size ;
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( DEFAULT_PORT );
 
    //Connect to proxy
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
    
    counter = 0;	
   
    while(1)
    {
        if (counter == 0)
		{	
			puts("-----------------------------------");
			printf("--Mail From : ");
			gets(message1);
			
			//Send some data
			if( send(sock , message1 , strlen(message1) , 0) < 0)
			{
				puts("Send failed");
				return 1;
			}
			//Receive a reply from the server
			
			if( recv(sock , serverReply1 , DEFAULT_BUFLEN , 0) > 0)
			{
				if(strcmp(serverReply1, "The user is blocked!") == 0){
					puts(serverReply1);
					close(sock);
					return 0;
				}
				else
					
					printf("--%s\n",serverReply1);
				
			} 
			++counter;
		} else
		{
			puts("-----------------------------------");
			memset(message2,'\0', DEFAULT_BUFLEN);
			printf("--Enter Message : ");
			gets(message2);
			
			//Send some data
			
			if( send(sock , message2 , strlen(message2) , 0) < 0)
			{
				puts("Send failed");
				return 1;
			}

			if(( read_size =recv(sock , serverReply2 , DEFAULT_BUFLEN , 0)) < 0)
			{
				puts("recv failed");
				break;
			}
			puts("-----------------------------------");
			serverReply2[read_size] = '\0';
			printf("--Server reply : %s\n", serverReply2);
			puts("-----------------------------------");
			fflush(stdout);
			
		}
         
        
    }
     
    close(sock);
	
    return 0;
}






































