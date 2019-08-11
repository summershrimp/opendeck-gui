#include "lvgl/lvgl.h"
#include "opendeck_app.h"
#include "opendeck_hid.h"
#include "protocol.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void lv_btn_group_create(lv_obj_t*);

static void btn_event_cb(lv_obj_t * imgbtn, lv_event_t event);
static void hid_task(lv_task_t * task);
int hid_read_handler(uint8_t *data, int size);

static lv_img_dsc_t glassKey;
static lv_img_dsc_t icon_tmp;
static lv_img_dsc_t g_img_dscs[15][2];
static in_report key_report = {0};

unsigned char icon_datas[15][2][120*120*4] = {0};

static lv_obj_t *imgbtn[3][5];
static lv_obj_t *img[3][5];

void create_app(void){
    hid_open(hid_read_handler);

    static lv_style_t bg_style;
    lv_style_copy(&bg_style, &lv_style_plain);
    bg_style.body.main_color = LV_COLOR_MAKE(0x40, 0x40, 0x40);
    bg_style.body.grad_color = LV_COLOR_MAKE(0x40, 0x40, 0x40);
    lv_obj_set_style(lv_scr_act(), &bg_style);
    int i;
    int status = png_loader("glassKey_small.png", &glassKey);
    if(! status){
        printf("glassKey load failed\n");
    }
    status = png_loader("action.png", &icon_tmp);
    if(! status){
        printf("action load failed\n");
    }
    assert(icon_tmp.data_size == 120*120*4);

    for(i=0; i<15; ++i) {
        memcpy(&g_img_dscs[i][0], &icon_tmp, sizeof(icon_tmp));
        memcpy(&g_img_dscs[i][1], &icon_tmp, sizeof(icon_tmp));
        memcpy(icon_datas[i][0], icon_tmp.data, icon_tmp.data_size);
        memcpy(icon_datas[i][1], icon_tmp.data, icon_tmp.data_size);
        
        g_img_dscs[i][0].data = icon_datas[i][0];
        g_img_dscs[i][1].data = icon_datas[i][1];

    }
    free((char *)icon_tmp.data);
    icon_tmp.data = NULL;
    printf("ok read img\n");

    lv_btn_group_create(lv_scr_act());
    printf ("ok create btn\n");
    lv_task_create(hid_task, 10, LV_TASK_PRIO_HIGH, NULL);
    printf ("ok create task\n");
}

static void hid_task(lv_task_t * task){
    hid_poll();
    hid_send_report((uint8_t *)&key_report, sizeof(key_report));
}

static unsigned char *g_recv_data = NULL;
unsigned short g_recv_cur = 0;

int create_g_recv(int size) {
    if(g_recv_data) {
        free(g_recv_data);
    }
    g_recv_cur = 0;
    g_recv_data = malloc(size);
    return 0;
}

int free_g_recv() {
    if(g_recv_data) {
        free(g_recv_data);
        g_recv_cur = 0;
        g_recv_data = NULL;
    }
    return 0;
}

int copy_to_g_recv(uint8_t *data, int size) {
    out_report *p = (out_report *) data;
    if(p->length + OUT_REPORT_HEADER_LEN > size ) {
        printf("cur length exceed\n");
        return -1;
    }
    if(g_recv_cur >= p->total_length) {
        printf("total length exceed\n");
        return -1;
    }
    memcpy(g_recv_data+g_recv_cur, p->data, p->length);
    g_recv_cur += p->length;
    return 0;
}

int deal_data(unsigned char * data, int size, int key, int msg_type);

int hid_read_handler(uint8_t *data, int size){
    static out_report last;
    static int has_last = 0;
    int status;
    out_report *p = (out_report *) data;
    //printf("hid received: %d type: %d, key_id: %d, has more: %d, total_size: %x, cur_size: %x \n", 
    //    size, p->type, p->key_id, p->has_more, p->total_length, p->length);
    if(has_last) {
        if(p->total_length != last.total_length || p->key_id != last.key_id ||
        p->type != last.type ) {
            printf("bad future data.\n");
            has_last = 0;
            free_g_recv();
            return 0;
        }
    } else {
        create_g_recv(p->total_length);
    }

    status = copy_to_g_recv(data, size);
    if(status == -1) {
        printf("copy_to failed\n");
        has_last = 0;
        free_g_recv();
    }
    if(p->has_more) {
        has_last = 1;
        memcpy(&last, p, OUT_REPORT_HEADER_LEN);
    } else {
        //printf("total_len: %x, recv_cur: %x\n", p->total_length, g_recv_cur);
        deal_data(g_recv_data, p->total_length, p->key_id, p->type);
        free_g_recv();
        memset(&last, 0, OUT_REPORT_HEADER_LEN);
        has_last = 0;
    }
    return 0;
}


int deal_data(unsigned char * data, int size, int key, int msg_type) {
    int status;
    if(key >= 15) {
        printf("key from hid too large: %d.\n", key);
        return -1;
    }
    int j = key/5, i = key % 5;
    if(msg_type == KEY_NORM_IMG || msg_type == KEY_TOGG_IMG) {
        status = png_data_loader(data, size, &icon_tmp);
        if(! status){
            printf("load png data from hid failed\n");
            return -1;
        }
        if(icon_tmp.data_size != 120*120*4) {
            printf("png data from hid size incorrect \n");
            return -1;
        }
        memcpy(icon_datas[key][msg_type], icon_tmp.data, icon_tmp.data_size);
        lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_REL, &g_img_dscs[key][0]);
        lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_PR, &g_img_dscs[key][0]);
        if(key == KEY_NORM_IMG) {
            memcpy(icon_datas[key][KEY_TOGG_IMG], icon_tmp.data, icon_tmp.data_size);
            lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_TGL_REL, &g_img_dscs[key][KEY_TOGG_IMG]);
            lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_TGL_PR, &g_img_dscs[key][KEY_TOGG_IMG]);
        }
        free((char *)icon_tmp.data);
        //printf("update key %d succes.\n", key);
    }                                     
    return 0;
}


void lv_btn_group_create(lv_obj_t *parent)
{
    static lv_style_t style_pr;
    lv_style_copy(&style_pr, &lv_style_plain);
    style_pr.image.color = LV_COLOR_BLACK;
    style_pr.image.intense = LV_OPA_50;
    style_pr.text.color = lv_color_hex3(0xaaa);

    int i, j, id;
    /*Create an Image button*/
    for(j=0;j<3;++j) {
        for(i=0; i< 5; ++i) {
            id = 5*j + i;
            imgbtn[j][i] = lv_imgbtn_create(parent, NULL);
            img[j][i] = lv_img_create(imgbtn[j][i], NULL);
            lv_img_set_src(img[j][i], &glassKey);
            lv_imgbtn_set_toggle(imgbtn[j][i], true);
            lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_REL, &g_img_dscs[id][0]);
            lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_PR, &g_img_dscs[id][0]);
            lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_TGL_REL, &g_img_dscs[id][1]);
            lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_TGL_PR, &g_img_dscs[id][1]);
            lv_imgbtn_set_style(imgbtn[j][i], LV_BTN_STATE_PR, &style_pr);        /*Use the darker style in the pressed state*/
            lv_imgbtn_set_style(imgbtn[j][i], LV_BTN_STATE_TGL_PR, &style_pr);

            imgbtn[j][i]->user_data = (void*) (id);
            if(i==0 && j==0) {
                lv_obj_align(imgbtn[0][0], NULL, LV_ALIGN_IN_TOP_LEFT, 36, 30);
            } else if (j == 0) {
                lv_obj_align(imgbtn[0][i], imgbtn[0][i-1], LV_ALIGN_OUT_RIGHT_TOP, 32, 0);
            } else {
                lv_obj_align(imgbtn[j][i], imgbtn[j-1][i], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);
            }
            lv_obj_align(img[j][i], NULL, LV_ALIGN_CENTER, 0, 0);
            lv_obj_move_background(img[j][i]);
            lv_obj_set_click(img[j][i], false);  
            lv_obj_set_event_cb(imgbtn[j][i], btn_event_cb);
        }
    }
}


/**********************
 *   STATIC FUNCTIONS
 **********************/



static void btn_event_cb(lv_obj_t * btn, lv_event_t event)
{   
    int key_id = (int) btn->user_data;
    if(event == LV_EVENT_RELEASED) {
        lv_imgbtn_toggle(btn);
        lv_btn_state_t state = lv_btn_get_state(btn);
        switch(state) {
            case LV_BTN_STATE_TGL_PR:
            case LV_BTN_STATE_TGL_REL:
                state = 1; break;
            case LV_BTN_STATE_PR:
            case LV_BTN_STATE_REL:
                state = 0; break;
        }
        key_report.key_status[key_id] = 0;
        fprintf(stderr, "Clicked %d, toggle: %d\n", key_id, state);
    } else if(event == LV_EVENT_PRESSED){
        key_report.key_status[key_id] = 1;
    }

}