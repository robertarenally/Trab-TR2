#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "analyzer_web_proxy.hpp"

void* getDadoCliente(void*);
char* converte_Request_to_string(struct PedidoAnalisado *pedido);
int createServerSocket(char *pcAddress, char *pcPort);
void writeToServerSocket(const char* bufferServer,int socketfd,int sizeBuffer);
void writeToClientSocket(const char* bufferServer,int socketfd,int sizeBuffer);
void writeToClient (int Clientfd, int Serverfd);
void showRequestClientHttp(struct PedidoAnalisado *pedido);

using namespace std;

int main (int argc, char *argv[]) 
{
	//descritores 
	int sockfd,newsockfd;
	//retornos
	int binded, sizeClient, pid;
	//numero da porta
	int numPort;
	//enderecos
	struct sockaddr_in endereco_servidor_proxy; 
	struct sockaddr endereco_cliente;

	if (argc<2) 
  	{
  		fprintf(stderr,"Programa encerrado! Por favor, forneca uma porta! \n");
  		return 1;
  	}

  	sockfd = socket(AF_INET, SOCK_STREAM, 0);   // cria um socket

  	if (sockfd<0) {
  		fprintf(stderr,"Nao foi possivel criar um socket. O programa foi encerrado! \n");
  		return 1;
  	}

  	memset(&endereco_servidor_proxy,0,sizeof(endereco_servidor_proxy));

  	numPort = atoi(argv[1]);           									// argumento passado pela linha de comando
 	endereco_servidor_proxy.sin_family = AF_INET;     					// utiliza o protocolo IPv4
	endereco_servidor_proxy.sin_addr.s_addr = inet_addr("127.0.0.1");	// seta o endereco IP
 	endereco_servidor_proxy.sin_port = htons(numPort); 					// seta a porta que ficara escutando por conexoes

 	// Anexa um endereço local a um socket
 	binded = bind(sockfd, (struct sockaddr *)&endereco_servidor_proxy, sizeof(endereco_servidor_proxy));

 	if (binded <0 ) {
 		fprintf(stderr,"Erro ao anexar um endereco local a um socket! O programa foi encerrado \n");
  		return 1;
 	}
  	// Torna o servidor proxy apto para receber conexoes do cliente
  	listen(sockfd, 100);  // aloca uma fila de tamanho 100 para conexões pendentes do browser

  	sizeClient = sizeof(struct sockaddr);


  	while(1) {
  		
  		// bloqueia a execucao do programa, ate que exista um pedido de conexao por parte do cliente
  		newsockfd = accept(sockfd,&endereco_cliente, (socklen_t*) &sizeClient); 

  		if (newsockfd <0){
  			printf("Erro ao aceitar pedido de conexao!\n");
 		}

 		pid = fork();

 		if(pid == 0){

 			getDadoCliente((void*)&newsockfd);
 			close(newsockfd);
 			_exit(0);
 		}else{
 			close(newsockfd);     // pid =1 parent process
 		}
 	}

 	close(sockfd);
	return 0;
}
char* converte_Request_to_string(struct PedidoAnalisado *pedido)
{
	char *serverRequest, *cabecalhoBuffer;
	int sizeRequest, sizeCabecalho;
	/* Seta o cabecalho */
	CabecalhoDoPedido_set(pedido, "Host", pedido->host);
	CabecalhoDoPedido_set(pedido, "Connection", "close");

	sizeCabecalho = CabecalhoDoPedido_size(pedido);
	cabecalhoBuffer = (char*) malloc(sizeCabecalho + 1);

	if (cabecalhoBuffer == NULL) {
		fprintf(stderr," Erro na alocacao do cabecalhoBuffer! O programa foi encerrado \n");
		exit (1);
	}

	recupera_cabecalho_PedidoHTTP(pedido, cabecalhoBuffer, sizeCabecalho);
	cabecalhoBuffer[sizeCabecalho] = '\0';

	sizeRequest = strlen(pedido->method) + strlen(pedido->path) + strlen(pedido->version) + sizeCabecalho + 4;
	serverRequest = (char *) malloc(sizeRequest + 1);

	if(serverRequest == NULL){
		fprintf(stderr," Erro na alocacao do serverRequest! O programa foi encerrado\n");
		exit (1);
	}
	serverRequest[0] = '\0';
	strcpy(serverRequest, pedido->method);
	strcat(serverRequest, " ");
	strcat(serverRequest, pedido->path);
	strcat(serverRequest, " ");
	strcat(serverRequest, pedido->version);
	strcat(serverRequest, "\r\n");
	strcat(serverRequest, cabecalhoBuffer);
	free(cabecalhoBuffer);
	return serverRequest;
}
int createServerSocket(char *pcAddress, char *pcPort) {
  
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  int idSocket;
  //Obtem informações de endereço para o soquete de fluxo na porta de entrada
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(pcAddress, pcPort, &host_info, &host_info_list) != 0) {
   		fprintf(stderr," Erro no formato do endereco do servidor! O programa foi encerrado\n");
		exit (1);
  }
  //cria um socket
  if ((idSocket = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol)) < 0) 
  {
    	fprintf(stderr," Erro ao criar socket para o servidor! O programa foi encerrado\n");
		exit (1);
  }
  //faz a conecção
  if (connect(idSocket, host_info_list->ai_addr, host_info_list->ai_addrlen) < 0)
  {
    	fprintf(stderr," Erro ao tentar conectar o servidor! O programa foi encerrado\n");
		exit (1);
  }
  freeaddrinfo(host_info_list);
  return idSocket;
}
void writeToServerSocket(const char* bufferServer,int socketfd,int sizeBuffer)
{

	string temp;

	temp.append(bufferServer);
	
	int totalSent = 0;

	int numSent;

	while (totalSent < sizeBuffer) {
		if ((numSent = send(socketfd, (void *) (bufferServer + totalSent), sizeBuffer - totalSent, 0)) < 0) {
			fprintf(stderr," Erro ao enviar para o servidor!\n");
			exit (1);
		}
		totalSent += numSent;

	}	

}
void writeToClientSocket(const char* bufferServer,int socketfd,int sizeBuffer)
{
	string temp;

	temp.append(bufferServer);
	
	int totalSent = 0;

	int numSent;

	while (totalSent < sizeBuffer) {
		if ((numSent = send(socketfd, (void *) (bufferServer + totalSent), sizeBuffer - totalSent, 0)) < 0) {
			fprintf(stderr," Erro ao receber do servidor!\n");
			exit (1);
		}
		totalSent += numSent;

	}	
}
void writeToClient (int Clientfd, int Serverfd) 
{
	int sizeBuffer = 5000;
	int iRecv;
	char buffer[sizeBuffer];
	while ((iRecv = recv(Serverfd, buffer, sizeBuffer, 0)) > 0) {
	    writeToClientSocket(buffer, Clientfd,iRecv);         // writing to client	  
		//printf("----------------------------------------------------\n");
		//printf("Resposta do servidor:\n");
		//printf("%s", buffer);
		//printf("----------------------------------------------------\n");
		memset(buffer,0,sizeof (buffer));	
	}      
	if (iRecv < 0) {
	  fprintf(stderr,"Erro enquanto recebia do servidor!\n");
	  exit (1);
	}
}
void* getDadoCliente(void* socketid)
{
	int sizeBuffer = 5000;

	char buffer[sizeBuffer];

	int newsockfd = *((int*)socketid);

	int total_de_bits_recebidos = 0, recvd;

	int iServerfd;

	char *mensagem;  // armazena a mensagem da URL

	char *browser_request;

	mensagem = (char *) malloc(sizeBuffer); 

	if (mensagem == NULL) {
		fprintf(stderr,"Erro durante a alocacao de memoria. O programa foi encerrado\n");
		exit (1);
	}	

	mensagem[0] = '\0';

	while (strstr(mensagem, "\r\n\r\n") == NULL) {  // determines end of request

	  recvd = recv(newsockfd, buffer, sizeBuffer, 0) ;

	  if(recvd < 0 ){
	  	fprintf(stderr," Erro ao receber mensagem do cliente!\n");
		exit (1);
	  				
	  }else if(recvd == 0) {
	  		break;
	  } else {
	  	total_de_bits_recebidos += recvd;
	  	//Se o tamanho da mensagem for maior que o tamanho da string buffer, dobra o tamanho da string
	  	buffer[recvd] = '\0';
	  	if (total_de_bits_recebidos > sizeBuffer) {
			sizeBuffer *= 2;
			mensagem = (char *) realloc(mensagem, sizeBuffer);
			if (mensagem == NULL) {
				fprintf(stderr," Erro durante a realocação de memoria!\n");
				exit (1);
			}
		}
	  }
	  strcat(mensagem, buffer);
	}
	//printf("%s\n",mensagem);
	struct PedidoAnalisado *pedido;    // contem o pedido analisado

	pedido = PedidoAnalisado_create();

	if (Analise_do_pedido(pedido, mensagem, strlen(mensagem)) < 0) {		
		//fprintf(stderr,"Erro na mensagem de pedido, apenas http e get com cabecalhos sao permitido!\n");
		exit(0);
	}
	//Se a porta não foi setada na mensagem URL, coloquei como padrao a porta 8228
	if (pedido->port == NULL)             
		 pedido->port = (char *) "80";
	showRequestClientHttp(pedido);
	//pedido final para ser enviado
	browser_request  = converte_Request_to_string(pedido);		
	iServerfd = createServerSocket(pedido->host, pedido->port);
	writeToServerSocket(browser_request, iServerfd, total_de_bits_recebidos);
	writeToClient(newsockfd, iServerfd);
	PedidoAnalisado_destroy(pedido);	
	close(newsockfd);   
	close(iServerfd);
	int y = 3;
	int *p = &y;
	return p;
}

void showRequestClientHttp(struct PedidoAnalisado *pedido)
{
	printf("----------------------------------------------------\n");
	printf("Request HTTP interceptado do browser:\n");
	printf("method: %s\n", pedido->method);
	printf("url: %s\n", pedido->path);
	printf("protocol: %s\n", pedido->protocol);
	printf("host: %s\n", pedido->host);
	printf("port: %s\n", pedido->port);
	printf("version: %s\n", pedido->version);
	printf("buffer: %s\n", pedido->buf);
	printf("tamanho do buffer: %zu\n", pedido->buflen);
	printf("key: %s\n", pedido->headers->key);
	printf("sizeKey: %zu\n", pedido->headers->sizeKey);
	printf("value: %s\n", pedido->headers->value);
	printf("sizeValue: %zu\n", pedido->headers->sizeValue);
	printf("----------------------------------------------------\n");
}