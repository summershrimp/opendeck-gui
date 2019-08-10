#ifndef _OPENDECK_APP_H_
#define _OPENDECK_APP_H_

void create_app(void);
int png_loader(const char *file, lv_img_dsc_t *out_png_dsc);
int png_data_loader(const uint8_t *png_data, size_t png_data_size, lv_img_dsc_t *out_png_dsc);
#endif
