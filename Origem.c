#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

enum estados
{
    iniciando,
    comunicando,
    finalizando,
    encerrado
};

static const int BUFSIZE = 512;

struct addrinfo *addr;

const char *server;
const char *service;

//"cliente"
int meuSend(int socket,char *mens,int tamMen){   
    int result=sendto(socket,mens,tamMen,0,addr->ai_addr,addr->ai_addrlen);
    return result;
}
int meuRecv(int socket,char *buffer,int BUFSIZE,int zero,struct sockaddr *recv_addr,int *addr_len){
    int result=recvfrom(socket,buffer,BUFSIZE,zero,(struct sockaddr*)&recv_addr, addr_len);
    return result;
}

int MeuConnect(int socket){
	printf("Pedido de conexao\n");
	int sendMsg,recvMsg;
    const char *mens = "SYN";
	char buffer[BUFSIZE];

    struct sockaddr_storage recv_addr;
    socklen_t addr_len=sizeof recv_addr;

	sendMsg=sendto(socket,mens,sizeof(&mens),0,addr->ai_addr,addr->ai_addrlen);
    if(sendMsg<0){
	   perror("sendto() falhou");
       return EXIT_FAILURE;
    }
    puts("Origem enviou SYN");
    for (int i = 0; i < 10; i++){
            buffer[i]='\0';
    }
	recvMsg=recvfrom(socket,buffer,BUFSIZE,0,(struct sockaddr *)&recv_addr, &addr_len);
    if(recvMsg<0){
		perror("recvfrom() falhou\n");
        exit(EXIT_FAILURE);
	}
    if(strcmp(buffer,"SYNACK")==0){
        puts("Origem recebeu SYNACK");
        puts("Conexao Estabelecida");
        char*mens2="TESTE";
		sendMsg=meuSend(socket,mens2,sizeof(&mens2));
        if(sendMsg<0){
            perror("meuSend() falhou");
            return EXIT_FAILURE;
        }
        puts("Origem enviou TESTE");
        for (int i = 0; i < 10; i++){
            buffer[i]='\0';
        }
        recvMsg=meuRecv(socket,buffer,BUFSIZE,0,(struct sockaddr *)&recv_addr, &addr_len);
        if(recvMsg<0){
            perror("meuRecv() falhou");
            return EXIT_FAILURE;
        }
        if(strcmp(buffer,"TESTANDO")==0){
            puts("Origem recebeu TESTANDO");
        }
        return 0;
	}
	return -1;
}

int MeuSocket(const char *server, const char *service){

    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(server, service, &addrCriteria, &servAddr);
    if (rtnVal != 0){
        {
            perror("getaddrinfo() falhou\n");
            exit(EXIT_FAILURE);
        }
    }

    int sock = -1;
    for (addr = servAddr; addr != NULL; addr = addr->ai_next){
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock < 0)
            continue;
        if(MeuConnect(sock) == 0){
        	break;
        }
        close(sock);
        sock = -1;
    }
    freeaddrinfo(servAddr);
    return sock;
}

int main(int argc, char const *argv[]){
    if (argc !=3){
        perror("Execucao correta: ./origem <ip> <porta>\n");
        return EXIT_FAILURE;
    }

    int result;
    server = argv[1];
    service = argv[2];

    int estado_atual = iniciando;
    int sock;

    while (estado_atual != encerrado){
        switch (estado_atual){
            case iniciando:
                sock = MeuSocket(server,service);
                estado_atual = comunicando;
                break;
            case comunicando:
                estado_atual = finalizando;
                break;
            case finalizando:
                close(sock);
                estado_atual = encerrado;
                break;
            default:
                break;
        }
    }
    puts("OK");
    return EXIT_SUCCESS;
}