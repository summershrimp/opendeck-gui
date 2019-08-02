#include "lodepng.h"
#include <stdint.h>
#include <stdlib.h>
#include "lvgl/lvgl.h"

int png_loader(const char *file, lv_img_dsc_t *out_png_dsc) {
    uint32_t error;                 /*For the return values of png decoder functions*/

    /*Load the PNG file into buffer. It's still compressed (not decoded)*/
    unsigned char * png_data;      /*Pointer to the loaded data. Same as the original file just loaded into the RAM*/
    size_t png_data_size;          /*Size of `png_data` in bytes*/
    /*Decode the PNG image*/
    unsigned char * png_decoded;    /*Will be pointer to the decoded image*/
    uint32_t png_width;             /*Will be the width of the decoded image*/
    uint32_t png_height;            /*Will be the width of the decoded image*/

    error = lodepng_load_file(&png_data, &png_data_size, file);   /*Load the file*/
    if(error) {
        return false;
    }

    /*Decode the loaded image in ARGB8888 */
    error = lodepng_decode32(&png_decoded, &png_width, &png_height, png_data, png_data_size);   
    if(error) {
        return false;
    }

    free(png_data);

    out_png_dsc->header.always_zero = 0;                          /*It must be zero*/
    out_png_dsc->header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;      /*Set the color format*/
    out_png_dsc->header.w = png_width;
    out_png_dsc->header.h = png_height;
    out_png_dsc->data_size = png_width * png_height * 4;
    out_png_dsc->data = png_decoded;

    return true;
}