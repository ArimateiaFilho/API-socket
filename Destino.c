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
struct sockaddr_storage recv_addr;
socklen_t addr_len=sizeof recv_addr;

int meuSend(int socket,char *mens,int tamMen){   
    int result=sendto(socket,mens,tamMen,0,(struct sockaddr*)&recv_addr,addr_len);
    return result;
}
int meuRecv(int socket,char *buffer,int BUFSIZE){
    int result=recvfrom(socket,buffer,BUFSIZE,0,(struct sockaddr*)&recv_addr, &addr_len);
    return result;
}

int MeuListen(){
	printf("Iniciando Escuta\n");
	return 0;
}

int MeuAccept(int socket, struct addrinfo *addr){
	puts("Esperando Conexao");
	int sendMsg,recvMsg;
	char *mens = "SYNACK";
	char buffer[BUFSIZE];

    for (int i = 0; i < 10; i++){
            buffer[i]='\0';
    }

	recvMsg=recvfrom(socket,buffer,BUFSIZE,0,(struct sockaddr*)&recv_addr, &addr_len);

    if(recvMsg<0){
        perror("recvfrom() falhou");
        EXIT_FAILURE;
    }

	if(strcmp(buffer,"SYN")==0){
		puts("Destino recebeu SYN");
        puts("Conexao Pedida");	
        sendMsg = sendto(socket,mens,sizeof(&mens),0,(struct sockaddr*)&recv_addr,addr_len);
        if(sendMsg<0){
            perror("sendto() falhou");
            return EXIT_FAILURE;
        }
        puts("Destino enviou SYNACK");
        puts("Conexao Aceita");
        
        for (int i = 0; i < 10; i++){
            buffer[i]='\0';
        }

        recvMsg=meuRecv(socket,buffer,BUFSIZE);
        if(recvMsg<0){
            perror("meuRecv() falhou");
            EXIT_FAILURE;
        }
        if(strcmp(buffer,"TESTE")==0){
            puts("Destino recebeu TESTE");
            char *mens2="TESTANDO";
            sendMsg = meuSend(socket,mens2,sizeof(&mens2));
            if(sendMsg<0){
                perror("sendto() falhou");
                return EXIT_FAILURE;
            }
            puts("Destino enviou TESTANDO");       
        }

	}
}

int MeuSocket(const char *service){
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria)); 
    addrCriteria.ai_family = AF_UNSPEC;             
    addrCriteria.ai_flags = AI_PASSIVE;             
    addrCriteria.ai_socktype = SOCK_DGRAM;         
    addrCriteria.ai_protocol = IPPROTO_UDP;        

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    if (rtnVal != 0){
        perror("getaddrinfo() falhou\n");
        exit(EXIT_FAILURE);
    }

    int servSock = -1;
    for (addr = servAddr; addr != NULL; addr = addr->ai_next){
        servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (servSock < 0)
            continue;

        if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) && (MeuListen() == 0)){
            MeuAccept(servSock,addr);
            break;
        }

        close(servSock);
        servSock = -1;
    }

    freeaddrinfo(servAddr);

    return servSock;
}


int main(int argc, char const *argv[])
{
    if (argc != 2){
        perror("Execucao correta: ./destino <porta>\n");
        return EXIT_FAILURE;
    }

    int estado_atual = iniciando;
    int servSock;

    while (estado_atual != encerrado){
        switch (estado_atual){
            case iniciando:
                servSock = MeuSocket(argv[1]);
                estado_atual = comunicando;
                break;
            case comunicando:
                estado_atual = finalizando;
                break;
            case finalizando:
                estado_atual = encerrado;
                close(servSock);
                break;
            default:
                break;
        }
    }
    puts("OK");
    return EXIT_SUCCESS;
} 
