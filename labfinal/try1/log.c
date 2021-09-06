#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include "errmacros.h"
#include "connmgr.h"
//#include "sensor_db.h"

#define FIFO_NAME   "logFifo" 
#define MAX     80

int main(int argc, char const *argv[])
{
  FILE *fp; //fp hasn been defined 
  int result;//an interger for error handling
  char *str_result;
  char recv_buf[MAX]; 
  printf("Now the log process begins!\n");
  fp = fopen(FIFO_NAME, "r"); 
  printf("synchronizing with main process ok\n");
  FILE_OPEN_ERROR(fp);
  FILE* fplog=fopen("gateway.log","w");
  FILE_OPEN_ERROR(fplog);

   do 
  {
    str_result = fgets(recv_buf, MAX, fp);
    if ( str_result != NULL )
    { 
      printf("Message received: %s\n", recv_buf);
      fprintf(fplog, "%s\n", recv_buf); 
    }
  } while ( str_result != NULL ); 



  //result = fclose( fp );
  //FILE_CLOSE_ERROR(result);
  result = fclose( fplog );
  FILE_CLOSE_ERROR(result);
  result=remove(FIFO_NAME);
  REMOVE_ERROR(result);

  exit(EXIT_SUCCESS);
}