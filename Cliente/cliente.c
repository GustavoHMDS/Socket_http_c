#include "cliente.h"

#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(0);
}

http prepara_http(char input[]) {
    http resultado;
    char *slash = strchr(input, '/');

    if (slash) {
        size_t host_len = slash - input;
        strncpy(resultado.host, input, host_len);
        resultado.host[host_len] = '\0';

        strcpy(resultado.caminho, slash);
    } else {
        // Se não houver barra, o host é tudo e o caminho é "/"
        strcpy(resultado.host, input);
        strcpy(resultado.caminho, "/");
    }

    strcpy(resultado.metodo, "GET");
    strcpy(resultado.protocolo, "HTTP/1.1");

    return resultado;
}

ssize_t recebe_header(int sockfd, char *header_buffer, size_t max_len) {
    size_t total_received = 0;
    char c;
    int state = 0;
    while (total_received < max_len - 1) {
        ssize_t n = recv(sockfd, &c, 1, 0);
        if (n <= 0) {
            return n; // erro ou conexão fechada
        }
        header_buffer[total_received++] = c;

        // Detecta os dois "enters"
        if (state == 0 && c == '\r') state = 1;
        else if (state == 1 && c == '\n') state = 2;
        else if (state == 2 && c == '\r') state = 3;
        else if (state == 3 && c == '\n') {
            header_buffer[total_received] = '\0';
            return total_received;
        }
        else state = 0;
    }
    return -1;
}

void recebe_http(int sockfd, const char *filename) {
    char header[BUFFER_SIZE];
    ssize_t header_len = recebe_header(sockfd, header, sizeof(header));
    if (header_len <= 0) {
        error("Erro ao receber header HTTP");
    }

    http dados = interpreta_resposta_http(header);
    if (!dados.valido) {
        printf("Erro HTTP: %s\n", dados.mensagem_erro);
        return;
    }
    // verifica se foi um codigo suportado
    if(dados.codigo_status < 200 || dados.codigo_status >= 300) {
        printf("%d: %s\n", dados.codigo_status, dados.mensagem_status);
        return;
    }

    // verifica se tem que abrir arquivo
    FILE *file;
    bool is_text = strstr(dados.tipo, "text/plain");
    char caminho_completo[256] = "./Cliente/ItensCliente";
    if(strcmp(filename, "/") == 0) {
        strcat(caminho_completo, "/em_branco");
    }
    else{
        strcat(caminho_completo, filename);
    }    
    file = fopen(caminho_completo, "wb");
    if (!file) {
        perror("Erro ao criar arquivo");
        return;
    }
    // faz a leitura do conteudo
    size_t total_lidos = 0;
    char buffer[BUFFER_SIZE];
    while (total_lidos < dados.tamanho_conteudo) {
        size_t num_bytes_restantes = BUFFER_SIZE;
        if (dados.tamanho_conteudo - total_lidos < BUFFER_SIZE) {
            num_bytes_restantes = dados.tamanho_conteudo - total_lidos;
        }
        ssize_t num_bytes_lidos = recv(sockfd, buffer, num_bytes_restantes, 0);
        if (num_bytes_lidos <= 0) {
            perror("Erro ao receber conteúdo HTTP");
            return;
        }
        if(is_text) {
            fwrite(buffer, 1, num_bytes_lidos, stdout);
        }
        fwrite(buffer, 1, num_bytes_lidos, file);
        total_lidos += num_bytes_lidos;
    }
    fclose(file);   
}

int main(int argc, char *argv[]) {
    int sockfd, portno, num_bytes_msg;
    // sockfd é um descritor e representa a conexão
    // portno é a porta do servidor
    // num_bytes_msg é o numero de bytes da mensagem lida ou is_text
    struct sockaddr_in serv_addr; // Endereço e porta do servidor
    struct hostent *server; // Informações do servidor que ele vai conetar

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    char buffer[256];
    portno = atoi(argv[1]);
    //--------------------------------------------------------------------------------------------
    // Cria o socket com ipv4 e TCP
    //--------------------------------------------------------------------------------------------
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //AF_INET é o ipv4, SOCK_STREAM O ipv4
    if (sockfd < 0) {                         
        error("error opening socket");
    }
    //--------------------------------------------------------------------------------------------
    // Conversa com o servidor
    //--------------------------------------------------------------------------------------------
    do {
        printf("http:// (exit para sair): ");
        memset(buffer, 0, 256);
        scanf("%255s", buffer);

        if(strcmp(buffer, "exit") == 0) {
            printf("saindo...\n");
            break;
        }

        // Limpa \n
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';

        char mensagem[BUFFER_SIZE];
        http dados = prepara_http(buffer);

        server = gethostbyname(dados.host);
        if (server == NULL) {
            printf("Host não existe\n");
            continue;
        }

        memset((char *) &serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], server->h_length); 
        serv_addr.sin_port = htons(portno);

        // <<< CRIAÇÃO DO SOCKET AQUI >>> 
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("Erro ao criar socket");
            continue;
        }

        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
            perror("error connecting");
            close(sockfd);
            continue;
        }

        constroi_requisicao_http(dados, mensagem, sizeof(mensagem));

        ssize_t num_bytes_msg = write(sockfd, mensagem, strlen(mensagem));
        if (num_bytes_msg < 0) {
            perror("error writing to socket");
        }

        recebe_http(sockfd, dados.caminho);

        close(sockfd);  // Fecha corretamente no final da iteração

    } while(true);
        return 0;
}
