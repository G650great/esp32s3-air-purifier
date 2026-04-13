#ifndef _OLED_H_
#define _OLED_H_

#include <stdint.h>

#define OLED_WIDTH  128
#define OLED_HEIGHT 64

void oled_init(void);
void oled_clear(void);
void oled_update(void);
void oled_off(void);
void oled_on(void);

void oled_draw_pixel(int x, int y);
void oled_draw_line(int x0, int y0, int x1, int y1);
void oled_draw_rect(int x, int y, int w, int h);
void oled_fill_rect(int x, int y, int w, int h);
void oled_draw_char(int x, int y, char c);
void oled_draw_string(int x, int y, const char *str);
void oled_draw_progress_bar(int x, int y, int w, int h, int percent);

void oled_set_font_small(void);
void oled_set_font_large(void);

#endif
