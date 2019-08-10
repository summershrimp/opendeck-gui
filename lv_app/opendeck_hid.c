#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <stdio.h>

#include "opendeck_hid.h"

static int usb_fd = -1;
static hid_data_cb hid_cb;
struct pollfd hid_pollfd;
uint8_t t_hid_report[8192];
int hid_open(hid_data_cb cb){
    usb_fd = open("/dev/hidg0", O_RDWR);
    if(usb_fd == -1) {
        perror("open(/dev/hidg0)");
        return -1;
    }
    hid_cb = cb;
    hid_pollfd.fd = usb_fd;
    hid_pollfd.events = POLLIN;
    return 0;
}

int hid_poll() {
    if(hid_cb == NULL || usb_fd == -1){
        return -1;
    }
    int stat = poll(&hid_pollfd, 1, 0);
    if(stat == -1) {
        perror("poll(hidg0)");
    }
    if(hid_pollfd.revents & POLLIN){
        int size = read(usb_fd, t_hid_report, 8192);
        hid_cb(t_hid_report, size);
    }
    return 0;
}

int hid_send_report(uint8_t *data, int size){
    if(usb_fd == -1){
        return -1;
    }
    int ok = write(usb_fd, data, size);

    if(ok == size) {
        return 0;
    }
    return -1;
}