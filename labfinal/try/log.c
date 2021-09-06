#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <time.h>
#include "errmacros.h"

//#include "connmgr.h"
//#include "sensor_db.h"

#define FIFO_NAME   "logFifo" 
//#define LOG_FILE_NAME "gateway.log"
#define MAX     120
static int print_sequency_number=0;
void print_to_file(FILE * fp,char * print_buf);

int main(int argc, char const *argv[])
{
  FILE * fp; 
  FILE * fplog;
  int result;//an interger for error handling
  print_sequency_number=0;
  char *str_result;
  char recv_buf[MAX]; 
  printf("Now the log process begins!\n");

  pid_t my_pid, parent_pid; 
  my_pid = getpid();
  parent_pid = getppid();

  fp = fopen(FIFO_NAME, "r"); 
  printf("synchronizing with main process ok\n");
  FILE_OPEN_ERROR(fp);
  fplog=fopen("gateway.log","w");
  FILE_OPEN_ERROR(fplog);
  printf("We get here1\n");

  printf("The first print finished!\n");
  printf("Now the log process is waiting for main process to end!\n");
  raise(SIGSTOP);
   do 
  {
    str_result = fgets(recv_buf, MAX, fp);
    if ( str_result != NULL )
    { 
      printf("Message received in log process: %s\n", recv_buf);
      print_to_file(fplog,recv_buf);  

      // print_sequency_number++;
      // fprintf(fplog, "%d ",print_sequency_number);
      // fprintf(fplog, "%s\n",recv_buf);
    }
  }while( str_result != NULL ); 
  printf("Now Log process goes out !!!!!!!!!!!!!!!!!!!!!!!\n");

  result = fclose( fp );
  FILE_CLOSE_ERROR(result);
  result = fclose( fplog );
  FILE_CLOSE_ERROR(result);
  result=remove(FIFO_NAME);
  REMOVE_ERROR(result);

  exit(EXIT_SUCCESS);
}


/*this function print the char* to the fp with
the formet <sequence number><timestamp><log message>
*/
void print_to_file(FILE * fp, char* print_buf){
  if(fp==NULL){
    perror("fplog is NULL");
    exit(EXIT_FAILURE);
  }
  print_sequency_number++;

  char sequence_number[10];
  sprintf(sequence_number,"%d ",print_sequency_number);

  char  message_to_print[120];
  strcpy(message_to_print,sequence_number);
  strcat(message_to_print,print_buf);
  fprintf(fp, "%s\n",message_to_print);
}