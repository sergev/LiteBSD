#include <stdio.h>
#include <fcntl.h>
#include <sys/gpio.h>

int main()
{
    int fd;

    fd = open("/dev/gpio1", O_RDWR);
    if (fd < 0) {
        perror("Error in open gpio");
        return -1;
    }

    ioctl(fd, GPIO_CONFOUT);

    for (;;) {
        ioctl(fd, GPIO_SET);
        sleep(1);
        ioctl(fd, GPIO_CLEAR);
        sleep(1);
    }
    close(fd);
    return 0;
}
