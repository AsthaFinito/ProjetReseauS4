
#include <stdio.h>
#include <stdlib.h> /* pour exit */
#include <unistd.h> /* pour close */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h> /* pour htons et inet_aton */
#include <math.h>

#define LG_MESSAGE 256
#define PORT IPPORT_USERRESERVED //5000

char messageRecu[LG_MESSAGE];/* le message de la couche Application ! */
	char messageEnvoi[LG_MESSAGE];/* le message de la couche Application ! */
char base46_map[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";



void TestLancementClient(int argc, char *argv[]);
void InterpretationMessageBoucle(char *MessageRecu);
char* base64_encode(char* bits);
int Menu();
int InterpretationMenu(char *input);
void CasSetPixel();
void print_bits(int n);
char* int_to_binary_string(int n);

void SplitCharEn3(char* input_str, char* output_str_1, char* output_str_2, char* output_str_3);
void SplitCharEn4(char* input_str, char* output_str_1, char* output_str_2, char* output_str_3, char* output_str_4);
char TransformeBinaire(char* input);



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
				//printf("Dans le default \n");
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
		strcpy(messageEnvoi,"/getSize\0");
		return 1;
	}
	else if(strcmp(input,"2\0")==0){
	
		printf("Vous avez choisi le getLimits \n");
		strcpy(messageEnvoi,"/getLimits\0");
		return 1;
	}
	else if(strcmp(input,"3\0")==0){
	
		printf("Vous avez choisi le getMatrice \n");
		strcpy(messageEnvoi,"/getMatrice\0");
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
		strcpy(messageEnvoi,"/getWaitTime\0");
		return 1;
	}
	else if(strcmp(input,"6\0")==0){
	
		printf("Vous avez choisi le getWaitTime \n");
		strcpy(messageEnvoi,"/getVersion\0");
		return 1;
	}
	else{
		printf("Erreur dans le choix d'action \n");
		return 0;
	}
	printf("Envoie au serveur : [%s]\n",messageEnvoi);

}

void SplitCharEn3(char* input_str, char* output_str_1, char* output_str_2, char* output_str_3) {
    int len = strlen(input_str);
    int part_len = len / 3;
    strncpy(output_str_1, input_str, part_len);
    strncpy(output_str_2, input_str + part_len, part_len);
    strncpy(output_str_3, input_str + 2 * part_len, part_len);
   // printf("Valeur des merdes : [%s] [%s] [%s] \n",output_str_1,output_str_2,output_str_3);
}

void SplitCharEn4(char* input_str, char* output_str_1, char* output_str_2, char* output_str_3, char* output_str_4) {
    int len = strlen(input_str);
    
    int part_len = len / 4;
    strncpy(output_str_1, input_str, part_len);
    strncpy(output_str_2, input_str + part_len, part_len);
    strncpy(output_str_3, input_str + 2 * part_len, part_len);
    strncpy(output_str_4, input_str + 3 * part_len, part_len);
    output_str_1[part_len] = '\0'; // Ajouter un caractère nul pour terminer la chaîne de sortie
    output_str_2[part_len] = '\0';
    output_str_3[part_len] = '\0';
    output_str_4[part_len] = '\0';
    // printf("Valeur des merdes : [%s] [%s] [%s] [%s] \n",output_str_1,output_str_2,output_str_3,output_str_4);
}



void CasSetPixel(){


	strcat(RenvoieSetPixel,"/setPixel ");

	printf("Veuillez rentrer une position-> Forme : LxC \n");
	char input[50];
	char test1[30]="";
	char test2[30]="";
	char test3[30]="";
	char test4[30]="";
	char reconstitution[20];
	fgets(input, 50, stdin); 
	int pos = strchr(input, '\n') - input;
    	if (pos >= 0 && pos < strlen(input)) {
    	
        	input[pos] = '\0';
        	//printf(" backslashN enlevé à %d\n",pos);
   	 } 
   	 strcat(RenvoieSetPixel,input);
   	 strcat(RenvoieSetPixel," ");
   	 printf("Veuillez rentrer une couleur->  \n");
   	 fgets(input, 50, stdin); 
   	 pos= strchr(input, '\n') - input;
   	 if (pos >= 0 && pos < strlen(input)) {
    	
        	input[pos] = '\0';
        	//printf(" backslashN enlevé à %d\n",pos);
   	 }
   	 
   	 SplitCharEn3(input,test1,test2,test3);
   	 
   	 strcpy(test1,int_to_binary_string(atoi(test1)));
   	 strcpy(test2,int_to_binary_string(atoi(test2)));
   	 strcpy(test3,int_to_binary_string(atoi(test3)));
   	 strcpy(input,"");
   	 
   	 strcat(input,test1);
   	 strcat(input,test2);
   	 strcat(input,test3);
   	// printf("Valeur de l'input avec la cassure en 4 : [%s]  + strlen: [%d]\n",input,strlen(input));
   	 strcpy(test1,"");
   	 strcpy(test2,"");
   	 strcpy(test3,"");
   	 //strcpy(test4,"");
   	 SplitCharEn4(input,test1,test2,test3,test4);
   	// strcpy(input,base64_encode(input));
   	 char V1=TransformeBinaire(test1);
   	// printf("V1 = [%c] \n",V1);
   	 reconstitution[0]=V1;
   	 V1=TransformeBinaire(test2);
   	 reconstitution[1]=V1;
   	//  printf("V1 = [%c] \n",V1);
   	 V1=TransformeBinaire(test3);
   	 // printf("V1 = [%c] \n",V1);
   	  reconstitution[2]=V1;
   	 V1=TransformeBinaire(test4);
   	 // printf("V1 = [%c] \n",V1);
   	  reconstitution[3]=V1;
   	  reconstitution[4]='\0';
   	  //printf("Voici la reconstitution : [%s] \n",reconstitution);
   	// printf("Valeur de la couleur encodé : [%s] \n",input);
   	 strcat(RenvoieSetPixel,reconstitution);
   	// strcat(RenvoieSetPixel,"\n");
   	 strcpy(messageEnvoi,RenvoieSetPixel);
   	 //strcpy(messageEnvoi,"");
   	  strcpy(RenvoieSetPixel,"");
   	 printf("Envoie au serveur : [%s]\n",messageEnvoi);
	
}

int Menu(){

	char test[30];
	strcpy(messageEnvoi,"");
	printf("Choissisez une fonction : \n");
	printf("	1 - getSize \n");
	printf("	2 - getLimits \n");
	printf("	3 - getMatrice \n");
	printf("	4 - setPixel \n");
	printf("	5 - getWaitTime \n");
	printf("	6 - getVersion \n");
	printf("	7 - TestConversion base64 \n");
	//print_bits(255);
	//strcpy(test,int_to_binary_string(255));
	//printf("Version s : [%s] \n",test);
	char input[10];
	fgets(input, 10, stdin); // Lecture de la chaîne de caractères avec fgets

    	// Recherche de la position du caractère \n dans la chaîne
    	int pos = strchr(input, '\n') - input;
    	if (pos >= 0 && pos < strlen(input)) {
    	
        	input[pos] = '\0';
        	//printf(" backslashN enlevé à %d\n",pos);
   	 }
   	 else{
   	 
   	 	//printf("Pas de modification \n");
   	 }
	//printf("Voici la touche tapé : [%s] \n",input);
	int test_FinInterpretation=InterpretationMenu(input);
	if(test_FinInterpretation==1){
		//printf("Envoie au serveur : [%s]\n",messageEnvoi);
		return 1;
	}
	else{
		return 0;
	}
	
	

}

void InterpretationMessageBoucle(char *MessageRecu){

	//printf("Interprétation en cours \n");
	printf("Reponse du serveur :\n%s \n",MessageRecu);
	//printf("Affichage de la matrice : \n");
	int compteur=0;
	
		//for(int j=0;j<strlen(MessageRecu);j++){
		//printf("Dans boucle \n");
		//	printf("%c",MessageRecu[j]);
		//	compteur++;
		//	if(compteur==84){
			
		//		printf("\n");
		//		compteur=0;
		//	}
		//}
	
	
}


char* base64_encode(char* bits) {

     size_t bits_len = strlen(bits);
    size_t base64_len = ((bits_len + 2) / 3) * 4 + 1;
    char* base64 = malloc(base64_len);

    size_t i = 0;
    size_t j = 0;
    uint32_t buffer = 0;
    uint8_t bits_in_buffer = 0;

    for (i = 0; i < bits_len; i++) {
        buffer <<= 1;
        buffer |= bits[i] - '0';
        bits_in_buffer++;

        if (bits_in_buffer == 6) {
            base64[j++] = base64_chars[(buffer >> 18) & 0x3F];
            base64[j++] = base64_chars[(buffer >> 12) & 0x3F];
            base64[j++] = base64_chars[(buffer >> 6) & 0x3F];
            base64[j++] = base64_chars[buffer & 0x3F];
            buffer = 0;
            bits_in_buffer = 0;
        }
    }

    if (bits_in_buffer == 2) {
        buffer <<= 4;
        base64[j++] = base64_chars[(buffer >> 6) & 0x3F];
        base64[j++] = base64_chars[buffer & 0x3F];
        base64[j++] = '=';
        base64[j++] = '=';
    } else if (bits_in_buffer == 4) {
        buffer <<= 2;
        base64[j++] = base64_chars[(buffer >> 12) & 0x3F];
        base64[j++] = base64_chars[(buffer >> 6) & 0x3F];
        base64[j++] = base64_chars[buffer & 0x3F];
        base64[j++] = '=';
    }

    base64[j] = '\0';

    return base64;
}



void print_bits(int n) {
    int i;
    for (i = 7; i >= 0; i--) {
        int bit = (n >> i) & 1;
        //printf("%d", bit);
    }
   // printf("\n");
}

char* int_to_binary_string(int n) {
    // Calculer le nombre de bits nécessaires pour représenter l'entier en binaire
    int num_bits = 0;
    int temp = n;
    while (temp > 0) {
        num_bits++;
        temp >>= 1;
    }
    if (num_bits == 0) num_bits = 1; // Pour le cas où n est égal à 0
    if (num_bits < 8) {
        num_bits = 8;
    }
    
    char* binary_str = malloc((num_bits + 1) * sizeof(char));
    if (binary_str == NULL) {
        printf("Erreur: Impossible d'allouer de la mémoire.\n");
        return NULL;
    }
    binary_str[num_bits] = '\0'; // Terminer la chaîne de caractères avec un caractère nul
    
    int i;
    for (i = num_bits - 1; i >= 0; i--) {
        int bit = (n >> i) & 1;
        binary_str[num_bits - 1 - i] = bit ? '1' : '0'; // Ajouter le bit courant à la chaîne de caractères
    }
    return binary_str;
}

char TransformeBinaire(char* input){
	unsigned int valeur=0;
	for(int i=0;i<strlen(input);i++){
		//printf("Valeur de input[i]-48  : [%d] \n",input[i]-48);
		//int test=pow(2,i);
		valeur=valeur+(pow(2,strlen(input)-1-i)*(input[i]-48));
		//printf("debug %d \n",pow(2,2));
	}
	//printf("Valeur : [%d] \n",valeur);
	return base46_map[valeur];
}

