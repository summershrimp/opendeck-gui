CSRCS += opendeck_app.c lodepng.c png_loader.c opendeck_hid.c

DEPPATH += --dep-path $(LVGL_DIR)/lv_app
VPATH += :$(LVGL_DIR)/lv_app

CFLAGS += "-I$(LVGL_DIR)/lv_app"