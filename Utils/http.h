#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

typedef struct {
    char metodo[16]; // GET, POST, etc.
    char metodos_aceitos[16]; // apenas as rquisicoes que ele aceita, como GET
    char caminho[128]; // Caminho do arquivo que ele quer acessar
    char host[128]; // www.exemplo.com
    char protocolo[16]; // Protocolo, como HTTP/1.1
    int codigo_status; // codigo do status como 200, 404, etc
    char mensagem_status[32]; // Mensagem de status, como OK, NOT_FOUND, etc
    char tipo[32]; // Tipo de retorno, como text/html
    size_t tamanho_conteudo; // Tamanho do retorno
    bool valido; // Define se a mensagem continha os parametros corretos
    char mensagem_erro[128]; // apenas se valido = false
} http;

void constroi_requisicao_http(http dados_envio, char mensagem[], size_t tamanho_mensagem); // pedido cliente
http interpreta_requisicao_http(char mensagem[]); // servidor entende o pedido
void constroi_resposta_http(http dados_envio, char mensagem[], size_t tamanho_mensagem); // resposta servidor
http interpreta_resposta_http(char mensagem[]); //servidor entende a resposta

#endif