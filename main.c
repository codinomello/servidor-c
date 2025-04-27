#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_PATH 256

// Função para enviar uma resposta HTTP
void send_response(int client_socket, const char *status, const char *content_type, const char *body) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %lu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             status, content_type, strlen(body), body);
    write(client_socket, response, strlen(response));
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char path[MAX_PATH];

    // Passo 1: Criar o socket do servidor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Passo 2: Configurar opções do socket (reutilizar porta)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Erro ao configurar opções do socket");
        exit(EXIT_FAILURE);
    }

    // Passo 3: Configurar endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Aceita conexões de qualquer IP
    server_addr.sin_port = htons(PORT);       // Porta 8080

    // Passo 4: Vincular o socket ao endereço
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao vincular socket");
        exit(EXIT_FAILURE);
    }

    // Passo 5: Escutar conexões (fila de até 10 conexões pendentes)
    if (listen(server_fd, 10) < 0) {
        perror("Erro ao escutar");
        exit(EXIT_FAILURE);
    }

    printf("Servidor rodando na porta %d...\n", PORT);

    // Passo 6: Loop principal para aceitar conexões
    while (1) {
        // Aceitar conexão do cliente
        client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Erro ao aceitar conexão");
            continue;
        }

        // Passo 7: Ler a requisição do cliente
        memset(buffer, 0, BUFFER_SIZE);
        read(client_socket, buffer, BUFFER_SIZE - 1);
        printf("Requisição recebida:\n%s\n", buffer);

        // Passo 8: Extrair o método e o caminho da requisição
        char method[16], protocol[16];
        sscanf(buffer, "%s %s %s", method, path, protocol);

        // Passo 9: Processar a requisição e enviar resposta
        if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
            const char *html = "<html><body><h1>Bem-vindo ao Servidor HTTP em C!</h1></body></html>";
            send_response(client_socket, "200 OK", "text/html", html);
        } else {
            const char *html = "<html><body><h1>404 Not Found</h1></body></html>";
            send_response(client_socket, "404 Not Found", "text/html", html);
        }

        // Passo 10: Fechar o socket do cliente
        close(client_socket);
    }

    // Passo 11: Fechar o socket do servidor (nunca alcançado neste exemplo)
    close(server_fd);
    return 0;
}