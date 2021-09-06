
#ifndef __CONNMGR_H__
#define __CONNMGR_H__
#include <time.h>
#include "lib/dplist.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"


typedef struct socket_sensor socket_sensor_t;

/*This methods hold the core fuctionality of your connmgr. It starts listening on the given port and 
* when a sensor node connects, it writes the data to a sensor_data_recv file. This file must have the 
* same formet as  the sensor data file in assignment 6 and 7
*/
/*
 * This method starts listening on the given port and when when a sensor node connects it 
 * stores the sensor data in the shared buffer.
 */

void connmgr_listen(int port_number, sbuffer_t ** buffer);

/*This method shoud be called to clean up the connmgr, and to free all used memory.
* After this no new connection will be accepted
*/
void connmgr_free();

// void print_help(void);

// get the refrence to the tcp_socket in the struct
tcpsock_t* get_socket(socket_sensor_t *s);

tcpsock_t* dpl_get_tcpsock(dplist_t * list,int index);
// int dpl_get_sd(dplist_t * list,int index);
time_t dpl_get_ts(dplist_t * list,int index);

int add_new_client_to_dpl(dplist_t *list,tcpsock_t * sock);
void updatetime(dplist_t *list, int index);
//delete everything of a dplist_node inthe dplist
dplist_t *remove_sensor_node(dplist_t *list, int index);
dplist_t * checkallclients(dplist_t *list);

socket_sensor_t * dpl_get_socket_sensor(dplist_t * list,int index);
// /*This will print the log message to the log process when the first data of this socket is read*/
// int print_for_new_connection(socket_sensor_t * socket_sensor,sensor_id_t id);
// /*This function will print log message when the socket is time out and removed*/
// void print_for_removing_connection(socket_sensor_t * socket_sensor);
#endif  //__TCPSOCK_H__
