#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <ctype.h>

#ifndef ANALYZER_WEB_PROXY
#define ANALYZER_WEB_PROXY

#define DEBUG 0

/**********************************************************************************
  A struct PedidoAnalisado serve para armazenar os principais campos do pedido HTTP
  necessarios para o funcionamento do web proxy
 **********************************************************************************/
struct PedidoAnalisado {
     char *method; 
     char *protocol; 
     char *host; 
     char *port; 
     char *path;
     char *version;
     char *buf;
     size_t buflen;
     struct CabecalhoDoPedido *headers;
     size_t headersused;
     size_t size_headers;
};

/**************************************************************************************
  Qualquer cabeçalho de um pedido HTTP possi um par de valor-chave no seguinte formato:
  "key:value\r\n que é mantido dentro da struct PedidoAnalisado
**************************************************************************************/
struct CabecalhoDoPedido {
     char * key;
     size_t sizeKey;
     char * value;
     size_t sizeValue;
};
/**************************************************************************************
  Cria um objeto vazio do tipo Pedido Analisado para ser usado na analise de de um 
  pedido HTTP
***************************************************************************************/
struct PedidoAnalisado* PedidoAnalisado_create();
/**************************************************************************************
  Analisa o buffer, que armazena a requisição HTTP
***************************************************************************************/
int Analise_do_pedido(struct PedidoAnalisado *p, const char *buffer,int size_buffer);

/* Destroi o objeto PedidoAnalisado */
void PedidoAnalisado_destroy(struct PedidoAnalisado *p);

/**************************************************************************************
  Recupera o buffer inteiro de um objeto do tipo  PedidoAnalisado. 
  buffer é alocado de tamanho de size_buffer, com espaço suficiente para escrever a 
  linha de solicitação, cabeçalhos e o \ r \ n à direita.
 **************************************************************************************/
int recuperaPedidoHTTP(struct PedidoAnalisado *p, char *buffer, size_t size_buffer);

/**************************************************************************************
  Recupera todo o buffer com a exceção da linha de solicitação de um objeto de pedido 
  analisado;
  buffer é alocado de tamanho de size_buffer, com espaço suficiente para gravar os 
  cabeçalhos e o \ r \ n final.
  Se não houver cabeçalhos, o rastreio \ r \ n não será analisado.
 **************************************************************************************/
int recupera_cabecalho_PedidoHTTP(struct PedidoAnalisado *p, char *buffer, size_t size_buffer);
/**************************************************************************************
  Retorna o comprimento total, incluindo linha de solicitação, cabeçalhos e o rastro\r\n
***************************************************************************************/
size_t PedidoAnalisado_sizeTotal(struct PedidoAnalisado *p);
/***************************************************************************************
  Retorna o comprimento incluindo cabeçalhos, se houver, e o rastreio \ r \ n,
   mas excluindo a linha de solicitação.
****************************************************************************************/
size_t CabecalhoDoPedido_size(struct PedidoAnalisado *p);

/* Set, get, e remove chaves de cabeçalho e valores terminados em nulo */
int CabecalhoDoPedido_set(struct PedidoAnalisado *p, const char * key, const char * value);
struct CabecalhoDoPedido* CabecalhoDoPedido_get(struct PedidoAnalisado *p, const char * key);
int CabecalhoDoPedido_remove (struct PedidoAnalisado *p, const char * key);

/* debug () imprime informações de depuração se DEBUG for definido como 1*/
void debug(const char * format, ...);

#endif