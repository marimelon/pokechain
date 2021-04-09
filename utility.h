#ifndef UTILLITY_H_
#define UTILLITY_H_

#include <wchar.h>
#include <stdbool.h>

void draw_box(int y, int x, int h, int w);
void fill_blank(int y, int x, int h, int w);

void draw_lchat_box(int y, int x, const wchar_t *text, bool is_color);
void draw_rchat_box(int y, int x, const wchar_t *text, bool is_color);

#endif // UTILLITY_H_