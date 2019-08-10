#ifndef PROTOCOL_H_
#define PROTOCOL_H_


#define KEY_COUNT 15

typedef struct type_in_report {
    unsigned char key_status[KEY_COUNT];
    unsigned char fill[64-KEY_COUNT];
} in_report;

typedef struct type_out_report {

} out_report;


#endif