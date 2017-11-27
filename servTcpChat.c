/* servTCPIt.c - Exemplu de server TCP iterativ
   Asteapta un nume de la clienti; intoarce clientului sirul
   "Hello nume".
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

/* portul folosit */
#define PORT 2024
#define FIFO_NAME "chat"

/* codul de eroare returnat de anumite apeluri */
extern int errno;

void serv_handle(int* clHist)
{
      char msg[100];
      while(1)
      {
        bzero(msg, 100);
        int fifod, badsd;
        mknod(FIFO_NAME, S_IFIFO | 0666, 0);
        fifod = open(FIFO_NAME, O_RDONLY);
        read(fifod, msg , 100);
        if(!memcmp("ERR:",msg,4))
        {
          badsd = atoi(&msg[4]);
          printf("EROARE SOCKET %d;", badsd);
        }
        //fflush(stdout);
        //strcat(msg, "BROADCAST\n");
        //fgets(msg, 100, stdin);
        for(int i = 0 ; clHist[i] != 0; i++)  
        {
          if (clHist[i] == badsd) continue;
          if(write(clHist[i], msg,100) <= 0)
          {
            perror("\nnu s-a trimis la client");
            exit(0);
          }
        }
      }
}

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  char msg[100];		//mesajul primit de la client 
  char msgrasp[100]=" ";        //mesaj de raspuns pentru client
  int sd;			//descriptorul de socket 
  int optval = 1;
  int clHist[20]; //lista descriptori clienti
  int crtCl = 0; //index curent lista
  pthread_t write_thread; //thread de broadcast
  signal(SIGPIPE, SIG_IGN);
  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  //int flags2 = fcntl(sd, F_GETFL, 0);
  //fcntl(sd, F_SETFL, flags2 | O_NONBLOCK);
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));
  /* pregatirea structurilor de date */
  bzero (clHist, 20);
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 5) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod iterativ clientii... */
  pthread_create(&write_thread, NULL, (void *)*serv_handle, &clHist);
  while (1)
    {
      int client;
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      client = accept (sd, (struct sockaddr *) &from, &length);
      
      // eroare la acceptarea conexiunii de la un client 
      if (client < 0)
      {
          //perror ("[server]Eroare la accept().\n");
          continue;
      }
      clHist[crtCl++]= client;
      switch(fork())
      {

        case -1:
            perror("err fork read");
            exit(-1);

        case 0:
            bzero(msg, 100);
            printf("[Server] User nou cu numele : User-%d\n",getpid());
            sprintf(msg,"Bine ai venit, User-%d\n",getpid());
            write(client, msg, 100);
            while(1)
            {
              char msg_out[100];
              bzero (msg_out, 100);
              bzero (msg, 100);
              int fifod = open(FIFO_NAME, O_WRONLY);
              //printf("Astept %d", getpid());
              fflush(stdout);  
              if(read(client, msg, 100) <= 0)
              {
                printf("Clientul %d s-a deconectat!\n", getpid());
                char msg_err[50];
                bzero(msg_err, 50);
                sprintf(msg_err,"ERR:%d",client);
                write(fifod,msg_err,sizeof(msg_err));
                perror("Status");
                exit(0);
              }
              printf("%d",client);
              sprintf(msg_out,"User-%d: %s",getpid(),msg);
              printf("%s",msg_out);
              write(fifod, msg_out, 100);
              //printf("\nUser-%d: %s",getpid(),msg);
            }
            close (client);
      }     //fork read
      //close (client);
    }       /* while */
}       /* main */
