#include <stdint.h>

typedef int (*hid_data_cb)(uint8_t *data, int size);

int hid_open(hid_data_cb cb);
int hid_poll();
int hid_send_report(uint8_t *data, int size);