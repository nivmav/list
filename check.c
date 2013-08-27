#include <stdio.h>
#include<stdlib.h>



int main()
{

    char niv[5] = "22.01";
    char new[5] = "22.1";
    int value1 = 0, value2 = 0;


    sscanf(niv, "%d.%d", &value1, &value2);
    printf("%d,%d\n", value1, value2);
    sscanf(new, "%d.%02d", &value1, &value2);
    printf("%d,%d\n", value1, value2);

    return;
}


