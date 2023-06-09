	#include <stdio.h>
#include <stdlib.h> /* pour exit */
#include <unistd.h> /* pour close */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h> /* pour htons et inet_aton */
#include <poll.h>
#include<time.h>
#include <math.h>




#define PORT IPPORT_USERRESERVED // = 5000
#define LG_MESSAGE 2000
#define L 4
#define C 8
#define Max_Pixel 10
#define VERSION 1.5

int POSITION_TABLEAU = 1;
int modulo=0;
int ancien_modulo=0;
int tableau_coup_Restant[10]={1,1,1,1,1,1,1,1,1,1};
char messageEnvoi[2000];/* le message de la couche Application ! */
char TableauDecoupage[3][500];
char TableauDecoupagePosition[2][10];
char base46_map[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'}; // Base64 transformer
                     
int b64invs[] = { 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51 };
	
	const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


typedef struct CASE{ //Structure présente dans la matrice
	char couleur[25]; 

}CASE;

CASE MatriceDeJeu[L][C]; //Matrice avec les pixels

typedef struct Users {  //Structure des "clients"
  int data;
  int User_Connexion_fdp;
  struct Users* suivant;
  int coup_restant;
} Users;



typedef struct queue { //Liste chainée
  Users* first;
  
} queue;


/* Fonctions pour gerer la liste chainée */
void InitialisationQueue(queue* q) {
  q->first = NULL;

}

void AjoutQueue(queue* q, int data,int coup_restant) {
  Users* NouvelleUsers = (Users*)malloc(sizeof(Users));
  if (NouvelleUsers == NULL) {
        printf("Erreur: Impossible d'allouer de la mémoire.\n");
        return NULL;
    }
  NouvelleUsers->data = data;
  NouvelleUsers->suivant = NULL;
  NouvelleUsers->coup_restant = coup_restant;
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

/* Fonctions pour gerer la liste chainée */



void initMatrice(CASE matrice[L][C]);
void setPixel(CASE matrice[L][C], int posL, int posC, char val[10]);




void MessageADecomposer(char Message[LG_MESSAGE]);
char* GetSizeCommande(char MotAReturn[256]);
char* GetLimitCommande(char MotAReturn[256]);
void AfficheMatriceDeJeu(CASE Matrice[L][C]);
char* ReturnMatriceDeJeu(CASE Matrice[L][C],char TestMatrice[2000]);
void SetPixelCommande(CASE Matrice[L][C], int ChoixLigne, int ChoixColonne, char NewCouleur[10]);
int DecoupeMessageSetPixel(char* MessageComplet);
void TestDecoupageMessage();
void TestPixelOutOfBound(char MessagePixel[20],char MessagePixel2[20]);
void TestBadColor(char MessageColor[20],int Ligne,int Colonne);







int TestChiffreDansBits(char* IN);

int char_in_array(char c, const char* array);
void ConvertirB64ToBin(char* input);
void int_to_binary(unsigned int n, char* binary);
void SplitCharEn3(char* input_str, char* output_str_1, char* output_str_2, char* output_str_3);
int TransformeBinaire(char* input);
int testValeurRGB(int test);


void TestLancementServeur(int argc, char *argv[]){ //Fonction de test pour lancer le serveur avec des arguments

	
	 int opt;
    int port=0, c=0, l=0, maxClients=0;


    if (argc != 7) {
        fprintf(stderr, "Usage: %s -p PORT -s LxH -l MAX_CLIENTS\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "p:s:l:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                sscanf(optarg, "%dx%d", &c, &l);
                break;
            case 'l':
                maxClients = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p PORT] [-s LxH] [-l MAX_CLIENTS]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

}
queue ListeUsersPoll;
int sec_left;
int actionUsers;
int statut=0;

int main(int argc, char *argv[])
{
	
	
		    
	/*Lancement du serveur*/
	TestLancementServeur(argc,argv);
	
	int socketEcoute;
	struct sockaddr_in pointDeRencontreLocal;
	socklen_t longueurAdresse;
	
	struct sockaddr_in pointDeRencontreDistant;
	
	char messageRecu[LG_MESSAGE];/* le message de la couche Application ! */
	

	
	

	

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
	
	
	InitialisationQueue(&ListeUsersPoll);
	

	
	
	struct pollfd ListeConnec[10];
	memset(ListeConnec, 0, sizeof(ListeConnec)); // On crée le tableau d'users
    	ListeConnec[0].fd = socketEcoute;
    	ListeConnec[0].events = POLLIN;
    	for(int i =0;i<10;i++){ //On doit syncroniser le tableau et la liste chainé donc le tableau commence à 1 et l'autre 0
    		//printf("fd: %d \n",ListeConnec[i].fd);
    		
    		ListeConnec[i+1].fd= -1;
    		ListeConnec[i+1].events=POLLIN;
    	}
    	
    	struct sockaddr_in UsersArrive;
    	socklen_t LongueurAddresseUser = sizeof(UsersArrive);
		
	
	initMatrice(MatriceDeJeu);
	// boucle d’attente de connexion : en théorie, un serveur attend indéfiniment !
	Users* test=ListeUsersPoll.first;
	
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	while(1)
	{
	
		
		time_t now = time(NULL); // obtenir l'heure actuelle en secondes depuis l'époque UNIX
		 struct tm * timeinfo = localtime(&now); // convertir en structure tm contenant les informations de temps	
		    sec_left = 60 - timeinfo->tm_sec;
		    statut=0;
		   // printf("Seconde restantes : %d \n",sec_left);
		    if(sec_left==1){
		    	statut=1;
		    	
		    	
		    }
		    if(statut==1){
		    	for(int i=0;i<10;i++){
		    	
		    		tableau_coup_Restant[i]=1; //Remise à zéro des coups restants pour chaque minutes
		    	}
		    }	
		//printf("Dans le while \n"); //Partie init du poll
		if(poll(ListeConnec,10,-1)==-1){
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
			
			AjoutQueue(&ListeUsersPoll,NewUser,1);
			afficherListe(&ListeUsersPoll);
			//AfficheStatUser(test);
			
			ListeConnec[POSITION_TABLEAU].fd=NewUser;
			POSITION_TABLEAU++;
			printf("Nouveau joueur accepté \n");
			//ajouter l'user dans la liste chainé
			
			
			
			
		}
		
	
		

		for(int i=1;i<11;i++){
			 //printf("Action? \n");
			 
		  // printf("Il reste %d secondes avant la prochaine minute.\n", sec_left);
			 if (ListeConnec[i].revents & POLLIN && ListeConnec[i].fd!=-1) {
			 	printf("Action detectée pour i= %d \n",i);
			 	actionUsers=i;
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
			 		//close(ListeConnec[i].fd);
			 		printf("Lecture d'un message \n");
			 		messageRecu[DetectionAction]='\0';
			 		MessageADecomposer(messageRecu);
			 		ListeConnec[i].events = POLLOUT;
			 	}
			 }
			 else  if (ListeConnec[i].fd!=-1 && ListeConnec[i].revents & POLLOUT) {
			 	//printf("C'est quel cas? \n");
			 	ListeConnec[i].events = POLLIN;
			 	printf(" strlen(messageEnvoi) == %d \n",strlen(messageEnvoi));
			 	//printf("Voici la matrice avant L'ecriture serveur : \n");
			 	//AfficheMatriceDeJeu(MatriceDeJeu);
			 	printf("Affichage de messageEnvoi : \n");
			 	printf("'%s'",messageEnvoi);
			 	printf("\n");
			 	int EcritureServeur = write(ListeConnec[i].fd, messageEnvoi, 100);
			 	
				switch (EcritureServeur) {
				    case -1:
				        printf("Cannot write : (Ecriture serveur = -1) \n");
				       // close(ListeConnec[i].fd);
				        perror("write");
				        //exit(-6);
				        
				        break;
				    case 0:
				    	printf("Bad file descriptor? \n");
				    	exit(-1);
				    	break;
				    default:
				        //printf("%s\n(%d octets)\n\n", messageEnvoi, EcritureServeur);
				        break;
				}
			 }
			 
			 
		}
		
		
		//printf("Apres le lus \n");
		
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		
		// On envoie des données vers le client (cf. protocole)
	//strcpy(messageEnvoi,"");	
	}
	//printf("Ceci est un test \n");
	// On ferme la ressource avant de quitter
	//close(socketEcoute);
	return 0;
}

/* int DecoupeMessageSetPixel(char* MessageComplet)

params : Char* MessageComplet -> la chaine de caractère à découper

Return : int -> valeur de test 

But : Découpe le message dans le cas du setPixel pour mieux gerer les différents paramètres
*/

int DecoupeMessageSetPixel(char* MessageComplet){

	char Decoupage[2]=" ";
	char *token;
	token=strtok(MessageComplet,Decoupage);
	int compteur =0;
	
	strcpy(TableauDecoupage[compteur],token);
	while(token!=NULL){
	
		
		if(compteur>=3){
		
			printf("Erreur pour le /setPixel (trop d'argument ) \n"); //2 paramètres max
			strcpy(messageEnvoi,"10 Bad Command");
			token=NULL;
			return -1;
			
			
			
		}
		else{
		
			printf("Valeur du token = %s\n",token);
			token = strtok(NULL, Decoupage);
			compteur++;
			if(token!=NULL){
			
				strcpy(TableauDecoupage[compteur],token);
			}
			
			
			//strcpy(messageEnvoi,"00 OK");
			
		}
		
		
	}
	for(int i=0;i<3;i++){
	
		printf("Lecture du tableau %s \n",TableauDecoupage[i]);
	}
	return 1;
	
}
/* int TestChiffreDansBits(char* IN)

params : Char* MessageComplet -> la chaine de caractère représentant un bit

Return : int -> valeur de test 

But : Fonction de debug utile pour tester le décodage base64
*/

int TestChiffreDansBits(char* IN){

	for(int i=0;i<9;i++){
	
		printf("Valeur test en int : %d \n",(int)IN[i]);
		if((int)IN[i]>=48 && (int)IN[i]<=57){
		
			printf("ok %d \n",i);
			printf("Valeur test : %c \n",IN[i]);
			
			
		}
		else{
			printf("Erreur dans le test , pas un chiffre \n");
			strcpy(messageEnvoi,"12 Bad Color");
			printf("Après le strcpy \n");
			return -1;
		}
	}
	return 0;
}

/* void TestBadColor(char MessageColor[20],int Ligne,int Colonne)

params : char MessageColor[20] -> La partie du string contenant l'information de la couleur
	 int Ligne,int Colonne -> les coordonées en int de la place du pixel

Return : void

But : Fonction de Test pour la couleur (case 12 BadColor)
*/

void TestBadColor(char MessageColor[20],int Ligne,int Colonne){

	printf("La couleur est : %s \n",MessageColor);
	int valeur_Bin=0;
	char CHAINE_ENCODE[40];
	char CHAINE_MATRICE[40];
	char CHAINE_DECODE[40];
	char test;
	char test2;
	char test12[30]="";
	char test22[30]="";
	char test32[30]="";
	MessageColor[strlen(MessageColor) - 1] = '\0';
	printf("Valeur du strlen :%d \n",strlen(MessageColor));
	
	
	
		
		
		
			
			
			strcpy(CHAINE_ENCODE,MessageColor);// encode en base64 
			strcpy(CHAINE_MATRICE,MessageColor);
			
			printf(" [%s] encoded \n",CHAINE_ENCODE); //j'affiche
		
			ConvertirB64ToBin(CHAINE_ENCODE); //On convertit en binaire
			SplitCharEn3(CHAINE_ENCODE,test12,test22,test32);//On découpe la char* en 3 parties pour récuperer les 3*8 bits
			printf("Valeur des merdes : [%s] [%s] [%s] \n",test12,test22,test32);
			
			int valeur_Bin1 = TransformeBinaire(test12); //Conversion en int pour tester chaque valeur
			printf("Valeur_bin1= [%d] \n",valeur_Bin1);
			int test_chiffre1=testValeurRGB(valeur_Bin1);
			int valeur_Bin2 = TransformeBinaire(test22);
			printf("Valeur_bin2= [%d] \n",valeur_Bin2);
			int test_chiffre2=testValeurRGB(valeur_Bin2);
			int valeur_Bin3 = TransformeBinaire(test32);
			printf("Valeur_bin3= [%d] \n",valeur_Bin3);
			int test_chiffre3=testValeurRGB(valeur_Bin3);
			
		
			if(test_chiffre1==-1 || test_chiffre2==-1 || test_chiffre3==-1){ //Cas ou un des test n'est pas concluants
			
				strcpy(messageEnvoi,"12 Bad Color");
			}
			else{
				
				Users *ParcourtDeLaQueue;
				int compteur=1;
				printf("Valeur de actionUser : %d \n",actionUsers);
				
				//on cherche qui a fait l'action
				
				if(actionUsers==compteur){
					printf("C'est le premier client qui a fait l'action \n");
					ParcourtDeLaQueue=ListeUsersPoll.first;
				}
				else{
					printf("On recherche quel client a fait l'action \n");
					for(int i = 1; i < actionUsers && ParcourtDeLaQueue->suivant != NULL; i++) {
							printf("Recherche de place \n");
						    ParcourtDeLaQueue = ParcourtDeLaQueue->suivant;
						    
						  }
				}
				printf("Fin de la recherche \n");
				if(ParcourtDeLaQueue==NULL){
				
					printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
				}else{
				
					printf("All good \n");
				}
				//AfficheStatUser(ParcourtDeLaQueue);
				if(tableau_coup_Restant[actionUsers]==0){
					printf("Impossible de poser le pixel \n");
					strcpy(messageEnvoi,"20 OutOfQuota");
				}
				else{
					printf("Coup possible \n");
					printf("Valeur de ligne et de colonne : [%d] [%d] \n",Ligne,Colonne);
					strcpy(MatriceDeJeu[Ligne][Colonne].couleur,CHAINE_MATRICE);//Modification de la matrice en fonction des params
					printf("Après modif de matrice \n");
					strcpy(messageEnvoi,"00 OK");
					//ParcourtDeLaQueue->coup_restant=ParcourtDeLaQueue->coup_restant-1;
					tableau_coup_Restant[actionUsers]=0;//Modification des coups
					
				}
				//on regarde si il a assez de pixel
				//si oui on modifie le pixel
				//si non on renvoie 20 outOfQuota
				
				
			}
			
		
		
		
	
	


}

/* int testValeurRGB(int test)

params : int test -> valeur du binaire décodé précedemment

Return :int -> valeur de test

But : Vérifie que le int est bien compris entre 0 et 255
*/


int testValeurRGB(int test){

	if(test<0 || test>255){
	
		printf("Erreur dans la couleur \n");
		return -1;
	}else{
		printf("Couleur valide \n");
		return 1;
	}
}

/* int TransformeBinaire(char* input)

params : char* input -> string représentant un bit

Return :int -> valeur en int du char*

But : Parcourt le bit du poids le plus fort au poids le plus faible pour connaitre sa valeur
*/
int TransformeBinaire(char* input){
	unsigned int valeur=0;
	for(int i=0;i<strlen(input);i++){
		//printf("Valeur de input[i]-48  : [%d] \n",input[i]-48);
		//int test=pow(2,i);
		valeur=valeur+(pow(2,strlen(input)-1-i)*(input[i]-48));
		//printf("debug %d \n",pow(2,2));
	}
	printf("Valeur : [%d] \n",valeur);
	return valeur;
}

/* int char_in_array(char c, const char* array) 

params : char c -> carractère a tester
	 const char* array -> tout les caractères de test possible

Return :int -> valeur en int d'un index

But : Trouver la position d'un char base64 dans son tableau pour connaitre sa valeur en int
*/

int char_in_array(char c, const char* array) {
    size_t len = strlen(array);
    int index=0;
    for (size_t i = 0; i < len; i++) {
        if (array[i] == c) {
            return index;
        }
        else{
        	index++;
        }
    }
    return -1;
}
/* void ConvertirB64ToBin(char* input)

params : char* input -> char* en base64 à convertir
	 

Return :void 

But : convertir une chaine de carac base64 en binaire
*/


void ConvertirB64ToBin(char* input){
	int index=0;
	int test;
	char test2[40];
	char reconstitution[80];
	for(int i=0; i<strlen(input);i++){
		index=char_in_array(input[i], base64_chars);
		printf("Valeur trouvé : [%d] \n",index); //Ensuite convertir en binaire
		//test=int_to_binary(index);
		//printf("Valeur de test : [%d] \n",test);
		//strcpy(test2,int_to_binary(index));
		int_to_binary(index,test2);
		if(i==0){
		
		strcpy(reconstitution,test2);
		}
		else{
		strcat(reconstitution,test2);
		}
		printf("Fin de conversion ? [%s] \n",test2);
	}
	printf("Voila la chaine entière : [%s] \n",reconstitution);
	strcpy(input,reconstitution);
}

/* void SplitCharEn3(char* input_str, char* output_str_1, char* output_str_2, char* output_str_3) 

params : char* input_str -> char* contenant plusieurs informations
	 char* output_str_1 -> valeur de retour1
	 char* output_str_2 -> valeur de retour2
	 char* output_str_3 -> valeur de retour3
	 

Return :void 

But : Découper une chaine de caractères en 3 parties égales
*/
void SplitCharEn3(char* input_str, char* output_str_1, char* output_str_2, char* output_str_3) {
    int len = strlen(input_str);
    int part_len = len / 3;
    strncpy(output_str_1, input_str, part_len);
    strncpy(output_str_2, input_str + part_len, part_len);
    strncpy(output_str_3, input_str + 2 * part_len, part_len);
    printf("Valeur des merdes : [%s] [%s] [%s] \n",output_str_1,output_str_2,output_str_3);
}

/* void int_to_binary(unsigned int n, char* binary) 

params : char* input_str -> char* valeur de retour en binaire
	 unsigned int n -> valeur en int d'un char base64
	
	 

Return :void 

But : Convertir un int en binaire (char*)
*/

void int_to_binary(unsigned int n, char* binary) {
    int i;
    for (i = 5; i >= 0; i--) {
        binary[i] = (n % 2) + '0';
        n /= 2;
    }
    binary[6] = '\0';
}








/* void TestPixelOutOfBound(char MessagePixel[20],char MessagePixel2[20])

params : char MessagePixel[20] -> Message contenant la position des coordonnées
	 char MessagePixel2[20] -> char* valeur de retour 
	
	 

Return :void 

But : Tester les coordonées rentrées par le client
*/


void TestPixelOutOfBound(char MessagePixel[20],char MessagePixel2[20]){

	char *token;
	char Separateur[2]="x";
	int compteur=0;
	token=strtok(MessagePixel,Separateur);
	strcpy(TableauDecoupagePosition[compteur],token);
	while(token!=NULL){
	
		compteur++;
		
		token=strtok(NULL,Separateur);
		if(token!=NULL){
			strcpy(TableauDecoupagePosition[compteur],token);
		}
		
	}
	printf("Lecture du tableau mais que Pixel %s et %s  \n",TableauDecoupagePosition[0],TableauDecoupagePosition[1]);
	if(atoi(TableauDecoupagePosition[0])<0 || atoi(TableauDecoupagePosition[0])>L){
	
		printf("Erreur dans les coo (ligne) \n");
		strcpy(messageEnvoi,"12 PixelOutOfBond \n");
		return;
		
	}
	else if(atoi(TableauDecoupagePosition[1])<0 || atoi(TableauDecoupagePosition[1])>C){
		printf("Erreur dans les coo (colonne) \n");
		strcpy(messageEnvoi,"12 PixelOutOfBond \n");
		return;
	}
	else{
		printf("Coordonnées Valide \n");
		TestBadColor(MessagePixel2,atoi(TableauDecoupagePosition[0]),atoi(TableauDecoupagePosition[1]));
	}

}

/* void TestDecoupageMessage()

params : 
	
	 

Return :void 

But : Découper le cas du setPixel en 2 partie pour mieux les tester
*/
void TestDecoupageMessage(){

	char test1[20];
	char test2[20];
	strcpy(test1,TableauDecoupage[1]);
	strcpy(test2,TableauDecoupage[2]);
	printf("Chaine 1 :%s \n",test1);
	printf("Chaine 2 :%s \n",test2);
	TestPixelOutOfBound(test1,test2);
	//TestBadColor(test2);
	
	
	
}

/* void MessageADecomposer(char Message[LG_MESSAGE])

params : char Message[LG_MESSAGE] -> Message recu du client
	
	 

Return :void 

But : Interprétation du message du client
*/

void MessageADecomposer(char Message[LG_MESSAGE]){


	
	printf("Entrez dans MessageADecomposer \n");
	printf("Voici le message recu : [%s] \n",Message);
	char MessageAfficheClient[256];
	char TestMatrice[2000];
	
	    char pattern[] = "/setPixel";

	    // On teste si la chaîne commence par le motif
	    
	if(strcmp(Message,"/getSize\0")==0){  //done
		printf("Commande ok /getSize \n");
		
		strcpy(messageEnvoi,GetSizeCommande(MessageAfficheClient));
		
	}
	else if(strcmp(Message,"/getMatrice\0")==0){
		printf("Commande ok /getMatrice \n");
		 //strcpy(messageEnvoi,getMatrice(matrice, l*c));
		 //AfficheMatriceDeJeu(MatriceDeJeu);
		// strcpy(TestMatrice,ReturnMatriceDeJeu(MatriceDeJeu,TestMatrice));
		// printf("Sortie de Strcpy \n");
		for (int i = 0; i < L; ++i)//parcours des lignes
	{
		printf("Modification en cours I=[%d] \n",i);
		for (int j = 0; j < C; ++j)//parcours des colonnes 
		{
			printf("Modification en cours J=[%d] \n",j);
			strcat(TestMatrice,MatriceDeJeu[i][j].couleur);
			
		}
		
	}
		 strcpy(messageEnvoi,TestMatrice);
		  printf("Sortie de Strcpy du messageEnvoi \n");
		 printf(messageEnvoi);
		 strcpy(TestMatrice,"");
		 printf("Fin de /getMatrice \n");
	
	}
	else if(strcmp(Message,"/getLimits\0")==0){ //done
		printf("Commande ok /getLimits \n");
		strcpy(messageEnvoi,GetLimitCommande(MessageAfficheClient));
		//printf(messageEnvoi);
	
	}
	else if(strcmp(Message,"/getVersion\0")==0){
		printf("Commande ok /getVersion \n");
		//printf("Version 1.2 \n");
		strcpy(messageEnvoi,"Version 1");
	
	}
	else if(strcmp(Message,"/getWaitTime\0")==0){
		printf("Commande ok /getWaitTime \n");
		char str[20];
		sprintf(str, "%d", sec_left);
		strcpy(messageEnvoi,str);
		printf("Fin de copy getwaitTime\n");
	
	}
	else if(strcmp(Message,"/setPixel\n")==0){//debug
		printf("Commande ok /setPixel \n");
		SetPixelCommande(MatriceDeJeu,0,0,"100100100");
		strcpy(messageEnvoi,"Pixel correctement modifié \n");
	
	}
	
	 
	else if(strncmp(Message, pattern, strlen(pattern))==0){
		int test=DecoupeMessageSetPixel(Message);
		printf("Dans le cas ou sa commance par /setPixel \n");
		if(test==-1){
		
			printf("Mauvaise Commande \n");
			strcpy(messageEnvoi,"10 Bad Command");
		}
		else if(test==1){
		
			printf("Verification des coo et de la couleur \n");
			TestDecoupageMessage();
		}
		
	}
	else{
		printf("Bad command \n"); //Ajout d'un test pour le cas ou le client tape nimp
		
		strcpy(messageEnvoi,"99 Unknow command \n");
	}
	strcpy(Message,"");
	strcpy(MessageAfficheClient,"");

}


/* char* GetLimitCommande(char MotAReturn[256])

params : char MotAReturn[256] -> Valeur de retour pour le client
	
	 

Return :char* 

But : Gestion du cas ou le client fait /getLimits
*/

char* GetLimitCommande(char MotAReturn[256]){

	printf("Voici les limites du jeu PixelWar : \n");
	char temp[150];
	char temp2[3];
	sprintf(temp2, "%d",Max_Pixel);
	strcpy(temp,temp2);
	//strcat(temp," est le nombre de pixel max par minute \n");
	
	strcat(MotAReturn,temp);
	return MotAReturn;
}


/* void initMatrice(CASE matrice[L][C])

params : CASE matrice[L][C] -> la matrice de jeu à init
	
	 

Return :void

But : Initialiser la matrice de jeu en 255255255 (base64)
*/

void initMatrice(CASE matrice[L][C]){

	for (int i = 0; i < L; ++i)//parcours des lignes
	{
		for (int j = 0; j < C; ++j)//parcours des colonnes 
		{
			strcpy(matrice[i][j].couleur,"////");
		}
	}
}

/* void AfficheMatriceDeJeu(CASE Matrice[L][C])

params : CASE matrice[L][C] -> la matrice de jeu 
	
	 

Return :void

But : Afficher la matrice de jeu
*/
void AfficheMatriceDeJeu(CASE Matrice[L][C]){

	for (int i = 0; i < L; ++i)//parcours des lignes
	{
		for (int j = 0; j < C; ++j)//parcours des colonnes 
		{
			printf("%s",Matrice[i][j].couleur);
		}
		printf("\n");
	}

}
/* char* ReturnMatriceDeJeu(CASE Matrice[L][C],char TestMatrice[2000])

params : CASE matrice[L][C] -> la matrice de jeu 
	 char TestMatrice[2000]) -> valeur de retour
	
	 

Return :char* la matrice de jeu

But : Renvoyer la matrice de jeu quand le client fait /getMatrix
*/

char* ReturnMatriceDeJeu(CASE Matrice[L][C],char TestMatrice[2000]){


	char TestMatrice2[2000];
	for (int i = 0; i < L; ++i)//parcours des lignes
	{
		printf("Modification en cours I=[%d] \n",i);
		for (int j = 0; j < C; ++j)//parcours des colonnes 
		{
			printf("Modification en cours J=[%d] \n",j);
			strcat(TestMatrice,Matrice[i][j].couleur);
			
		}
		
	}
	printf("Fin de strcat pour renvoiyer la matrice \n");
	return TestMatrice;

}
/* void SetPixelCommande(CASE Matrice[L][C], int ChoixLigne, int ChoixColonne, char NewCouleur[10])

params : CASE matrice[L][C] -> la matrice de jeu 
	 int ChoixLigne-> ligne choisi
	  int ChoixColonne-> Colonne choisi
	char NewCouleur[10] -> couleur choisi
	 

Return :char* la matrice de jeu

But : Fonction de debug pour modifier la matrice
*/


void SetPixelCommande(CASE Matrice[L][C], int ChoixLigne, int ChoixColonne, char NewCouleur[10]){

	printf("Modification de la matrice \n");
	if(ChoixLigne>L || ChoixColonne>C){
	
		printf("Erreur dans les positions \n");
	}
	else{
	
		strcpy(MatriceDeJeu[ChoixLigne][ChoixColonne].couleur,NewCouleur);
	}

}
/* void SetPixelCommande(CASE Matrice[L][C], int ChoixLigne, int ChoixColonne, char NewCouleur[10])

params : CASE matrice[L][C] -> la matrice de jeu 
	 int ChoixLigne-> ligne choisi
	  int ChoixColonne-> Colonne choisi
	char NewCouleur[10] -> couleur choisi
	 

Return :char* la matrice de jeu

But : Fonction de debug pour modifier la matrice
*/

void setPixel(CASE matrice[L][C], int posL, int posC, char val[10]){
	strcpy(matrice[posL][posC].couleur,val);
}




/* char* GetSizeCommande(char MotAReturn[256])

params : char MotAReturn[256]-> valeur de retour
	 

Return :char* Message pour le getSize

But : Renvoyer les infos pour le cas ou le client utilise /getSize
*/


char* GetSizeCommande(char MotAReturn[256]){

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

















