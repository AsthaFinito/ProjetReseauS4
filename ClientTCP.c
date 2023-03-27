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


void TestLancementClient(int argc, char *argv[]);
void InterpretationMessageBoucle(char *MessageRecu);



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
	char messageRecu[LG_MESSAGE];/* le message de la couche Application ! */
	char messageEnvoi[LG_MESSAGE];/* le message de la couche Application ! */
	char InputClient[LG_MESSAGE];
	memset(messageEnvoi, 0x00, LG_MESSAGE*sizeof(char));
	memset(messageRecu, 0x00, LG_MESSAGE*sizeof(char));
	
	while(1){
	
		fgets(InputClient, sizeof(InputClient), stdin);
		strcpy(messageEnvoi, InputClient);
		ecrits = write(socketEcoute, messageEnvoi, strlen(messageEnvoi));// message à TAILLE variable
		//printf("Avant le switch ecrit \n");
		switch(ecrits)
		{
			case-1 :/* une erreur ! */
				perror("write");
				close(socketEcoute);
				exit(-3);
				break;
			case 0 :/* la socket est fermée */
				fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
				close(socketEcoute);
				exit(-1);
				break;
			default:
				break;

		}
	
		lus = read(socketEcoute, messageRecu, LG_MESSAGE*sizeof(char));
		//printf("Le message recu est : %s \n",messageRecu);
		switch(lus)
		{
			case-1 :/* une erreur ! */
				perror("read");
				close(socketEcoute);
				exit(-4);
			case 0 :/* la socket est fermée */
				fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
				close(socketEcoute);
				exit(-1);
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

void InterpretationMessageBoucle(char *MessageRecu){

	printf("Interprétation en cours \n");
	printf("Le message recu est : %s \n",MessageRecu);
}
