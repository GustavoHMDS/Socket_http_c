/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include "servidor.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}
// bzero() foi descontinuado, substitui por memset()
int main(int argc, char *argv[]) {
    // sockfd é um descritor e representa a conexão
    // newsockfd é o cliente conectado
    // portno é a porta do servidor
    int sockfd, newsockfd, portno;
    socklen_t clilen; // tamanho 
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr; // endereço do servidor e do cliente
    int num_bytes_msg; // num_bytes_msg é o numero de bytes da mensagem lida ou escrita
    //--------------------------------------------------------------------------------------------
    // Verifica os parametros
    //--------------------------------------------------------------------------------------------
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // diz que esta usando ipv4 e TCP
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
    //--------------------------------------------------------------------------------------------
    // Abre o servidor
    //--------------------------------------------------------------------------------------------
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET; // diz que esta usando ipv4
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { // Abrindo
        error("ERROR on binding");
    }
    //--------------------------------------------------------------------------------------------
    // Espera uma conexão
    //--------------------------------------------------------------------------------------------
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        error("ERROR on accept");
    }
    //--------------------------------------------------------------------------------------------
    // Conversa com o cliente
    //--------------------------------------------------------------------------------------------
    memset(buffer, 0, 256);
    num_bytes_msg = read(newsockfd,buffer,255);
    if (num_bytes_msg < 0) {
        error("ERROR reading from socket");
    }
    printf("Here is the message: %s\n",buffer);
    num_bytes_msg = write(newsockfd,"I got your message",18);
    if (num_bytes_msg < 0) {
        error("ERROR writing to socket");
    }
    close(newsockfd);
    close(sockfd);
    return 0; 
}