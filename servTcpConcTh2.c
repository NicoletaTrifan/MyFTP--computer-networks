/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009 
   Adaptari realizate de Trifan Nicoleta-Tatiana, grupa IIB3
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include <dirent.h>
#include <linux/limits.h>
#include <sys/stat.h>
/* portul folosit */
#define PORT 2020

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
char password[100];
char meniu[1000];
int optiune,optiune_vizualizare,optiune_stergere;
char alegere[500];
char lista[1000]="";
char lista_descarcare[1000]="";
int logare(int cl);
void listare_fisiere_server(const char * nume_fisier, int cl);
char nume_fisier[100];
int logare_user(const char* username, const char* parola);
void descarcare_fisiere_server(const char * nume_fisier,const char* fisier_descarcare);
char raspuns_descarcare[100];
char raspuns_stergere[100];
long long int dimensiune_fisier(const char *nume_fisier);
void trimite_fisier(FILE * nume_fisier,int cl, long long int dimensiune_fisier);
void primeste_fisier(int cl, FILE * nume_fisier, long long int dimensiune_fisier);
void stergere_fisier(const char * nume_fisier);
ssize_t total;
int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
	int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i=0;


  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
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
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam conectarea clientilor la portul %d\n",PORT);
      fflush (stdout);
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;

	pthread_create(&th[i], NULL, &treat, td);	      
				
	} 
};				

static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
  		
};
void raspunde(void *arg)
{
  int nr,i=0;
  int flag=1;
	struct thData tdL; 
  char server_file[100];
  strcpy(server_file,"server_file_system");
	tdL= *((struct thData*)arg);
  strcpy(nume_fisier,"server_file_system");
  char username[128];
  char fisier_descarcare[100];
  char * path_real;
  char* path_incarcare;
  char cale_incarcare[100]; 
  long long int dimensiune;
  long long int dimensiune_incarcare;
  char fisier_incarcare[100];
  char raspuns_incarcare[1000];
  FILE * fisier_d; //fisierul descarcat
  FILE * fisier_i; //fisierul incarcat
  char fisier_stergere[100];
  char * path_stergere;
  char cale_stergere[100];
 //logare prin bucla infinita
    if (logare(tdL.cl)) {
    
          strcpy(meniu,"Alegeti urmatoarea optiune pe care doriti sa o executati:\n");
          strcat(meniu,"Vizualizeaza fisierele/directoarele (1)\n");
          strcat(meniu,"Descarca fisier/director (2)\n");
          strcat(meniu,"Sterge fisier/director (3)\n");
          strcat(meniu,"Incarca fisier/director (4)\n");
          strcat(meniu,"Deconectare (5)\n");
          strcat(meniu,"Pentru a putea executa una din optiunile de mai sus, tastati numarul corespunzator din paranteza\n");
          if (write (tdL.cl, meniu, 1000) <= 0)
          {
          printf("[Thread %d] ",tdL.idThread);
          perror ("[Thread]Eroare la write() catre client.\n");
          }
          while(flag){
            if (read(tdL.cl, &optiune, sizeof(int)) <= 0) 
          {
          printf("[Thread %d] ",tdL.idThread);
          perror ("[Thread]Eroare la read() catre client.\n");
          }
          printf("[Thread %d] Optiunea aleasa de client este : ",tdL.idThread);
          if (read(tdL.cl, username, sizeof(username)) <= 0) 
          {
          printf("[Thread %d] ",tdL.idThread);
          perror ("[Thread]Eroare la read() catre client.\n");
          }
            switch (optiune)
            {
            case 1: 
              strcpy(alegere,"Vizualizare fisiere/directoare\n");
              printf("%s",alegere);
              if (read(tdL.cl, &optiune_vizualizare, sizeof(int)) <= 0) 
              {
                 printf("[Thread %d] ",tdL.idThread);
                 perror ("[Thread]Eroare la read() catre client.\n");
              }
              if (optiune_vizualizare==1) {
                printf("Transmitem clientului lista fisierelor de pe server\n"); fflush(stdout);
                listare_fisiere_server(nume_fisier,tdL.cl);  printf("%s\n",lista);
                if (write (tdL.cl,lista, 1000) <= 0)
                 {
                  printf("[Thread %d] ",tdL.idThread);
                  perror ("[Thread]Eroare la write() catre client.\n");
                 }
                for (int i=0; i<strlen(lista);i++)
                {
                  lista[i]='\0';
                }}
              else{
                printf("Transmitem clientului lista fisierelor din dosarul personal "); fflush(stdout);
                listare_fisiere_server(username,tdL.cl);  printf("%s\n",lista);
                 if (write (tdL.cl,lista, 1000) <= 0)
                 {
                  printf("[Thread %d] ",tdL.idThread);
                  perror ("[Thread]Eroare la write() catre client.\n");
                 }
                for (int i=0; i<strlen(lista);i++)
                {
                  lista[i]='\0';
                }}
                break;
            case 2:
                strcpy(alegere,"Descarca fisier/director\n");
                printf("%s",alegere);
                printf("Clientul doreste sa descarce urmatorul fisier : ");
                if (read(tdL.cl,fisier_descarcare , 100) <= 0) 
              {
                 printf("[Thread %d] ",tdL.idThread);
                 perror ("[Thread]Eroare la read() catre client.\n");
              }
                printf("%s\n",fisier_descarcare);
                descarcare_fisiere_server(server_file,fisier_descarcare);
                printf("%s\n",raspuns_descarcare);
                if (write (tdL.cl,raspuns_descarcare, 1000) <= 0)
                 {
                  printf("[Thread %d] ",tdL.idThread);
                  perror ("[Thread]Eroare la write() catre client.\n");
                 }
                 if (strcmp(raspuns_descarcare,"Acest fisier nu exista! Incercati din nou!")==0) goto aici;
                 path_real=realpath(fisier_descarcare,NULL);
                 dimensiune=dimensiune_fisier(path_real);
                 if (write (tdL.cl,&dimensiune, sizeof(dimensiune)) <= 0)
                 {
                  printf("[Thread %d] ",tdL.idThread);
                  perror ("[Thread]Eroare la write() catre client.\n");
                 }
                fisier_d=fopen(path_real,"rb");
                trimite_fisier(fisier_d,tdL.cl,dimensiune);
                fclose(fisier_d);
                
               for (int i=0; i<strlen(raspuns_descarcare);i++)
                {
                  raspuns_descarcare[i]='\0';
                }
                aici:
                
                break;
            case 3:
                strcpy(alegere,"Sterge fisier/director\n");
                printf("%s",alegere);
                if (read(tdL.cl, &optiune_stergere, sizeof(int)) <= 0) 
                {
                 printf("[Thread %d] ",tdL.idThread);
                 perror ("[Thread]Eroare la read() catre client.\n");
                }
                if (optiune_stergere==1)
                {
                  printf("Clientul doreste stergerea unui fisier de pe server\n");
                  if (read(tdL.cl, fisier_stergere, 100) <= 0) 
                  {
                   printf("[Thread %d] ",tdL.idThread);
                   perror ("[Thread]Eroare la read() catre client.\n");
                  }
                  strcpy(cale_stergere,server_file);
                  strcat(cale_stergere,"/");
                  strcat(cale_stergere,fisier_stergere);
                  path_stergere=realpath(cale_stergere,NULL);
                  stergere_fisier(path_stergere);
                  printf("%s\n",raspuns_stergere);
                  fflush(stdout);
                  if (write (tdL.cl,raspuns_stergere, 1000) <= 0)
                 {
                  printf("[Thread %d] ",tdL.idThread);
                  perror ("[Thread]Eroare la write() catre client.\n");
                 }
                }
                else
                {
                  printf("Clientul doreste stergerea unui fisier din dosar personal\n");
                }
                  for (int i=0; i<strlen(raspuns_stergere);i++)
                {
                  raspuns_stergere[i]='\0';
                }
                break;
            case 4:
                strcpy(alegere,"Incarca fisier/director\n");
                printf("%s",alegere);
                if (read(tdL.cl,raspuns_incarcare , 1000) <= 0) 
                {
                 printf("[Thread %d] ",tdL.idThread);
                 perror ("[Thread]Eroare la read() catre client.\n");
                }
                if (strcmp(raspuns_incarcare,"Acest fisier nu exista! Incercati din nou!")==0) goto eticheta;
                if (read(tdL.cl,fisier_incarcare , 100) <= 0) 
                {
                 printf("[Thread %d] ",tdL.idThread);
                 perror ("[Thread]Eroare la read() catre client.\n");
                }
                if (read(tdL.cl,&dimensiune_incarcare, 100) <= 0) 
                {
                 printf("[Thread %d] ",tdL.idThread);
                 perror ("[Thread]Eroare la read() catre client.\n");
               }
                printf("Clientul doreste sa incarce  %s cu dimensiunea %lld",fisier_incarcare,dimensiune_incarcare);
                fflush(stdout);
                //construiesc path pt fisierul ce urmeaza a fi incarcat
                path_incarcare=realpath(server_file,NULL);
                strcat(path_incarcare,"/");
                strcat(path_incarcare,fisier_incarcare);
                fisier_i=fopen(path_incarcare,"wb");
                primeste_fisier(tdL.cl,fisier_i,dimensiune_incarcare);
                fclose(fisier_i);
                eticheta:
                break;
            case 5:
              strcpy(alegere,"Deconectare utilizator\n");
              printf("%s",alegere); 
              flag=0;
              break;
          }
          
        }
       
	    }
}


int logare(int cl) {
    char mesaj[64];
    char username[128];
    char password[128];

     while (1) {
         strcpy(mesaj, "Introduceti username-ul: ");
         // solicit username
         if (write(cl, mesaj, sizeof(mesaj)) < 0) {
           perror("Eroare la write la server -> client\n");
           return -1;
         }

         // primesc username
         if (read(cl, username, sizeof(username)) < 0) {
           perror("Eroare la read server -> client\n");
           return -1;
         }
        
        strcpy(mesaj, "Introduceti parola: ");
         // solicit parola
         if (write(cl, mesaj, sizeof(mesaj)) < 0) {
           perror("Eroare la write la server -> client\n");
           return -1;
         }

         // primesc parola
         if (read(cl, password, sizeof(password)) < 0) {
           perror("Eroare la read server -> client\n");
           return -1;
         }
        
        int ok = logare_user(username, password);
        if (ok == 1) {
          // trimit mesaj la client sa opresc bucla
          if (write(cl, &ok, sizeof(int)) < 0) {
            perror("Eroare la write la server -> client\n");
            return -1;
          }
          return 1;
        }
        else {
          // trimit 0 la client pentru a continua bucla
          if (write(cl, &ok, sizeof(int)) < 0) {
            perror("Eroare la write la server -> client\n");
            return -1;
          }

          printf("Username gresit sau parola gresita sau blacklist!\n");
          fflush(stdout);
        }
     }
    return 0;
}


int logare_user(const char* username, const char* parola) {
    printf("username: %s\n parola: %s\n", username, parola);
    fflush(stdout);
    sqlite3 *db;
    char *err_msg = 0;
    char mesaj2[100];
    int baza = sqlite3_open("users.db", &db);
    
    if (baza != SQLITE_OK) {
        fprintf(stderr, "Eroare la deschiderea bazei de date : %s\n", 
        sqlite3_errmsg(db));
        sqlite3_close(db);
    }

		  sqlite3_stmt *res; 
      char  sql[500];
      strcpy(sql,"SELECT * FROM utilizatori WHERE USERNAME='");
      strcat(sql, username);
      strcat(sql,"' AND PASSWORD='");
      strcat(sql,parola);
      strcat(sql,"' AND LIST='");
      strcat(sql,"whitelist';");
      printf("%s ", sql);
      baza = sqlite3_prepare_v2(db, sql, -1, &res, 0);

     if (baza != SQLITE_OK)
      {       
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
      }    

     int step = sqlite3_step(res);
     if (step == SQLITE_ROW)
      {
         sqlite3_finalize(res);
         sqlite3_close(db);
         return 1;
      } 
    sqlite3_finalize(res);
    sqlite3_close(db);
     return 0;
}
void listare_fisiere_server(const char * nume_fisier, int cl)
{
    DIR *dir;
    struct dirent *de;
    char nume[PATH_MAX];
    dir=opendir(nume_fisier);
    if (!dir)
    {
      return; //fisierul nu este director
      strcpy(lista,nume_fisier);
    }
    if (NULL==dir)
    {
      fprintf(stderr,"Eroare deschidere director %s .\t",nume_fisier);  perror("Cauza este");
      return;
    }
    else
    {
        while(NULL != (de = readdir(dir)) )
        {
            if( strcmp(de->d_name,".") && strcmp(de->d_name,"..") )  
            {
               strcpy(nume,nume_fisier);
               strcat(nume,"/");
               strcat(nume,de->d_name);
               strcat(lista,nume);
               strcat(lista,"\n");
               listare_fisiere_server(nume,cl); 
            }
        }

   closedir(dir);
   }
}


void descarcare_fisiere_server(const char * nume_fisier, const char * fisier_descarcare)
{
   char * path;
   char fisier[100];
   FILE * file;
   path=realpath(fisier_descarcare,NULL);
   strcpy(fisier,path);
   //strcat(fisier,"/");
   //strcat(fisier,fisier_descarcare);
   file=fopen(fisier,"r");
  if (file==NULL)
  {
    strcpy(raspuns_descarcare,"Acest fisier nu exista! Incercati din nou!");
  }
  else 
  {
     strcpy(raspuns_descarcare,"Acest fisier  exista! Transferul va fi initiat in curand");
  }
}
long long int dimensiune_fisier(const char *nume_fisier) {
    struct stat st; 
    long long int dimensiune_return;  
    if (stat(nume_fisier,&st)==0) {
        dimensiune_return=st.st_size;
        return dimensiune_return;
    }
    else  return -1;
} //Functia dimensiune_fisier a fost preluata partial de aici : https://www.includehelp.com/c-programs/find-size-of-file.aspx
//si adaptata de mine conform cerintei
void trimite_fisier(FILE * nume_fisier,int cl, long long int dimensiune_fisier)
{
  int citit=0;
  char linia_transmitere[4096]={0};
  while(dimensiune_fisier>0)
  {
    citit=fread(linia_transmitere,sizeof(char),4096,nume_fisier);
    total=total+citit;
    dimensiune_fisier-=citit;
    if (citit!= 4096 && ferror(nume_fisier))
    {
      perror("Eroare la citirea fisierului!");
      exit(1);
    }
    if (send(cl,linia_transmitere,citit,0)==-1)
    {
      perror("Fisierul nu poate fi transmis!");
      exit(1);
    }
    for (int i=0; i<4096; i++)
    {
      linia_transmitere[i]='\0';
    }
  }
} //functia a fost preluata partial si adaptata https://github.com/omair18/Socket-Programming-in-C/blob/master/send_file.c
void primeste_fisier(int cl, FILE * nume_fisier, long long int dimensiune_fisier)
{
  ssize_t primit;
  char buffer[4096]={0};
  while(dimensiune_fisier>0)
  {
    primit=recv(cl,buffer,4096,0);
    total=total+primit;
    dimensiune_fisier-=primit;
    if(primit==-1)
    {
      perror("Eroare la primirea fisierului!");
      exit(1);
    }
    if (primit==0)
    {
      break;
    }
     if (fwrite(buffer, sizeof(char), primit, nume_fisier) != primit) {
            perror("Eroare la scriere!");
            return;
        }
        printf("Total transferat: %ld bytes ", total);
        fflush(stdout);
        for (int i=0; i<4096; i++)
    {
      buffer[i]='\0';
    }
    }
    printf("\nFisier primit cu succes!\n");
    fflush(stdout);
} //functia a fost preluata partial si adaptata conform cerintei https://github.com/omair18/Socket-Programming-in-C/blob/master/receive_file.c
void stergere_fisier(const char * nume_fisier)
{
  if (remove(nume_fisier)==0)
  {
    strcpy(raspuns_stergere,"Fisier sters cu succes!\n");
  }
  else {
    strcpy(raspuns_stergere,"Eroare la stergerea fisierului!Verificati daca acesta exista\n");
  }
}