#include "types.h"
#include "user.h"

int main()
{
    char *buf;
    buf = (char*)malloc(10240);
    wolfie(buf, 10240);
    
    free(buf);
    exit();
}