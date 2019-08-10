#include "lvgl/lvgl.h"
#include "opendeck_app.h"
#include "opendeck_hid.h"
#include "protocol.h"

#include <stdio.h>
#include <unistd.h>

void lv_btn_group_create(lv_obj_t*, lv_img_dsc_t *imgs[2]);

static void btn_event_cb(lv_obj_t * imgbtn, lv_event_t event);
static void hid_task(lv_task_t * task);
int hid_read_handler(uint8_t *data, int size);

static lv_img_dsc_t glassKey;
static lv_img_dsc_t icon[2];

static in_report key_report = {0};

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
    status = png_loader("action.png", &icon[0]);
    if(! status){
        printf("action load failed\n");
    }
    memcpy(&icon[1], &icon[0], sizeof(icon[0]));
    lv_img_dsc_t *imgs[15]; 
    for(i=0; i<15; ++i) {
        imgs[i] = icon; 
    }
    printf("ok read img\n");

    lv_btn_group_create(lv_scr_act(), imgs);
    printf ("ok create btn\n");
    lv_task_create(hid_task, 10, LV_TASK_PRIO_HIGH, NULL);
    printf ("ok create task\n");
}

static void hid_task(lv_task_t * task){
    hid_poll();
    hid_send_report((uint8_t *)&key_report, sizeof(key_report));
}

int hid_read_handler(uint8_t *data, int size){
    fprintf(stderr, "Hid received %u data\n", size);
    return 0;
}

void lv_btn_group_create(lv_obj_t *parent, lv_img_dsc_t *img_dscs[2])
{
    static lv_style_t style_pr;
    lv_style_copy(&style_pr, &lv_style_plain);
    style_pr.image.color = LV_COLOR_BLACK;
    style_pr.image.intense = LV_OPA_50;
    style_pr.text.color = lv_color_hex3(0xaaa);

    int i, j, id;

    /*Create an Image button*/
    lv_obj_t *imgbtn[3][5];
    lv_obj_t *img[3][5];
    for(j=0;j<3;++j) {
        for(i=0; i< 5; ++i) {
            id = 5*j + i;
            imgbtn[j][i] = lv_imgbtn_create(parent, NULL);
            img[j][i] = lv_img_create(imgbtn[j][i], NULL);
            lv_img_set_src(img[j][i], &glassKey);
            lv_imgbtn_set_toggle(imgbtn[j][i], true);
            lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_REL, &img_dscs[id][0]);
            lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_PR, &img_dscs[id][0]);
            lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_TGL_REL, &img_dscs[id][1]);
            lv_imgbtn_set_src(imgbtn[j][i], LV_BTN_STATE_TGL_PR, &img_dscs[id][1]);
            lv_imgbtn_set_style(imgbtn[j][i], LV_BTN_STATE_PR, &style_pr);        /*Use the darker style in the pressed state*/
            lv_imgbtn_set_style(imgbtn[j][i], LV_BTN_STATE_TGL_PR, &style_pr);
            lv_imgbtn_set_toggle(imgbtn[j][i], false);
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



static void btn_event_cb(lv_obj_t * imgbtn, lv_event_t event)
{
    int key_id = (int) imgbtn->user_data;
    key_report.key_status[key_id] = event;
    
    if(event == LV_EVENT_RELEASED) {
        lv_imgbtn_toggle(imgbtn);
        lv_btn_state_t state = lv_btn_get_state(imgbtn);
        switch(state) {
            case LV_BTN_STATE_TGL_PR:
            case LV_BTN_STATE_TGL_REL:
                state = 1; break;
            case LV_BTN_STATE_PR:
            case LV_BTN_STATE_REL:
                state = 0; break;
        }
        fprintf(stderr, "Clicked %d, toggle: %d\n", key_id, state);
    }

}