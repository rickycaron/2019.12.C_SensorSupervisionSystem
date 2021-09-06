#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include "config.h"
#include "tcpsock.h"

#define PORT 5678
#define MAX_CONN 3  // state the max. number of connections the server will handle before exiting
//Bengong writes it myself to get familiar with the code

/* Implements a sequential test server (only one connection at the same time)
 */

int main( void )
{
  tcpsock_t* server;
  tcpsock_t* client;
  sensor_data_t data;
  int bufsize,result;
  int conn=0;
  printf("Test sever is started!\n");
  
  if(tcp_passive_open(&server, PORT)!=TCP_NO_ERROR){
    exit(EXIT_FAILURE);
  }

do{
  if(tcp_wait_for_connection(server,&client)!=TCP_NO_ERROR){
  	exit(EXIT_FAILURE);
  }
  // the connection was established successfully
  printf("Incoming client!");
  conn++;

do{
   bufsize=sizeof(data.id);
  result=tcp_receive(client,(void*)&data.id,&bufsize);
   
    bufsize=sizeof(data.value);
  result=tcp_receive(client,(void*)&data.value,&bufsize);
    
    bufsize=sizeof(data.ts);
  result=tcp_receive(client,(void*)&data.ts,&bufsize);
  if(result==TCP_NO_ERROR&&bufsize)
  	printf("sensor id=%"PRIu16 " - temperature=%g - timestamp= %ld\n",data.id,data.value,data.ts);
  
   }while(result==TCP_NO_ERROR);

  if(result==TCP_CONNECTION_CLOSED){
  	printf("Peer has closed connection! \n");
  }else{
  	printf("Error has occured to the peer! \n");
  	tcp_close(&client);
  }}while(conn<MAX_CONN);

if(tcp_close(&server)!=TCP_NO_ERROR)
{
	exit(EXIT_FAILURE);
}
printf("Tester server is shutting down!\n");

  return 0;

}




