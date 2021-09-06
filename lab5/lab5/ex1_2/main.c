#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

dplist_t * list = NULL;

void dpl_print( dplist_t * list );

int main(void)
{
  list = dpl_create();
  
  dpl_insert_at_index(list,1,0);
  dpl_insert_at_index(list,2,1);
  dpl_insert_at_index(list,4,5);
  
  printf("list size = %d\n", dpl_size(list) );
  dpl_insert_at_index(list,3,2);
  dpl_print( list );
  printf("element '4' has index = %d\n", dpl_get_index_of_element(list, 4) );
  list = dpl_remove_at_index(list,-5);
  list = dpl_remove_at_index(list, 20);
  dpl_print(list);
  dpl_free(&list);
  dpl_print(list);
  return 0;
}



void dpl_print( dplist_t * list )
{
  element_t element;
  int i, length;

  length = dpl_size(list);
  for ( i = 0; i < length; i++)    
  {
    element = dpl_get_element_at_index( list, i );
    printf("index element %d = %d\n", i, element);
  }
}
