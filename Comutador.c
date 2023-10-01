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

int sockS, sockC;

struct addrinfo *Saddr;
struct addrinfo *Caddr;

struct sockaddr_storage Srecv_addr;
socklen_t Saddr_len=sizeof Srecv_addr;

struct sockaddr_storage Crecv_addr;
socklen_t Caddr_len=sizeof Crecv_addr;


void Comutacao(){
    puts("Comutador em funcionamento");
    int sendMsg,recvMsg;
    char buffer[BUFSIZE];
    int i,cont=0;  
    while(cont<2){
        //exucultar somente duas(2) vezes ja é o suficiente para trocar as mensagens "SYN","SYNACK","TESTE" e "TESTANDO";
        cont++;
        for ( i = 0; i < 10; i++){
        buffer[i]='\0';
        }
        //Recebe da origem
        recvMsg=recvfrom(sockS,buffer,BUFSIZE,0,(struct sockaddr*)&Srecv_addr, &Saddr_len);
        //Envia para o destino
        sendMsg=sendto(sockC,buffer,BUFSIZE,0,Caddr->ai_addr,Caddr->ai_addrlen);
        for ( i = 0; i < 10; i++){
            buffer[i]='\0';
        }
        //Recebe do destino
        recvMsg=recvfrom(sockC,buffer,BUFSIZE,0,(struct sockaddr*)&Crecv_addr, &Caddr_len);
        //Envia para a origem
        sendMsg=sendto(sockS,buffer,BUFSIZE,0,(struct sockaddr*)&Srecv_addr,Saddr_len);
    }
}


//socket para Cliente
int CMeuSocket(const char *server, const char *service){

    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(server, service, &addrCriteria, &servAddr);
    if (rtnVal != 0)
    {
        {
            perror("getaddrinfo() falhou\n");
            exit(EXIT_FAILURE);
        }
    }

    int sock = -1;
    for (Caddr = servAddr; Caddr != NULL; Caddr = Caddr->ai_next){
        sock = socket(Caddr->ai_family, Caddr->ai_socktype, Caddr->ai_protocol);
        if (sock < 0)
            continue;
        break;
        close(sock);
        sock = -1;
    }
    freeaddrinfo(servAddr);
    return sock;
}

//socket para servidor
int SMeuSocket(const char *service){
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
    for (Saddr = servAddr; Saddr != NULL; Saddr = Saddr->ai_next){
        servSock = socket(Saddr->ai_family, Saddr->ai_socktype, Saddr->ai_protocol);
        if (servSock < 0)
            continue;

        if ((bind(servSock, Saddr->ai_addr, Saddr->ai_addrlen) == 0)){
            break;
        }

        close(servSock);
        servSock = -1;
    }

    freeaddrinfo(servAddr);

    return servSock;
}

int main(int argc, char const *argv[]){
    if (argc != 4){
        perror("Execucao correta: ./comudador <porta Origem> <ip> <porta Destino>\n");
        return EXIT_FAILURE;
    }

    int estado_atual = iniciando;
    
    while (estado_atual != encerrado){
        switch (estado_atual){
            case iniciando:
                sockS = SMeuSocket(argv[1]);
                sockC = CMeuSocket(argv[2],argv[3]);
                estado_atual = comunicando;
                break;
            case comunicando:
                Comutacao();
                estado_atual = finalizando;
                break;
            case finalizando:
                estado_atual = encerrado;
                close(sockS);
                close(sockC);
                break;
            default:
                break;
        }
    }
    puts("OK");
    return EXIT_SUCCESS;
} 
