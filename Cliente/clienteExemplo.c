#include "cliente.h"

void error(const char *msg) {
    perror(msg);
    exit(0);
}

// bzero() foi descontinuado, substitui por memset()
// bcopy() foi descontinuado, substitui por memcpy()

int main(int argc, char *argv[]) {
    int sockfd, portno, num_bytes_msg;
    // sockfd é um descritor e representa a conexão
    // portno é a porta do servidor
    // num_bytes_msg é o numero de bytes da mensagem lida ou escrita
    struct sockaddr_in serv_addr; // Endereço e porta do servidor
    struct hostent *server; // Informações do servidor que ele vai conetar

    char buffer[256];
    //--------------------------------------------------------------------------------------------
    // Verifica os parametros
    //--------------------------------------------------------------------------------------------
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    //--------------------------------------------------------------------------------------------
    // Cria o socket com ipv4 e TCP
    //--------------------------------------------------------------------------------------------
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //AF_INET é o ipv4, SOCK_STREAM O ipv4
    if (sockfd < 0) {                         
        error("error opening socket");
    }
    //--------------------------------------------------------------------------------------------
    // Busca o ip do host pelo nome
    //--------------------------------------------------------------------------------------------
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"error, no such host\n");
        exit(0);
    }
    //--------------------------------------------------------------------------------------------
    // Configura o endereço do servidor
    //--------------------------------------------------------------------------------------------
    memset((char *) &serv_addr, 0, sizeof(serv_addr)); // garante que o endereço esta limpo
    serv_addr.sin_family = AF_INET; // diz que esta usando ipv4
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], server->h_length); 
    // copia o IP do servidor para a estrutura que contem o endereço (IP) e a porta do servidor
    serv_addr.sin_port = htons(portno); // salva a porta no formato de rede (big-endian)
    //--------------------------------------------------------------------------------------------
    // Conecta ao servidor
    //--------------------------------------------------------------------------------------------
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        error("error connecting");
    }
    //--------------------------------------------------------------------------------------------
    // Conversa com o servidor
    //--------------------------------------------------------------------------------------------
    printf("Please enter the message: ");
    memset(buffer, 0, 256);
    fgets(buffer,255,stdin);
    
    num_bytes_msg = write(sockfd,buffer,strlen(buffer)); // Escreve a mensagem para o servidor
    if (num_bytes_msg < 0) {
        error("error writing to socket");
    }
    memset(buffer, 0, 256);
    num_bytes_msg = read(sockfd,buffer,255); // Lê a resposta do servidor
    if (num_bytes_msg < 0) {
        error("error reading from socket");
    }
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}
