#include "servidor.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int pega_conteudo_diretorio(char *mensagem) {
    DIR *diretorio;
    struct dirent *conteudo_diretorio;
    const char *path = "./Servidor/ItensServidor/";
    char buffer[BUFFER_SIZE] = {0};
    size_t pos = 0;

    diretorio = opendir(path);
    if(!diretorio) {
        perror("Erro ao abrir diretório");
        return -1;
    }

    while((conteudo_diretorio = readdir(diretorio)) != NULL) {
        if(strcmp(conteudo_diretorio->d_name, ".") == 0 || strcmp(conteudo_diretorio->d_name, "..") == 0)
            continue;

        size_t tamanho_nome = strlen(conteudo_diretorio->d_name);
        if(pos + tamanho_nome + 2 >= BUFFER_SIZE) {
            break;
        }

        strcpy(&buffer[pos], conteudo_diretorio->d_name);
        pos += tamanho_nome;
        buffer[pos++] = '\n';
        buffer[pos] = '\0';
    }

    closedir(diretorio);
    strcpy(mensagem, buffer);
    return pos;  // retorna tamanho real do conteúdo
}

void envia_arquivo(int sockfd, const char *caminho) {
    http dados_resposta;
    char buffer[BUFFER_SIZE];

    char caminho_completo[256] = "./Servidor/ItensServidor/";
    strcat(caminho_completo, caminho);
    
    // Imprime o caminho completo para garantir que está correto
    FILE *arquivo = fopen(caminho_completo, "rb");  // Abre o arquivo de imagem em modo binário
    if(arquivo == NULL) {
        dados_resposta.codigo_status = 404;
        strcpy(dados_resposta.mensagem_status, "Not Found");
        constroi_resposta_http(dados_resposta, buffer, sizeof(buffer)); 
        close(sockfd);
        return;
    }
    else {
        fseek(arquivo, 0, SEEK_END);      // vai pro fim do arquivo
        dados_resposta.tamanho_conteudo = ftell(arquivo);
        char *tipo = strstr(caminho, ".jpeg");
        if(strstr(caminho, ".jpeg") || strstr(caminho, ".jpg")) {
            strcpy(dados_resposta.tipo, "image/jpeg");
        }
        else {
            strcpy(dados_resposta.tipo, "text/plain");
        }
        strcpy(dados_resposta.protocolo, "HTTP/1.1");
        strcpy(dados_resposta.mensagem_status, "OK");
        dados_resposta.codigo_status = 200;
        constroi_resposta_http(dados_resposta, buffer, sizeof(buffer));
        if (send(sockfd, buffer, strlen(buffer), 0) == -1) {
            error("Erro ao enviar o arquivo");
        }
    }
    
    fseek(arquivo, 0, SEEK_SET);
    size_t num_bytes_lidos;

    // Lê e envia o arquivo em pedaços
    while((num_bytes_lidos = fread(buffer, 1, BUFFER_SIZE, arquivo)) > 0) {
        // Envia o pedaço lido para o cliente
        if (send(sockfd, buffer, num_bytes_lidos, 0) == -1) {
            error("Erro ao enviar o arquivo");
        }
    }
    fclose(arquivo);  // Fecha o arquivo
}

// Função que trata a comunicação com o cliente
void *tratar_cliente(void *arg) {
    http dados_requisicao;
    http dados_resposta;
    int newsockfd = *((int *)arg);  // Extrai o descritor de socket do argumento
    free(arg);  // Libera a memória alocada para o descritor

    char buffer[BUFFER_SIZE];
    int num_bytes_msg;

    // Limpeza do buffer
    memset(buffer, 0, sizeof(buffer));

    // Lê a mensagem do cliente
    num_bytes_msg = read(newsockfd, buffer, sizeof(buffer));
    if (num_bytes_msg < 0) {
        error("ERROR reading from socket");
    }
    dados_requisicao = interpreta_requisicao_http(buffer);
    strcpy(dados_resposta.protocolo, "HTTP/1.1");
    if(strcmp(dados_requisicao.metodo, "GET") != 0) {
        dados_resposta.codigo_status = 405;
        strcpy(dados_resposta.mensagem_status, "Method Not Allowed");
        constroi_resposta_http(dados_resposta, buffer, sizeof(buffer)); 
        if (send(newsockfd, buffer, strlen(buffer), 0) == -1) {
            error("Erro ao enviar o arquivo");
        }
    }
    if(strcmp(dados_requisicao.protocolo, "HTTP/1.1")) {
        dados_resposta.codigo_status = 505;
        strcpy(dados_resposta.mensagem_status, "HTTP Version Not Supported");
        constroi_resposta_http(dados_resposta, buffer, sizeof(buffer)); 
        if (send(newsockfd, buffer, strlen(buffer), 0) == -1) {
            error("Erro ao enviar o arquivo");
        }  
    }
    if(strcmp(dados_requisicao.caminho, "/") == 0) {
        char conteudo[BUFFER_SIZE];

        strcpy(dados_resposta.mensagem_status, "OK");
        dados_resposta.codigo_status = 200;
        strcpy(dados_resposta.tipo, "text/plain");
        dados_resposta.tamanho_conteudo = pega_conteudo_diretorio(conteudo);
        constroi_resposta_http(dados_resposta, buffer, sizeof(buffer));
        // Envia o Header
        if (send(newsockfd, buffer, strlen(buffer), 0) == -1) {
            error("Erro ao enviar o arquivo");
        }
        // Envia o conteudo
        if (send(newsockfd, conteudo, dados_resposta.tamanho_conteudo, 0) == -1) {
            error("Erro ao enviar o arquivo");
        }
    }
    else {
        envia_arquivo(newsockfd, dados_requisicao.caminho);
    }
    return NULL;
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
    // Espera uma conexão e abre uma thread para o cliente conversar
    //--------------------------------------------------------------------------------------------
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while(true) {
        // Aceita uma conexão
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            error("ERROR on accept");
        }
        printf("Nova Conexao\n");
        // Aloca memória para passar o descritor do socket para a thread
        int *new_sock = malloc(sizeof(int));
        *new_sock = newsockfd;

        // Cria uma thread para tratar a comunicação com o cliente
        pthread_t tid;
        if (pthread_create(&tid, NULL, tratar_cliente, (void *)new_sock) != 0) {
            error("ERROR creating thread");
        }

        // Opcional: Desanexa a thread para que ela seja limpa automaticamente ao terminar
        pthread_detach(tid);
    }
    
    close(sockfd);
    return 0; 
}