#include "Utils/http.h"

// Função principal (main) que simula a interação cliente-servidor
int main() {
    // Criando dados para a requisição
    http requisicao_cliente;
    strcpy(requisicao_cliente.metodo, "GET");
    strcpy(requisicao_cliente.caminho, "/index.html");
    strcpy(requisicao_cliente.host, "www.exemplo.com");
    strcpy(requisicao_cliente.protocolo, "HTTP/1.1");

    printf("--------------------------------------------------------------------------\n");
    printf("estrutura http de requisicao criada com sucesso! iniciando testes... \n\n");
    // Construir a requisição HTTP do cliente
    char mensagem_cliente[BUFFER_SIZE];
    constroi_requisicao_http(requisicao_cliente, mensagem_cliente, sizeof(mensagem_cliente));
    printf("Requisição HTTP do Cliente:\n%s\n", mensagem_cliente);

    // Simulando o servidor recebendo a requisição e interpretando
    http requisicao_servidor = interpreta_requisicao_http(mensagem_cliente);
    printf("Interpretação da Requisição no Servidor:\n");
    printf("Metodo: %s\n", requisicao_servidor.metodo);
    printf("Caminho: %s\n", requisicao_servidor.caminho);
    printf("Host: %s\n", requisicao_servidor.host);
    printf("Protocolo: %s\n", requisicao_servidor.protocolo);

    printf("\n--------------------------------------------------------------------------\n");
    printf("estrutura http de resposta criada com sucesso! iniciando testes... \n\n");

    // Construir a resposta HTTP do servidor
    http resposta_servidor;
    strcpy(resposta_servidor.protocolo, "HTTP/1.1");
    resposta_servidor.codigo_status = 400;
    strcpy(resposta_servidor.mensagem_status, "BAD REQUEST");
    strcpy(resposta_servidor.tipo, "text/plain; charset=utf-8");
    resposta_servidor.tamanho_conteudo = strlen("Hello, world!");

    char mensagem_resposta[BUFFER_SIZE];
    constroi_resposta_http(resposta_servidor, mensagem_resposta, sizeof(mensagem_resposta));
    printf("Resposta HTTP do Servidor:\n%s\n", mensagem_resposta);

    // Simulando o cliente recebendo a resposta e interpretando
    http resposta_cliente = interpreta_resposta_http(mensagem_resposta);
    if(resposta_cliente.valido) {
        printf("Interpretação da Resposta no Cliente:\n");
        printf("Protocolo: %s\n", resposta_cliente.protocolo);
        printf("Código: %d\n", resposta_cliente.codigo_status);
        printf("Mensagem: %s\n", resposta_cliente.mensagem_status);
        printf("Tipo: %s\n", resposta_cliente.tipo);
        printf("Tamanho: %zu\n", resposta_cliente.tamanho_conteudo);
    }
    else {
        printf("ERRO:: %s\n", resposta_cliente.mensagem_erro);
    }
    return 0;
}
