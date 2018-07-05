#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
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
#include <err.h>
#include <stdarg.h>

#include "analyzer_web_proxy.hpp"


int opcao;

/**
	Função responsavel receber o request HTTP do browser
	e chamar as funções da biblioteca "analyzer_web_proxy.hpp"
	Essa função também chama as demais funções listadas abaixo
**/
void* getRequestHTTP(void*);
/**
	Função responsavel por recuperar  buffer da requisição HTTP, depois que foi feito o parser
**/
char* converte_Request_to_string(struct PedidoAnalisado *pedido);
/**
	Função responsavel por criar e fazer a conexão com socket para o servidor remoto
	que o host da requisição HTTP está hospedado
	Retorna o identificador do socket para o servidor remoto
**/
int createServerSocket(char *requestHost, char *requestPort);
/**
	Função responsavel por enviar a requisição HTTP do browser para o servidor remoto de destino
	Entradas:	bufferServer armazena a requisicao HTTP
				socketfd 	é o identificador do socket para o servidor remoto de destino
				sizeBuffer  é o tamanho do buffer
**/
void sendToServerSocket(const char* bufferServer,int socketfd,int sizeBuffer);
/**
	Função responsavel por enviar a resposta HTTP recebida do servidor remoto 
	para o browser
	Entradas:	bufferServer armazena a resposta HTTP
				socketfd 	é o identificador do socket para o browser
				sizeBuffer  é o tamanho do buffer
**/
void sendToClientSocket(const char* bufferServer,int socketfd,int sizeBuffer);
/**
	Função rsponsavel por receber a resposta HTTP do servidor remoto e armazenala
	em um buffer.
	Também é responsavel por chamar a função void sendToClientSocket(const char* bufferServer,int socketfd,int sizeBuffer);
	Entradas:
				Clientfd é o identificador do socket para o browser
				Serverfd é o identificador do socket pata o servidor remoto
**/
void receiveFromServer (int Clientfd, int Serverfd);

using namespace std;

int main (int argc, char *argv[]) 
{
	//descritores 
	int sockfd,newsockfd;
	//retornos
	int binded, sizeClient;
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
  	listen(sockfd, 5);  // aloca uma fila de tamanho 5 para conexões pendentes do browser

  	sizeClient = sizeof(struct sockaddr);

  	printf("Aguardando requisicoes ...\n");
  	while(1) {
  		
  		// bloqueia a execucao do programa, ate que exista um pedido de conexao por parte do cliente
  		newsockfd = accept(sockfd,&endereco_cliente, (socklen_t*) &sizeClient); 

  		if (newsockfd <0){
  			printf("Erro ao aceitar pedido de conexao!\n");
 		}
 		getRequestHTTP((void*)&newsockfd);
 	}
 	close(newsockfd);
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
		fprintf(stderr," Erro na alocacao do cabecalhoBuffer! \n");
		exit (1);
	}

	recupera_cabecalho_PedidoHTTP(pedido, cabecalhoBuffer, sizeCabecalho);
	cabecalhoBuffer[sizeCabecalho] = '\0';

	sizeRequest = strlen(pedido->method) + strlen(pedido->path) + strlen(pedido->version) + sizeCabecalho + 4;
	serverRequest = (char *) malloc(sizeRequest + 1);

	if(serverRequest == NULL){
		fprintf(stderr," Erro na alocacao do serverRequest!\n");
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
int createServerSocket(char *requestHost, char *requestPort) {
  
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  int idSocket;
  //Obtem informações de endereço para o soquete de fluxo na porta de entrada
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(requestHost, requestPort, &host_info, &host_info_list) != 0) {
   		fprintf(stderr," Erro no formato do endereco do servidor!\n");
		exit (1);
  }
  //cria um socket
  if ((idSocket = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol)) < 0) 
  {
    	fprintf(stderr," Erro ao criar socket para o servidor!\n");
		exit (1);
  }
  //faz a conecção
  if (connect(idSocket, host_info_list->ai_addr, host_info_list->ai_addrlen) < 0)
  {
    	fprintf(stderr," Erro ao tentar conectar o servidor!\n");
		exit (1);
  }
  freeaddrinfo(host_info_list);
  return idSocket;
}
void sendToServerSocket(const char* bufferServer,int socketfd,int sizeBuffer)
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
void sendToClientSocket(const char* bufferServer,int socketfd,int sizeBuffer)
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
void receiveFromServer (int Clientfd, int Serverfd) 
{
	int sizeBuffer = 5000;
	int iRecv;
	char buffer[sizeBuffer];
	/**
		para receber a resposta HTTP do servidor remoto eu 
		crio um laço while que verifica se a quantidade de 
		bytes recebidos é maior que zero
			caso em que iRecv > 0, armazeno o que eu recebi em um buffer
								   e repasso imediatamente para o browser
			caso em que o iRecv == 0, significa que o servidor remoto
									  terminou de enviar toda a resposta HTTP
			caso em que o iRecv < 0, significa um erro e o programa é encerrado
	**/
	while ((iRecv = recv(Serverfd, buffer, sizeBuffer, 0)) > 0) {
	    sendToClientSocket(buffer, Clientfd,iRecv);         // writing to client	  
		memset(buffer,0,sizeof (buffer));	
	}      
	if (iRecv < 0) {
	  fprintf(stderr,"Erro enquanto recebia do servidor!\n");
	  exit (1);
	}
}
void* getRequestHTTP(void* socketid)
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
		fprintf(stderr,"Erro durante a alocacao de memoria.\n");
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
	if(strlen(mensagem) > 0)
	{
		char what[5000];
		struct PedidoAnalisado *pedido;    // contem o pedido analisado
		printf("----------------------------------------------------\n");
		printf("Resquisicao HTTP do browser:\n");
		printf("%s\n", mensagem);
		printf("1 - Spider \n2 - Cliente Recursivo\n3 - Apenas responder o brownser\nDigite a opcao >> ");
		scanf("%d", &opcao);
		switch(opcao)
		{
			case 1:
				pedido = PedidoAnalisado_create();
				Analise_do_pedido(pedido, mensagem, strlen(mensagem));
			    memset(what,'\0',5000);
			    strcpy(what,"./spider ");
			    strcat(what,pedido->host);
			    system(what);
				break;
			case 2:
				pedido = PedidoAnalisado_create();
				Analise_do_pedido(pedido, mensagem, strlen(mensagem));
			    memset(what,'\0',5000);
			    strcpy(what,"./my_wget ");
			    strcat(what,pedido->host);
			    system(what);
				break;
			default:

				pedido = PedidoAnalisado_create();

				Analise_do_pedido(pedido, mensagem, strlen(mensagem));
				//Se a porta não foi setada na mensagem URL, coloquei como padrao a porta 80
				if (pedido->port == NULL) pedido->port = (char *) "80";
				
				int pid = fork();

		 		if(pid == 0)	//processo filho
		 		{
		 			browser_request  = converte_Request_to_string(pedido);		
					iServerfd = createServerSocket(pedido->host, pedido->port);
					sendToServerSocket(browser_request, iServerfd, total_de_bits_recebidos);
					receiveFromServer(newsockfd, iServerfd);
					PedidoAnalisado_destroy(pedido);	
					close(newsockfd);   
					close(iServerfd);
					_exit(0);
		 		}
				break;
		}
	}
	int y = 3;
	int *p = &y;
	return p;
}