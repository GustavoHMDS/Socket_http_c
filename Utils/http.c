#include "http.h"

void constroi_requisicao_http(http dados_envio, char mensagem[], size_t tamanho_mensagem) {
// Monta a requisição HTTP
    memset(mensagem, 0, tamanho_mensagem);
    snprintf(mensagem, tamanho_mensagem,
             "%s %s %s\r\n" // metodo caminho protocolo (GET /gato.jpeg HTTP/1.1)
             "Host: %s\r\n" // host (localhost)
             "\r\n", //finaliza o cabeçalho
             dados_envio.metodo, dados_envio.caminho, dados_envio.protocolo,
             dados_envio.host);
}

http interpreta_requisicao_http(char mensagem[]) {
    http dados_recebidos;

    // Verifica a primeira linha
    int num_lidos = sscanf(mensagem, "%s %s %s", dados_recebidos.metodo, dados_recebidos.caminho, dados_recebidos.protocolo);
    if(num_lidos == 3) { // o ultimo %s é apenas para garantir que tem uma mensagem
        char *protocolo_ptr = strstr(mensagem, "HTTP");
        if(protocolo_ptr) {
            if(strcmp(protocolo_ptr, "HTTP/1.1") == 0) {
                strcpy(dados_recebidos.mensagem_erro, "Essa versão do protocolo HTTP não é suportada.");
                return dados_recebidos;
            }
        }
    }
    // Verifica a segunda linha
    char *host_ptr = strstr(mensagem, "Host: ");
    if (host_ptr) {
        sscanf(host_ptr, "Host: %s", dados_recebidos.host);
    }

    return dados_recebidos;
}

void constroi_resposta_http(http dados_envio, char mensagem[], size_t tamanho_mensagem) {
    // Monta a resposta HTTP
    memset(mensagem, 0, tamanho_mensagem);
    if(dados_envio.codigo_status == 200) {
        snprintf(mensagem, tamanho_mensagem,
                "%s %d %s\r\n" // protocolo codigo mensagem (http/1.1 200 OK)
                "Content-Type: %s\r\n" // tipo (text/plain image/jpeg)
                "Content-Length: %lu\r\n" //tamanho (4096)
                "\r\n",
                dados_envio.protocolo, dados_envio.codigo_status, dados_envio.mensagem_status,
                dados_envio.tipo, 
                dados_envio.tamanho_conteudo);
    }
    else {
        snprintf(mensagem, tamanho_mensagem,
                "%s %d %s\r\n" // protocolo codigo mensagem (http/1.1 405 BAD REQUEST)
                "\r\n",
                dados_envio.protocolo, dados_envio.codigo_status, dados_envio.mensagem_status);
    }

}

http interpreta_resposta_http(char mensagem[]) {
    http dados_recebidos;
    dados_recebidos.valido = false;
    
    // Usa sscanf para pegar os principais componentes da resposta HTTP
    int num_lidos = sscanf(mensagem, "%15s %d %255[^\r\n]", dados_recebidos.protocolo, &dados_recebidos.codigo_status, dados_recebidos.mensagem_status);
    if(num_lidos == 3) { // o ultimo %s é apenas para garantir que tem uma mensagem
        char *protocolo_ptr = strstr(mensagem, "HTTP");
        if(protocolo_ptr) {
            if(strcmp(protocolo_ptr, "HTTP/1.1") == 0) {
                strcpy(dados_recebidos.mensagem_erro, "Essa versão do protocolo HTTP não é suportada.");
                return dados_recebidos;
            }
        }
        else {
            strcpy(dados_recebidos.mensagem_erro, "Não encontrou a versão do protocolo HTTP.");
            return dados_recebidos;
        }
        if(dados_recebidos.codigo_status < 100 || dados_recebidos.codigo_status > 599) {
            strcpy(dados_recebidos.mensagem_erro, "Codigo de status inválido.");
            return dados_recebidos;
        }
    } 
    else {
        strcpy(dados_recebidos.mensagem_erro, "Não encontrou um parâmetro obrigatório.");
        return dados_recebidos;
    }
    // Encontrar o cabeçalho "Content-Type" e "Content-Length"
    char *tipo_ptr = strstr(mensagem, "Content-Type: ");
    if (tipo_ptr) {
        sscanf(tipo_ptr, "Content-Type: %[^\r\n]", dados_recebidos.tipo);
    }

    char *tamanho_ptr = strstr(mensagem, "Content-Length: ");
    if (tamanho_ptr) {
        sscanf(tamanho_ptr, "Content-Length: %zu", &dados_recebidos.tamanho_conteudo); 
    }
    else {
        strcpy(dados_recebidos.mensagem_erro, "Não encontrou um parâmetro obrigatório.");
        return dados_recebidos;
    }

    dados_recebidos.valido = true;
    return dados_recebidos;
}