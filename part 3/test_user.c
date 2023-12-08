#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define PAGE_SIZE 4096
#define DATA_SIZE 12

int main(int argc , char *argv[])
{
    int fd = -1;
    unsigned char *p_map = NULL;
    /* open proc file */
    if((fd = open("/proc/mydir/myinfo", O_RDWR)) < 0) 
    {
        printf("open fail\n");
        exit(1);
    }
    
    printf("open successfully\n");
    
    if ((p_map = mmap(NULL, DATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) 
    {
        perror("Error mapping memory");
        close(fd);
        return EXIT_FAILURE;
    }
    
    for (int i = 0; i < DATA_SIZE; i++)
    {
        printf("%d\n", p_map[i]);
    }

    if (munmap(p_map, DATA_SIZE) == -1) 
    {
        perror("Error unmapping memory");
    }

    close(fd);

    return 0;
}