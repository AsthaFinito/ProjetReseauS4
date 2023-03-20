#include <stdio.h>
#include <stdlib.h> /* pour exit */
#include <unistd.h> /* pour close */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h> /* pour htons et inet_aton */
#include <poll.h>




#define PORT IPPORT_USERRESERVED // = 5000
#define LG_MESSAGE 256
#define L 4
#define C 8
#define Max_Pixel 10
#define VERSION 1.5

int POSITION_TABLEAU = 1;
char messageEnvoi[LG_MESSAGE];/* le message de la couche Application ! */

typedef struct CASE{
	char couleur[10]; //couleur format RRRGGGBBB

}CASE;



typedef struct Users {
  int data;
  int User_Connexion_fdp;
  struct Users* suivant;
} Users;



typedef struct queue {
  Users* first;
  Users* suivant;
} queue;

void InitialisationQueue(queue* q) {
  q->first = NULL;
  q->suivant = NULL;
}

void AjoutQueue(queue* q, int data) {
  Users* NouvelleUsers = (Users*)malloc(sizeof(Users));
  NouvelleUsers->data = data;
  NouvelleUsers->suivant = NULL;
  if (q->first == NULL) {
    
    q->first = NouvelleUsers;
  } else {
    Users *parcourt = q->first;
        while (parcourt->suivant != NULL) {
            parcourt = parcourt->suivant;
        }
        parcourt->suivant = NouvelleUsers;
    }
  }


void afficherListe(queue *liste)
{
    if (liste == NULL)
    {
        printf("Erreur Liste vide \n");
    }

    Users *actuel = liste->first;

    while (actuel != NULL)
    {
        printf("%d -> ", actuel->data);
        actuel = actuel->suivant;
    }
    printf("\n");
}





void initMartice(CASE matrice[L][C]);
void setPixel(CASE matrice[L][C], int posL, int posC, char val[10]);
char *getMatrice(CASE matrice[L][C]);
char *getSize();


void MessageADecomposer(char Message[LG_MESSAGE]);
char* GetSizeCommande(char* MotAReturn);
char* GetLimitCommande(char* MotAReturn);



int main()
{

	
	int socketEcoute;
	struct sockaddr_in pointDeRencontreLocal;
	socklen_t longueurAdresse;
	
	struct sockaddr_in pointDeRencontreDistant;
	
	char messageRecu[LG_MESSAGE];/* le message de la couche Application ! */
	

	
	
	CASE matrice[L][C];
	initMartice(matrice);

	// Partie SOCKET

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	socketEcoute = socket(PF_INET, SOCK_STREAM, 0);
	/* 0 indique que l’on utilisera le protocole par défaut associé à SOCK_STREAM soit TCP */

	// Teste la valeur renvoyée par l’appel système socket()
	if(socketEcoute < 0)/* échec ? */
	{
		perror("socket");// Affiche le message d’erreur
		exit(-1);// On sort en indiquant un code erreur
	}

	printf("Socket créée avec succès ! (%d)\n", socketEcoute);
	// On prépare l’adresse d’attachement locale
	longueurAdresse =sizeof(struct sockaddr_in);
	memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
	pointDeRencontreLocal.sin_family = PF_INET;
	pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);// toutes les interfaces locales disponibles
	pointDeRencontreLocal.sin_port = htons(PORT);

	// On demande l’attachement local de la socket
	if((bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse)) < 0)
		{
			perror("bind");
			exit(-2);
		}
	printf("Socket attachée avec succès !\n");

	// On fixe la taille de la file d’attente à 5 (pour les demandes de connexion non encore traitées)
	if(listen(socketEcoute, 5) < 0){
		perror("listen");
		exit(-3);
	}

	printf("Socket placée en écoute passive ...\n");

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	
	//Initialisation du Poll avant la boucle while true
	
	queue ListeUsersPoll;
	InitialisationQueue(&ListeUsersPoll);
	

	
	
	struct pollfd ListeConnec[10];
	memset(ListeConnec, 0, sizeof(ListeConnec));
    	ListeConnec[0].fd = socketEcoute;
    	ListeConnec[0].events = POLLIN;
    	for(int i =0;i<10;i++){ //On doit syncroniser le tableau et la liste chainé donc le tableau commence à 1 et l'autre 0
    		//printf("fd: %d \n",ListeConnec[i].fd);
    		
    		ListeConnec[i+1].fd= -1;
    		ListeConnec[i+1].events=POLLIN;
    	}
    	
    	struct sockaddr_in UsersArrive;
    	socklen_t LongueurAddresseUser = sizeof(UsersArrive);
		
	// boucle d’attente de connexion : en théorie, un serveur attend indéfiniment !
	
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	while(1)
	{
		//printf("Dans le while \n"); //Partie init du poll
		if(poll(ListeConnec,100,-1)==-1){
			printf("Erreur Init du poll \n");
			
		}
		else{
		
			//printf("Poll bien initialisé \n");
			
		}
	//------------------------------------------------------ Partie Connexion
		if(ListeConnec[0].revents & POLLIN){ //Connection detectée
		
			//printf("Connexion recu \n");
			printf("Connexion établie avec %s:", inet_ntoa(pointDeRencontreDistant.sin_addr) );
			printf("%d\n",ntohs(pointDeRencontreDistant.sin_port));
			int NewUser = accept(socketEcoute, (struct sockaddr*)&UsersArrive, &LongueurAddresseUser);
			printf("Valeur du Newuser : %d \n",NewUser);
			AjoutQueue(&ListeUsersPoll,NewUser);
			afficherListe(&ListeUsersPoll);
			ListeConnec[POSITION_TABLEAU].fd=NewUser;
			POSITION_TABLEAU++;
			printf("Nouveau joueur accepté \n");
			//ajouter l'user dans la liste chainé
			
			
			
			
		}
		else{
		
			//printf("Pas de connection \n");
		}
		// c’est un appel bloquant
		

		for(int i=1;i<11;i++){
			 //printf("Action? \n");
			 
			 if (ListeConnec[i].revents & POLLIN && ListeConnec[i].fd!=-1) {
			 	printf("Action detectée pour i= %d \n",i);
			 	printf("Valeur de ListeConnec[i].fd : %d \n",ListeConnec[i].fd);
			 	int DetectionAction = read(ListeConnec[i].fd, messageRecu, LG_MESSAGE * sizeof(char));
			 	printf("Valeur de detectionAction : %d\n",DetectionAction);
			 	//printf("Message lu : %s \n",messageRecu);
			 	
			 	if(DetectionAction==-1){
			 	
			 		printf("Cas ou DetectionAction=-1 (erreur?) \n");
			 		
			 	}
			 	else if(DetectionAction==0){
			 		printf("Deconnexion d'un client \n");
			 		close(ListeConnec[i].fd);
			 		POSITION_TABLEAU--;
			 	
			 	}
			 	else{
			 		printf("Lecture d'un message \n");
			 		MessageADecomposer(messageRecu);
			 		ListeConnec[i].events = POLLOUT;
			 	}
			 }
			 else  if (ListeConnec[i].fd!=-1 && ListeConnec[i].revents & POLLOUT) {
			 	printf("C'est quel cas? \n");
			 	ListeConnec[i].events = POLLIN;
			 	int EcritureServeur = write(ListeConnec[i].fd, messageEnvoi, strlen(messageEnvoi));
				switch (EcritureServeur) {
				    case -1:
				        perror("write");
				        close(ListeConnec[i].fd);
				        
				        exit(-6);
				     
				    default:
				        printf("%s\n(%d octets)\n\n", messageEnvoi, EcritureServeur);
				}
			 }
			 
		}
		
		
		//printf("Apres le lus \n");
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		
		// On envoie des données vers le client (cf. protocole)
		
	}

	// On ferme la ressource avant de quitter
	close(socketEcoute);
	return 0;
}


void MessageADecomposer(char Message[LG_MESSAGE]){

	printf("Entrez dans MessageADecomposer \n");
	printf("Voici le message recu : %s \n",Message);
	char MessageAfficheClient[250];
	if(strcmp(Message,"/getSize\n")==0){  //done
		printf("Commande ok /getSize \n");
		
		strcpy(messageEnvoi,GetSizeCommande(MessageAfficheClient));
		
	}
	else if(strcmp(Message,"/getMatrice\n")==0){
		printf("Commande ok /getMatrice \n");
		 strcpy(messageEnvoi,getMatrice(matrice, l*c));
	
	}
	else if(strcmp(Message,"/getLimits\n")==0){ //done
		printf("Commande ok /getLimits \n");
		strcpy(messageEnvoi,GetLimitCommande(MessageAfficheClient));
		//printf(messageEnvoi);
	
	}
	else if(strcmp(Message,"/getVersion\n")==0){
		printf("Commande ok /getVersion \n");
		//printf("Version 1.2 \n");
		strcpy(messageEnvoi,"Version 1.2 \n");
	
	}
	else if(strcmp(Message,"/getWaitTime\n")==0){
		printf("Commande ok /getWaitTime \n");
	
	}
	else if(strcmp(Message,"/setPixel\n")==0){
		printf("Commande ok /setPixel \n");
	
	}
	
	
	else{
		printf("Bad command \n");
		strcpy(messageEnvoi,"Bad Command \n");
	}

}


char* GetLimitCommande(char* MotAReturn){

	printf("Voici les limites du jeu PixelWar : \n");
	char temp[150];
	char temp2[4];
	sprintf(temp2,"%d",Max_Pixel);
	strcat(temp,temp2);
	strcat(temp," est le nombre de pixel max par minute \n");
	
	strcat(MotAReturn,temp);
	return MotAReturn;
}




void initMartice(CASE matrice[L][C]){

	for (int i = 0; i < L; ++i)//parcours des lignes
	{
		for (int j = 0; j < C; ++j)//parcours des colonnes 
		{
			strcpy(matrice[i][j].couleur,"255255255");
		}
	}
}

void setPixel(CASE matrice[L][C], int posL, int posC, char val[10]){
	strcpy(matrice[posL][posC].couleur,val);
}

char* getMatrice(CASE matrice[L][C]) {
    char* matstr = NULL;
    int matstr_size = 0;
    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < C; ++j) {
            char* couleur = matrice[i][j].couleur;
            int couleur_size = strlen(couleur);
			matstr_size += couleur_size;
            matstr = (char*)realloc(matstr, matstr_size + 1);// +1 pour le \0
            strcat(matstr, couleur);   
        }
    }
    return matstr;
}

char *getSize(){//C et L paramettre de serveur
	printf("La commande est en cours d'éxecution \n");
	char *size;
	size = (char*)calloc(20, sizeof(char));
	sprintf(size, "%d", L);
	strcat(size, "\n");
	char temp[3];
	sprintf(temp, "%d", C);
	strcat(size,temp);
	return size;
}


char* GetSizeCommande(char* MotAReturn){

	printf("Voici les dimensions de la matrice : \n");
	char temp[10];
	
	char L2[4];
	sprintf(temp, "%d", L);
	sprintf(L2, "%d", C);
	
	strcat(temp,"x");
	strcat(temp,L2);
	strcat(temp,"\n");
	//printf(temp);
	strcat(MotAReturn,temp);
	return MotAReturn;
	

}

















