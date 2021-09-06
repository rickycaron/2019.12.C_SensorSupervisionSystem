#ifndef _SBUFFER_H_
#define _SBUFFER_H_

#include "config.h"

#define SBUFFER_FAILURE -1     
#define SBUFFER_NO_DATA 1
#define SBUFFER_SUCCESS 0 
#define SBUFFER_DO_NOT_REMOVE_NOW 3
#define SBUFFER_HAS_READ_REMOVED 2 
#define SBUFFER_HAS_READ_NOT_REMOVED 5 
#define SBUFFER_DATAMGR_HAS_NOT_READ 6

typedef struct sbuffer sbuffer_t;

/*
 * All data that can be stored in the sbuffer should be encapsulated in a
 * structure, this structure can then also hold extra info needed for your implementation
 */
typedef struct sbuffer_data sbuffer_data_t;


/*
 * Allocates and initializes a new shared buffer
 * Returns SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
 */
int sbuffer_init(sbuffer_t ** buffer);


/*
 * All allocated resources are freed and cleaned up
 * Returns SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
 */
int sbuffer_free(sbuffer_t ** buffer);


/*
 * Removes the first data in 'buffer' (at the 'head') and returns this data as '*data'  
 * 'data' must point to allocated memory because this functions doesn't allocated memory
 * If 'buffer' is empty, the function doesn't block until new data becomes available but returns SBUFFER_NO_DATA
 * Returns SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
 */
//I don't need this function. Do not use it.
// int sbuffer_remove(sbuffer_t * buffer, sensor_data_t * data);


/* Inserts the data in 'data' at the end of 'buffer' (at the 'tail')
 * Returns SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
*/
int sbuffer_insert(sbuffer_t * buffer, sensor_data_t * data);


/*this function is used in datamgr to get the data out, set the datamgr_toread to 0,
remove it if both value is 0
*/
int sbuffer_remove_in_datamgr(sbuffer_t * buffer,sensor_data_t * data);

/*this function is used in databasemgr to get the data out, set the databasemgr_toread to 0,
remove it if both value is 0
*/
int sbuffer_remove_in_databasemgr(sbuffer_t * buffer,sensor_data_t * data);

/*This function prints out everything in the shared buffer
*/
void sbuffer_print(sbuffer_t ** buffer);

/*This function can be called in all .c file and main.c file, it send the message pointed by
send_buf to FIFO.
*/
void send_to_log(char * send_buf);
#endif  //_SBUFFER_H_

