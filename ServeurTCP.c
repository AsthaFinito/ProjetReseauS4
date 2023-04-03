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
int tableau_coup_Restant[10]={1,1,1,1,1,1,1,1,1,1};
char messageEnvoi[2000];/* le message de la couche Application ! */
char TableauDecoupage[3][500];
char TableauDecoupagePosition[2][10];
char base46_map[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
                     
int b64invs[] = { 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51 };
	
	const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


typedef struct CASE{
	char couleur[25]; //couleur format RRRGGGBBB

}CASE;

CASE MatriceDeJeu[L][C];

typedef struct Users {
  int data;
  int User_Connexion_fdp;
  struct Users* suivant;
  int coup_restant;
} Users;



typedef struct queue {
  Users* first;
  
} queue;

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





void initMatrice(CASE matrice[L][C]);
void setPixel(CASE matrice[L][C], int posL, int posC, char val[10]);
char *getMatrice(CASE matrice[L][C]);
char *getSize();


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

char base64_to_ascii(char base64);
char* base64_to_bits(char* cipher);
size_t b64_decoded_size(const char *in);
char* base64_encode(char* plain);
int b64_decode(const char *in, unsigned char *out, size_t outlen);
int b64_isvalidchar(char c);
size_t b64_encoded_size(size_t inlen);
int TestChiffreDansBits(char* IN);
void AfficheStatUser(Users *User);
int char_in_array(char c, const char* array);
void ConvertirB64ToBin(char* input);
void int_to_binary(unsigned int n, char* binary);
void SplitCharEn3(char* input_str, char* output_str_1, char* output_str_2, char* output_str_3);
int TransformeBinaire(char* input);
int testValeurRGB(int test);


void TestLancementServeur(int argc, char *argv[]){

	
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
	
	
		    
		  // nombre de secondes restantes jusqu'à la prochaine minute
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
		    	
		    		tableau_coup_Restant[i]=1;
		    	}
		    }
		   printf("Il reste %d secondes avant la prochaine minute.\n", sec_left);
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
		else{
		
			//printf("Pas de connection \n");
		}
		// c’est un appel bloquant
		

		for(int i=1;i<11;i++){
			 //printf("Action? \n");
			 
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

	// On ferme la ressource avant de quitter
	close(socketEcoute);
	return 0;
}

int DecoupeMessageSetPixel(char* MessageComplet){

	char Decoupage[2]=" ";
	char *token;
	token=strtok(MessageComplet,Decoupage);
	int compteur =0;
	
	strcpy(TableauDecoupage[compteur],token);
	while(token!=NULL){
	
		
		if(compteur>=3){
		
			printf("Erreur pour le /setPixel (trop d'argument ) \n");
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

void AfficheStatUser(Users *User){
	printf("Affichage des stats de l'user : \n");
	printf("%d -> data \n",User->data);
	
	printf("%d -> coup restant \n",User->coup_restant);
	if(User->suivant==NULL){
		printf("Pas de suivant à l'user \n");
	}else{
	
		printf("Un autre user suit \n");
	}
}

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
	
	
	
		printf("Test en cours \n");
		
		
			
			//strcpy(CHAINE_ENCODE,base64_encode(MessageColor)); 
			strcpy(CHAINE_ENCODE,MessageColor);// encode en base64 
			strcpy(CHAINE_MATRICE,MessageColor);
			//unsigned int SIZE_OF_ENCODE=b64_encoded_size(strlen(CHAINE_ENCODE));
			printf(" [%s] encoded \n",CHAINE_ENCODE); //j'affiche
			//printf("Valeur de b64_encoded_size : %d \n",SIZE_OF_ENCODE);
			//test=base64_to_ascii(test2);
			//strcpy(NEWSTR,base64_to_bits(NEWSTR2)); //conversion bits 
			//int test_valeur=b64_decode(CHAINE_ENCODE,CHAINE_DECODE,10);
			ConvertirB64ToBin(CHAINE_ENCODE);
			SplitCharEn3(CHAINE_ENCODE,test12,test22,test32);
			printf("Valeur des merdes : [%s] [%s] [%s] \n",test12,test22,test32);
			
			int valeur_Bin1 = TransformeBinaire(test12);
			printf("Valeur_bin1= [%d] \n",valeur_Bin1);
			int test_chiffre1=testValeurRGB(valeur_Bin1);
			int valeur_Bin2 = TransformeBinaire(test22);
			printf("Valeur_bin2= [%d] \n",valeur_Bin2);
			int test_chiffre2=testValeurRGB(valeur_Bin2);
			int valeur_Bin3 = TransformeBinaire(test32);
			printf("Valeur_bin3= [%d] \n",valeur_Bin3);
			int test_chiffre3=testValeurRGB(valeur_Bin3);
			//printf("Chaine décode ? [%c] \n",test);
			//printf("Chaine décode2 ? [%s] \n",NEWSTR);
			//printf("Chaine décode2 ? [%02x] \n",CHAINE_DECODE);
			//printf("Chaine décode2 en s ? [%s] \n",CHAINE_DECODE);
			//unsigned int test_size=b64_decoded_size(NEWSTR);
			//unsigned int test_size=b64_decoded_size(CHAINE_DECODE);
			//printf("Valeur de b64_decoded_size : %d \n",test_size);
			//int test_chiffre=TestChiffreDansBits(CHAINE_DECODE);
		
			if(test_chiffre1==-1 || test_chiffre2==-1 || test_chiffre3==-1){
			
				strcpy(messageEnvoi,"12 Bad Color");
			}
			else{
				
				Users *ParcourtDeLaQueue;
				int compteur=1;
				printf("Valeur de actionUser : %d \n",actionUsers);
				
				//on chercher qui a fait l'action
				
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
					strcpy(MatriceDeJeu[Ligne][Colonne].couleur,CHAINE_MATRICE);
					printf("Après modif de matrice \n");
					strcpy(messageEnvoi,"00 OK");
					//ParcourtDeLaQueue->coup_restant=ParcourtDeLaQueue->coup_restant-1;
					tableau_coup_Restant[actionUsers]=0;
					
				}
				//on regarde si il a assez de pixel
				//si oui on modifie le pixel
				//si non on renvoie 20 outOfQuota
				
				
			}
			
		
		
		
	
	


}

int testValeurRGB(int test){

	if(test<0 || test>255){
	
		printf("Erreur dans la couleur \n");
		return -1;
	}else{
		printf("Couleur valide \n");
		return 1;
	}
}

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


void SplitCharEn3(char* input_str, char* output_str_1, char* output_str_2, char* output_str_3) {
    int len = strlen(input_str);
    int part_len = len / 3;
    strncpy(output_str_1, input_str, part_len);
    strncpy(output_str_2, input_str + part_len, part_len);
    strncpy(output_str_3, input_str + 2 * part_len, part_len);
    printf("Valeur des merdes : [%s] [%s] [%s] \n",output_str_1,output_str_2,output_str_3);
}

void int_to_binary(unsigned int n, char* binary) {
    int i;
    for (i = 5; i >= 0; i--) {
        binary[i] = (n % 2) + '0';
        n /= 2;
    }
    binary[6] = '\0';
}

size_t b64_encoded_size(size_t inlen)
{
	size_t ret;

	ret = inlen;
	if (inlen % 3 != 0)
		ret += 3 - (inlen % 3);
	ret /= 3;
	ret *= 4;

	return ret;
}
int b64_decode(const char *in, unsigned char *out, size_t outlen)
{
	size_t len;
	size_t i;
	size_t j;
	int    v;

	if (in == NULL || out == NULL)
		return 0;

	len = strlen(in);
	if (outlen < b64_decoded_size(in) || len % 4 != 0)
		return 0;

	for (i=0; i<len; i++) {
		if (!b64_isvalidchar(in[i])) {
			return 0;
		}
	}

	for (i=0, j=0; i<len; i+=4, j+=3) {
		v = b64invs[in[i]-43];
		v = (v << 6) | b64invs[in[i+1]-43];
		v = in[i+2]=='=' ? v << 6 : (v << 6) | b64invs[in[i+2]-43];
		v = in[i+3]=='=' ? v << 6 : (v << 6) | b64invs[in[i+3]-43];

		out[j] = (v >> 16) & 0xFF;
		if (in[i+2] != '=')
			out[j+1] = (v >> 8) & 0xFF;
		if (in[i+3] != '=')
			out[j+2] = v & 0xFF;
	}

	return 1;
}
int b64_isvalidchar(char c)
{
	if (c >= '0' && c <= '9')
		return 1;
	if (c >= 'A' && c <= 'Z')
		return 1;
	if (c >= 'a' && c <= 'z')
		return 1;
	if (c == '+' || c == '/' || c == '=')
		return 1;
	return 0;
}

char* base64_encode(char* plain) {

    char counts = 0;
    char buffer[3];
    char* cipher = malloc(strlen(plain) * 4 / 3 + 4);
    int i = 0, c = 0;

    for(i = 0; plain[i] != '\0'; i++) {
        buffer[counts++] = plain[i];
        if(counts == 3) {
            cipher[c++] = base46_map[buffer[0] >> 2];
            cipher[c++] = base46_map[((buffer[0] & 0x03) << 4) + (buffer[1] >> 4)];
            cipher[c++] = base46_map[((buffer[1] & 0x0f) << 2) + (buffer[2] >> 6)];
            cipher[c++] = base46_map[buffer[2] & 0x3f];
            counts = 0;
        }
    }

    if(counts > 0) {
        cipher[c++] = base46_map[buffer[0] >> 2];
        if(counts == 1) {
            cipher[c++] = base46_map[(buffer[0] & 0x03) << 4];
            cipher[c++] = '=';
        } else {                      // if counts == 2
            cipher[c++] = base46_map[((buffer[0] & 0x03) << 4) + (buffer[1] >> 4)];
            cipher[c++] = base46_map[(buffer[1] & 0x0f) << 2];
        }
        cipher[c++] = '=';
    }

    cipher[c] = '\0';   /* string padding character */
    return cipher;
}

size_t b64_decoded_size(const char *in)
{
	size_t len;
	size_t ret;
	size_t i;

	if (in == NULL)
		return 0;

	len = strlen(in);
	ret = len / 4 * 3;

	for (i=len; i-->0; ) {
		if (in[i] == '=') {
			ret--;
		} else {
			break;
		}
	}

	return ret;
}

char* base64_to_bits(char* cipher) {

    char counts = 0;
    char buffer[4];
    char* plain = malloc(strlen(cipher) * 3 / 4);
    int i = 0, p = 0;

    for(i = 0; cipher[i] != '\0'; i++) {
        char k;
        for(k = 0 ; k < 64 && base46_map[k] != cipher[i]; k++);
        buffer[counts++] = k;
        if(counts == 4) {
            plain[p++] = (buffer[0] << 2) + (buffer[1] >> 4);
            if(buffer[2] != 64)
                plain[p++] = (buffer[1] << 4) + (buffer[2] >> 2);
            if(buffer[3] != 64)
                plain[p++] = (buffer[2] << 6) + buffer[3];
            counts = 0;
        }
    }

    plain[p] = '\0';    /* string padding character */
    return plain;
}
char base64_to_ascii(char base64) {
	printf("Dans décode 1 carac \n");
    char* table_base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char* position = strchr(table_base64, base64);
    int index = (int)(position - table_base64);
	
    if (position == NULL) {
        return '\0';
    }
	printf("Après le if \n");
    unsigned char ascii = index;
	printf("Après le ascii \n");
    return (char)ascii;
}




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

void MessageADecomposer(char Message[LG_MESSAGE]){

	printf("Entrez dans MessageADecomposer \n");
	printf("Voici le message recu : [%s] \n",Message);
	char MessageAfficheClient[256];
	char TestMatrice[2000];
	
	    char pattern[] = "/setPixel";

	    // On teste si la chaîne commence par le motif
	    
	if(strcmp(Message,"/getSize\n")==0){  //done
		printf("Commande ok /getSize \n");
		
		strcpy(messageEnvoi,GetSizeCommande(MessageAfficheClient));
		
	}
	else if(strcmp(Message,"/getMatrice\n")==0){
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
	else if(strcmp(Message,"/getLimits\n")==0){ //done
		printf("Commande ok /getLimits \n");
		strcpy(messageEnvoi,GetLimitCommande(MessageAfficheClient));
		//printf(messageEnvoi);
	
	}
	else if(strcmp(Message,"/getVersion\n")==0){
		printf("Commande ok /getVersion \n");
		//printf("Version 1.2 \n");
		strcpy(messageEnvoi,"Version 1");
	
	}
	else if(strcmp(Message,"/getWaitTime\n")==0){
		printf("Commande ok /getWaitTime \n");
		char str[5];
		sprintf(str, "%d", sec_left);
		strcpy(messageEnvoi,str);
		printf("Fin de copy getwaitTime\n");
	
	}
	else if(strcmp(Message,"/setPixel\n")==0){
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




void initMatrice(CASE matrice[L][C]){

	for (int i = 0; i < L; ++i)//parcours des lignes
	{
		for (int j = 0; j < C; ++j)//parcours des colonnes 
		{
			strcpy(matrice[i][j].couleur,"////");
		}
	}
}


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

void SetPixelCommande(CASE Matrice[L][C], int ChoixLigne, int ChoixColonne, char NewCouleur[10]){

	printf("Modification de la matrice \n");
	if(ChoixLigne>L || ChoixColonne>C){
	
		printf("Erreur dans les positions \n");
	}
	else{
	
		strcpy(MatriceDeJeu[ChoixLigne][ChoixColonne].couleur,NewCouleur);
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

char *getSize(){
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

















