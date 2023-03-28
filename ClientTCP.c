
#include <stdio.h>
#include <stdlib.h> /* pour exit */
#include <unistd.h> /* pour close */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h> /* pour htons et inet_aton */

#define LG_MESSAGE 256
#define PORT IPPORT_USERRESERVED //5000

char messageRecu[LG_MESSAGE];/* le message de la couche Application ! */
	char messageEnvoi[LG_MESSAGE];/* le message de la couche Application ! */
char base46_map[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

void TestLancementClient(int argc, char *argv[]);
void InterpretationMessageBoucle(char *MessageRecu);
char* base64_encode(char* plain);
int Menu();
int InterpretationMenu(char *input);
void CasSetPixel();


char RenvoieSetPixel[100];

int main(int argc, char *argv[]){


	int opt;
	    int port=0;
		char *ip = NULL;
	
	    if (argc != 5) {
		fprintf(stderr, "Usage: %s -i IP -p PORT\n", argv[0]);
		exit(EXIT_FAILURE);
	    }
	    while ((opt = getopt(argc, argv, "i:p:")) != -1) {
		switch (opt) {
		    case 'i':
		        ip = optarg;
		        break;
		    case 'p':
		        port = atoi(optarg);
		        break;
		    default:
		        fprintf(stderr, "Usage: %s [-i IP] [-p PORT]\n", argv[0]);
		        exit(EXIT_FAILURE);
		}
	    }

	int socketEcoute = socket(PF_INET, SOCK_STREAM, 0);
	    if (socketEcoute == -1) {
		perror("Erreur lors de la création du socket");
		exit(EXIT_FAILURE);
	    }
	    printf("Socket créée avec succès ! (%d)\n", socketEcoute);
	    struct sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(port); 
		serv_addr.sin_addr.s_addr = inet_addr(ip); 

		// connexion vers le serveur
		if((connect(socketEcoute, (struct sockaddr *)&serv_addr,sizeof(serv_addr))) == -1){
			perror("Connexion vers le serveur à échouée");
			close(socketEcoute);
			exit(-2);
	}
	printf("Connexion au serveur ?\n");
	int ecrits, lus;/* nb d’octets ecrits et lus */
	
	char InputClient[LG_MESSAGE];
	memset(messageEnvoi, 0x00, LG_MESSAGE*sizeof(char));
	memset(messageRecu, 0x00, LG_MESSAGE*sizeof(char));
	printf("Bienvenue dans le pixel war du groupe A05 \n");
	int test_SortieMenu=0;
	while(1){
	
		//fgets(InputClient, sizeof(InputClient), stdin);
		//strcpy(messageEnvoi, InputClient);
		while(test_SortieMenu==0){
			test_SortieMenu=Menu();
		}
		test_SortieMenu=0;
		ecrits = write(socketEcoute, messageEnvoi, strlen(messageEnvoi));// message à TAILLE variable
		printf("Avant le switch ecrit \n");
		switch(ecrits)
		{
			case-1 :/* une erreur ! */
				perror("write");
				close(socketEcoute);
				exit(-3);
				break;
			case 0 :/* la socket est fermée */
				//fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
				//close(socketEcoute);
				//exit(-1);
				break;
			default:
				//printf("Dans le default \n");
				break;

		}
		strcpy(messageEnvoi,"");
		lus = read(socketEcoute, messageRecu, LG_MESSAGE*sizeof(char));
		//printf("Le message recu est : %s \n",messageRecu);
		switch(lus)
		{
			case-1 :/* une erreur ! */
				perror("read");
				close(socketEcoute);
				exit(-4);
			case 0 :/* la socket est fermée */
				//fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
				//close(socketEcoute);
				//exit(-1);
				break;
			default:/* réception de n octets */
				printf("Dans le default \n");
				messageRecu[lus]='\0';
				InterpretationMessageBoucle(messageRecu);
				//printf("Message reçu du serveur : %s (%d octets)\n\n", messageRecu, lus);
		}
	
	
	}
	

}

void TestLancementClient(int argc, char *argv[]){

	
	int opt;
	    int port=0;
		char *ip = NULL;
	
	    if (argc != 5) {
		fprintf(stderr, "Usage: %s -i IP -p PORT\n", argv[0]);
		exit(EXIT_FAILURE);
	    }
	    while ((opt = getopt(argc, argv, "i:p:")) != -1) {
		switch (opt) {
		    case 'i':
		        ip = optarg;
		        break;
		    case 'p':
		        port = atoi(optarg);
		        break;
		    default:
		        fprintf(stderr, "Usage: %s [-i IP] [-p PORT]\n", argv[0]);
		        exit(EXIT_FAILURE);
		}
	    }


}
int InterpretationMenu(char *input){
	printf("Lettre reçu dans l'interpretation : [%s] \n",input);
	if(strcmp(input,"1\0")==0){
		printf("Vous avez choisi le getSize \n");
		strcpy(messageEnvoi,"/getSize\n");
		return 1;
	}
	else if(strcmp(input,"2\0")==0){
	
		printf("Vous avez choisi le getLimits \n");
		strcpy(messageEnvoi,"/getLimits\n");
		return 1;
	}
	else if(strcmp(input,"3\0")==0){
	
		printf("Vous avez choisi le getMatrice \n");
		strcpy(messageEnvoi,"/getMatrice\n");
		return 1;
	}
	else if(strcmp(input,"4\0")==0){
	
		printf("Vous avez choisi le setPixel \n");
		CasSetPixel();
		//strcpy(messageEnvoi,"/setPixel 0x0 128128128\n");
		return 1;
	}
	else if(strcmp(input,"5\0")==0){
	
		printf("Vous avez choisi le getWaitTime \n");
		//strcpy(messageEnvoi,"/getSize");
		return 1;
	}
	else if(strcmp(input,"6\0")==0){
	
		printf("Vous avez choisi le getWaitTime \n");
		strcpy(messageEnvoi,"/getVersion\n");
		return 1;
	}
	else{
		printf("Erreur dans le choix d'action \n");
		return 0;
	}


}

void CasSetPixel(){


	strcat(RenvoieSetPixel,"/setPixel ");

	printf("Veuillez rentrer une position-> Forme : LxC \n");
	char input[30];
	fgets(input, 30, stdin); 
	int pos = strchr(input, '\n') - input;
    	if (pos >= 0 && pos < strlen(input)) {
    	
        	input[pos] = '\0';
        	//printf(" backslashN enlevé à %d\n",pos);
   	 } 
   	 strcat(RenvoieSetPixel,input);
   	 strcat(RenvoieSetPixel," ");
   	 printf("Veuillez rentrer une couleur-> Forme : RGBRGBRGB \n");
   	 fgets(input, 30, stdin); 
   	 pos= strchr(input, '\n') - input;
   	 if (pos >= 0 && pos < strlen(input)) {
    	
        	input[pos] = '\0';
        	printf(" backslashN enlevé à %d\n",pos);
   	 } 
   	// strcat(RenvoieSetPixel,input);
   	 strcpy(input,base64_encode(input));
   	 printf("Valeur de la couleur encodé : [%s] \n",input);
   	 strcat(RenvoieSetPixel,input);
   	 strcat(RenvoieSetPixel,"\n");
   	 strcpy(messageEnvoi,RenvoieSetPixel);
   	  strcpy(RenvoieSetPixel,"");
   	// printf("Envoie au serveur : [%s]\n",RenvoieSetPixel);
	
}

int Menu(){

	strcpy(messageEnvoi,"");
	printf("Choissisez une fonction : \n");
	printf("	1 - getSize \n");
	printf("	2 - getLimits \n");
	printf("	3 - getMatrice \n");
	printf("	4 - setPixel \n");
	printf("	5 - getWaitTime \n");
	printf("	6 - getVersion \n");
	char input[10];
	fgets(input, 10, stdin); // Lecture de la chaîne de caractères avec fgets

    	// Recherche de la position du caractère \n dans la chaîne
    	int pos = strchr(input, '\n') - input;
    	if (pos >= 0 && pos < strlen(input)) {
    	
        	input[pos] = '\0';
        	printf(" backslashN enlevé à %d\n",pos);
   	 }
   	 else{
   	 
   	 	printf("Pas de modification \n");
   	 }
	printf("Voici la touche tapé : [%s] \n",input);
	int test_FinInterpretation=InterpretationMenu(input);
	if(test_FinInterpretation==1){
		printf("Envoie au serveur : [%s]\n",messageEnvoi);
		return 1;
	}
	else{
		return 0;
	}
	
	

}

void InterpretationMessageBoucle(char *MessageRecu){

	//printf("Interprétation en cours \n");
	printf("Reponse du serveur : %s ",MessageRecu);
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

