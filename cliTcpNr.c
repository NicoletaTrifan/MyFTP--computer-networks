/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
  Adaptari realizate de Trifan Nicoleta-Tatiana, grupa IIB3

*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <dirent.h>
#include <linux/limits.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
/* codul de eroare returnat de anumite apeluri */
extern int errno;
/* portul de conectare la server*/
int port;
char* criptare(const char* word);
int vizualizare_fisiere();
char nume[1000];
void primeste_fisier(int cl,FILE * nume_fisier, long long int dimensiune);
ssize_t total;
void verifica_exista_fisier(const char * username,const char * fisier_incarcare);
char lista_incarcare[1000]="";
char raspuns_incarcare[1000];//raspunsul clientului pentru incarcare
long long int dimensiune_fisier(const char *nume_fisier);
void stergere_fisier(const char * nume_fisier);
int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0;
  char username[128];
  char parola[128];
  char meniu[1000];
  int optiune;
  char alegere[500];
  char fisier_descarcare[100];
  char raspuns_descarcare[1000]; //raspunsul serverului la descarcare
  char raspuns_stergere[1000]; //raspunsul serverelui la stergerea 
  long long int dimensiune; //dimensiunea fisierului ce urmeaza a fi descarcat
  long long int dimensiune_incarcare; //dimensiunea fisierului care urmeaza a fi incarcat
  char * path_incarcare;
  void trimite_fisier(FILE * nume_fisier,int cl, long long int dimensiune_fisier);
  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

      while (1) {
          /* citirea mesajului */
            char mesaj[100];
            /* primesc cerere de a citi username */
            if (read(sd, mesaj,sizeof(mesaj)) < 0) {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            printf("%s",mesaj);
            scanf("%s", username);
            // trimit username
            if (write(sd, username,sizeof(username)) <= 0) 
            {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }

            // primesc cerere parola
            if (read(sd, mesaj,sizeof(mesaj)) < 0) 
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }

            printf("%s", mesaj);
            scanf("%s",parola);
            char sir[100];
            strcpy(sir,criptare(parola));
            if (write(sd,sir, 100) < 0)
            {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }

        int ok;
        if (read(sd, &ok, sizeof(int)) < 0) {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }
        if (ok == 1)  {
          printf("V-ati logat cu succes!\n");
          fflush(stdout);
          break;
        }
        else {
          printf("Username sau parola incorecta sau blacklist!\n");
          fflush(stdout);
        }
      }
    FILE *nume_fisier; //fisierul care urmeaza a fi descarcat
    FILE *fisier_i; //fisierul pt incarcare
    DIR *director_client=opendir(username);
    char *path_real;  
    char cale[100]; //calea spre fisierul din username pt incarcare
    char fisier_descarcare_ales[100]; // fisierul in care urmeaza a fi salvat ceea ce esalvat de pe server
    char fisier_incarcare[100]; //numele fisierului ales pt incarcarea pe server
    char fisier_stergere[100];
    char * cale_stergere;
    if (director_client)
    {
      printf("Aveti deja un director personal.Urmeaza sa fie deschis\n\n");
    }
      else 
        {
          if(mkdir(username,0777)==-1)
          printf("Eroare la creare director\n\n");
          else printf("A fost creat un director personal pentru dumneavoastra\n\n");
        }
    if (read(sd, meniu,sizeof(meniu)) < 0) {
        perror ("[client]Eroare la read() de la server.\n");
        return errno;
    }
    printf("%s",meniu);
    while(1){
    scanf("%d",&optiune);
    if (write(sd, &optiune, sizeof(int)) <= 0) 
    {
        perror("[client]Eroare la write() spre server.\n");
        return errno;
    }
    printf("Ati ales : ");
    int j; //optiune pentru cazul 1
    int k; //optiune pentru cazul 3
     if (write(sd, username,sizeof(username)) <= 0) 
            {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
    switch (optiune)
    {
      case 1 :
        strcpy(alegere,"Vizualizare fisierele/directoarele\n");
        printf("%s",alegere);
          j=vizualizare_fisiere();
             if (write(sd,&j, sizeof(int)) <= 0) 
             {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
             }
             if (j==1) 
             {
                 printf("In server avem urmatoarele fisiere\n");
                 if (read(sd, nume,1000) < 0)    //primesc de la server o lista de fisiere aflate in server
                 {
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                 } 
                 if (strlen(nume)==0)
                 {
                   strcpy(nume,"Directorul este gol\n");
                 }
                 printf("%s\n",nume);
                 fflush(stdout);
                 printf("Tastati urmatoarea optiune\n");
             } 
             else{
               printf("In dosarul personal detineti urmatoarele fisiere : \n");
               if (read(sd, nume,1000) < 0)    //primesc de la server o lista de fisiere aflate in dosarul personal
                 {
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                 } 
                 if (strlen(nume)==0)
                 {
                   strcpy(nume,"Directorul este gol\n");
                 }
                 printf("%s\n",nume);
                 fflush(stdout);
                 printf("Tastati urmatoarea optiune\n");
             }
             for (int i; i<strlen(nume); i++)
             {
               nume[i]='\0';
             }
        break;
      case 2 :
        strcpy(alegere,"Descarcare fisier/director\n");
        printf("%s",alegere);
        printf("Ce fisier doriti sa descarcati?\n");
        scanf("%s",fisier_descarcare);
         if (write(sd,fisier_descarcare,100) <= 0) 
             {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
             }
        if (read(sd, raspuns_descarcare,1000) < 0)    //primesc de la server o lista de fisiere aflate in dosarul personal
                 {
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                 } 
          printf("%s\n",raspuns_descarcare);
          if (strcmp(raspuns_descarcare,"Acest fisier nu exista! Incercati din nou!")==0) goto aici;
           if (read(sd, &dimensiune,sizeof(dimensiune)) < 0)    //primesc de la server o lista de fisiere aflate in dosarul personal
                 {
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                 } 
            printf("Unde doriti sa descarcati fisierul? ");
            scanf("%s",fisier_descarcare_ales);
            path_real= realpath(username,NULL);
            strcat(path_real,"/");
            strcat(path_real,fisier_descarcare_ales);
            printf("%s\n",path_real);
            nume_fisier=fopen(path_real,"wb");
            primeste_fisier(sd,nume_fisier,dimensiune);
            fclose(nume_fisier);
            for (int i; i<strlen(path_real); i++)
             {
               path_real[i]='\0';
             }
            aici:
            printf("Tastati urmatoarea optiune\n");
        break;
      case 3 :
        strcpy(alegere,"Stergere fisier/director\n");
        printf("%s",alegere);
        printf("De unde doriti sa stergeti fisierul?\n");
        k=vizualizare_fisiere();
        if (write(sd,&k, sizeof(int)) <= 0) 
             {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
             }
        if (k==2)
        {
          printf("Tastati numele fisierului pe care doriti sa il stergeti :\n");
          scanf("%s",fisier_stergere);
            cale_stergere= realpath(username,NULL);
            strcat(cale_stergere,"/");
            strcat(cale_stergere,fisier_stergere);
            printf("%s\n",cale_stergere);
            stergere_fisier(cale_stergere);
            for (int i; i<strlen(cale_stergere); i++)
             {
               cale_stergere[i]='\0';
             }
         }
        
        else
        {
          printf("Tastati numele fisierului pe care doriti sa il stergeti :\n");
          scanf("%s",fisier_stergere);
          if (write(sd,fisier_stergere, 100) <= 0) 
             {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
             }
             if (read(sd, raspuns_stergere,1000) < 0)   {
                    perror ("[client]Eroare la read() de la server.\n");
                    return errno;
                 }  
         printf("%s\n",raspuns_stergere);     
          fflush(stdout) ;
        }
        printf("Tastati urmatoarea optiune\n");
        break;
      case 4 :
        strcpy(alegere,"Incarcare  fisier/director\n\n");
        printf("%s",alegere);
        printf("Ce fisier doriti sa incarcati pe server ?\n");
        scanf("%s",fisier_incarcare);
        verifica_exista_fisier(username,fisier_incarcare);
        printf("%s\n",raspuns_incarcare);
        strcpy(cale,username);
        strcat(cale,"/");
        strcat(cale,fisier_incarcare);
        path_incarcare=realpath(cale,NULL);
        dimensiune_incarcare=dimensiune_fisier(path_incarcare); //dimensiunea fisierului pe care vreau sa o incarc si o transmit la server
         if (write(sd,raspuns_incarcare,1000) <= 0) 
             {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
             }
       if (strcmp(raspuns_incarcare,"Acest fisier nu exista! Incercati din nou!")==0) goto eticheta;
       if (write(sd,fisier_incarcare,100) <= 0) 
             {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
             }
       if (write(sd,&dimensiune_incarcare, sizeof(dimensiune_incarcare)) <= 0) 
             {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
             }
        fisier_i=fopen(cale,"rb");
        trimite_fisier(fisier_i,sd,dimensiune_incarcare);
        fclose(fisier_i);
        eticheta:
        for (int i=0; i<strlen(raspuns_incarcare);i++)
                {
                  raspuns_incarcare[i]='\0';
                }
        for (int i; i<strlen(cale); i++)
             {
               cale[i]='\0';
             }
        printf("Tastati urmatoarea optiune\n");
        break;
      case 5 :
        strcpy(alegere,"Deconectare\n");
        printf("%s",alegere);
        printf("V-ati deconectat cu succes. O zi buna!\n");
        goto label;
        break;
    }
    }
    label:
    close (sd);

    return 0;
}
char* criptare(const char* word)
 {
	char* tmp = malloc(128);
	int idx = 0;
	for (int i = 0; i < strlen(word); ++i) {
		int ascii = (int)word[i] - 1;
		if (ascii < 100) {
			// am de pus doar 2 cifre
			tmp[idx++] = '0' + (ascii / 10); // pun prima cifra
			tmp[idx++] = '0' + (ascii % 10); // pun a doua cifra
		}
		else {
            // am de pus 3 cifre
			tmp[idx++] = '0' + (ascii / 100);     // pun prima cifra
			tmp[idx++] = '0' + (ascii / 10 % 10); // pun a doua cifra
			tmp[idx++] = '0' + (ascii % 10);      // pun a treia cifra
		}
	}
	tmp[idx] = '\0';
	return tmp;
}
int vizualizare_fisiere()
{
  printf("Alegeti mediul :\n Server(1) \n Director personal(2)\n");
  int optiune_viuzalizare;
  scanf("%d",&optiune_viuzalizare);
  return  optiune_viuzalizare;
}
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
void verifica_exista_fisier(const char * username,const char * fisier_incarcare)
{
   char * path;
   char fisier[100];
   FILE * file;
   path=realpath(username,NULL);
   strcpy(fisier,path);
   strcat(fisier,"/");
   strcat(fisier,fisier_incarcare);
   file=fopen(fisier,"r");
  if (file==NULL)
  {
    strcpy(raspuns_incarcare,"Acest fisier nu exista! Incercati din nou!");
  }
  else 
  {
     strcpy(raspuns_incarcare,"Acest fisier  exista! Transferul va fi initiat in curand");
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
}//Functia dimensiune_fisier a fost preluata partial de aici : https://www.includehelp.com/c-programs/find-size-of-file.aspx
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
} //Functia trimite_fisier a fost preluata partial si adaptata conform cerintei https://github.com/omair18/Socket-Programming-in-C/blob/master/send_file.c
void stergere_fisier(const char * nume_fisier)
{
  if (remove(nume_fisier)==0)
  {
    printf("Fisier sters cu succes!\n");
  }
  else {
    printf("Eroare la stergerea fisierului!Verificati daca acesta exista\n");
  }
}
