#ifndef PROTOCOL_H_
#define PROTOCOL_H_


#define KEY_COUNT 15

typedef struct type_in_report {
    unsigned char key_status[KEY_COUNT];
    unsigned char fill[64-KEY_COUNT];
} in_report;

typedef struct type_out_report {
    unsigned char type;
    unsigned char key_id;
    unsigned char has_more;
    unsigned char fill;
    unsigned short total_length;
    unsigned short length;
    unsigned char data[1];

} out_report;

#define OUT_REPORT_HEADER_LEN 8

enum msg_type {
    KEY_NORM_IMG = 0,
    KEY_TOGG_IMG,
    KEY_EXTRA_CONF
};

#endif