#include <stdio.h>
#include <stdlib.h>
#include"dplist.h"

typedef struct 
{
    int data;
    double temp;
} element_t; // start from simple datatype

void* element_copy(void *src_element); // use same function signature as defined in dplist.h, but user should know what type this element is
void element_free(void **element);
int element_compare(void *x, void *y);
void element_print(void *element);

int main()
{
    dplist_t* list = NULL;
    list = dpl_create(&element_copy, &element_free, &element_compare);
    // printf("Testing insersion: \n");     // test passed
    // dpl_insert_at_index(list, &a, 0, 1);
    // dpl_print_list(list);
    // a = 2048;
    // dpl_insert_at_index(list, &a, 0, 1);
    // dpl_print_list(list);
    // a = 256;
    // dpl_insert_at_index(list, &a, 20, 1);   // something wrong with this insertion, FIXME: implement size of the list.  
    // dpl_print_list(list);

    printf("Empty list size:%d\n", dpl_size(list));

    element_t *elements[5];
    dplist_node_t *references[5];
    int i = 0;
    for(i = 0; i<5; i++){
        elements[i] = (element_t*)malloc(sizeof(element_t));
        elements[i]->data = i;
        elements[i]->temp = 10.0*i;
        dpl_insert_at_index(list, elements[i], 0, 1);
        references[i] = dpl_get_reference_at_index(list, 0);
    }
    // dpl_print_list(list);

    // dpl_insert_at_index(list, &a, 0, 1);
    // a = 512;
    // dpl_insert_at_index(list, &a, 0, 1);
    // a = 256;
    // dpl_insert_at_index(list, &a, 0, 1);
    // a = 128;
    // dpl_insert_at_index(list, &a, 0, 1);
    // a = 64;
    // dpl_insert_at_index(list, &a, 0, 1);
    // a = 32;
    // dpl_insert_at_index(list, &a, 0, 1);
    // a = 16;
    // dpl_insert_at_index(list, &a, 0, 1);
    // dpl_print_list(list);

    printf("testing get element at index:\n");  // test passed
    // for(i=0;i<5;i++){
    //     element_print(dpl_get_element_at_index(list, i));
    // }

    for(i = 0; i<5; i++){
        element_print((void*)dpl_get_element_at_reference(list, references[i]));
    }

    for(i = 0; i<5; i++){
        dpl_remove_at_index(list, 0, 1);
    }

    for(i = 0; i<5; i++){
        dpl_insert_at_index(list, elements[i], 0, 1);
    }
    for(i = 0; i<5; i++){
        // try search by reference again, this time should not be found
        element_print((void*)dpl_get_element_at_reference(list, references[i]));
    }
    
    

    // printf("Testing remove element in index:\n");   // test passed
    // dpl_remove_at_index(list, -1, 1);
    // for(i=0;i<5;i++){
    //     element_print(dpl_get_element_at_index(list, i));
    // }
    // // dpl_print_list(list);
    // dpl_remove_at_index(list, 20, 1);
    // // dpl_print_list(list);
    // dpl_remove_at_index(list, 5, 1);
    // // dpl_print_list(list);
    // dpl_remove_at_index(list, 0, 1);
    // // dpl_print_list(list);
    
    // printf("Testing delete list:\n");
    dpl_free(&list, 1);
    // free memory in the heap
    for(i=0;i<5;i++){
        free(elements[i]);
        
    }

    // // dpl_free(&list, 1);
    // list = dpl_create(&element_copy, &element_free, &element_compare);
    // for(i=0;i<5;i++){
    //     element_print(dpl_get_element_at_index(list, i));
    // }
    // dpl_print_list(list);
    
    printf("success!\n");
    
    return 0;
}

/* user implement copy, free and compare function */

/**
 * deep copy function, TEST PASSED
 * */
void* element_copy(void* src_element)
{
    element_t temp = *((element_t*)src_element);
    element_t* new_ele = (element_t*)malloc(sizeof(element_t));
    *new_ele = temp;
    return (void*)new_ele;
}

/**
 * free space
 * */
void element_free(void **element)
{
    element_t* temp = (element_t*)(*element);   // TODO: how to test this code?
    free(temp);     
}

/**
 * Compare two element elements; returns -1 if x<y, 0 if x==y, or 1 if x>y 
 * */
int element_compare(void *x, void *y)
{
    element_t* x1 = (element_t*)x;
    element_t* y1 = (element_t*)y;
    if((x1->data)<(y1->data)){
        return -1;
    }
    else if((x1->data)>(y1->data)){
        return 1;
    }
    return 0;
}

void element_print(void *element)
{
    if(element!=NULL){
        element_t* t = (element_t*)element;
        printf("data = %d, temperature = %lf\n ",t->data, t->temp);
    }
    
}