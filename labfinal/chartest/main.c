#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>


int main(void) 
{
  time_t timr=time(NULL);
 char * stringtoprint="A sensor node with sensor id 15 has opened a new connection!\n";
 printf("%ld\n",timr );
 printf("%ld\n", sizeof(uint16_t));
 printf("%ld\n", sizeof(time_t));
 printf("%ld\n",strlen(stringtoprint));
 printf("%ld\n", sizeof(double));
 //char * s2="The sensor node with sensor id reports it's too cold (running avg temperature =)!\n";
 char * s2="1578001731 Received snesor data with invalid sensor node ID 12 !\n";
 char * s3="1578001731 The sensor node with sensor id 15 reports it's too hot (running avg temperature =20.058538)!\n";
char * s4="1578001731 Connection to SQL server established!\n";
 printf("%ld\n",strlen(s4));
}
