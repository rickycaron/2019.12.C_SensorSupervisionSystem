#include<time.h>
#include<stdio.h>
#include<stdint.h>
int main(){
uint16_t x=0;
printf("%d",x);
printf("The size of uint_16 is %ld \n",sizeof(uint16_t));
printf("The size of double  is %ld \n",sizeof(double));
printf("The size of time_t is %ld",sizeof(time_t));
return 0;
}
