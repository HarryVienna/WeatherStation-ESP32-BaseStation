#ifndef GUI_H
#define GUI_H

#include "lvgl.h"
#include "../ui/ui.h"
#include "../display/esp32_s3.h"


void disp_disable_scanbutton(bool is_disabled);
void disp_wifi_networks(char* allNetworks);

//void event_screen_init(lv_event_t *e);



#endif /* GUI_H */