#ifndef NCURSESW_FUNC_H_
#define NCURSESW_FUNC_H_

extern int border_set(const void *ls, const void *rs, const void *ts,
                      const void *bs, const void *tl, const void *tr,
                      const void *bl, const void *br);
extern int mvaddwstr(int y, int x, const wchar_t *wstr);
extern int mvaddnwstr(int y, int x, const wchar_t *wstr, int n);
extern int addwstr(const wchar_t *wstr);
extern int get_wch(wint_t *wch);

#endif // NCURSESW_FUNC_H_