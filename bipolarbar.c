/* bipolarbar.c
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xlocale.h>

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

typedef struct {
    unsigned long color;
    GC gc;
} Theme;
static Theme theme[10];

typedef struct {
    XFontStruct *font;          /* font structure */
    XFontSet fontset;           /* fontset structure */
    int height;                 /* height of the font */
    int width;
    unsigned int fh;            /* Y coordinate to draw characters */
    unsigned int ascent;
    unsigned int descent;
} Iammanyfonts;

#include "config.h"

static void get_font();
static void update_right();
static void print_text();
static void print_right_text();
static int wc_size(char *string, unsigned int num);

static const char *defaultcolor[] = { colour0, colour1, colour2, colour3, colour4, colour5, colour6, colour7, colour8, colour9, };
static const char *font_list = FONT;

static unsigned int count, counted, j, k, m, bg;
static unsigned int text_length, text_start, new_length;
static unsigned int old_length;
static char output[256];
static char right[256];

static Display *dis;
static unsigned int sw, sh, height, width, screen;
static Window root, barwin;
static Drawable winbar;

static Iammanyfonts font;

void get_font() {
	char *def, **missing;
	int i, n;

	missing = NULL;
	if(strlen(font_list) > 0)
	    font.fontset = XCreateFontSet(dis, (char *)font_list, &missing, &n, &def);
	if(missing) {
		if(FONTS_ERROR < 1)
            while(n--)
                fprintf(stderr, ":: bipolarbar :: missing fontset: %s\n", missing[n]);
		XFreeStringList(missing);
	}
	if(font.fontset) {
		XFontStruct **xfonts;
		char **font_names;

		font.ascent = font.descent = 0;
		n = XFontsOfFontSet(font.fontset, &xfonts, &font_names);
		for(i = 0, font.ascent = 0, font.descent = 0; i < n; ++i) {
			if (font.ascent < (*xfonts)->ascent) font.ascent = (*xfonts)->ascent;
            if (font.descent < (*xfonts)->descent) font.descent = (*xfonts)->descent;
			++xfonts;
		}
		font.width = XmbTextEscapement(font.fontset, " ", 1);
	} else {
		fprintf(stderr, ":: bipolarbar :: Font '%s' Not Found\n:: bipolarbar :: Trying Font 'Fixed'\n", font_list);
		if(!(font.font = XLoadQueryFont(dis, font_list))
		&& !(font.font = XLoadQueryFont(dis, "fixed")))
			fprintf(stderr, ":: bipolarbar :: Error, cannot load font: '%s'\n", font_list);
		font.ascent = font.font->ascent;
		font.descent = font.font->descent;
		font.width = XTextWidth(font.font, " ", 1);
	}
	font.height = font.ascent + font.descent;
}

void update_output() {
    j=1; bg = count = k = 0;
    text_length = 0;
    unsigned int n;
    ssize_t num;
    char win_name[256];

    memset(output, '\0', 256);
    if(!(num = read(STDIN_FILENO, output, sizeof(output)))) {
        fprintf(stderr, "bipolarbar :: FAILED TO READ STDIN!!\n");
        strncpy(output, "FAILED TO READ STDIN!!", 24);
        exit(1);
    }

    text_length = strlen(output);
    for(n=0;n<=text_length;++n) {
        while(output[n] == '&') {
            if(output[n+1]-'0' < 10 && output[n+1]-'0' >= 0)
                n += 2;
            if(output[n+1] == 'B' && output[n+2]-'0' < 10 && output[n+2]-'0' >= 0)
                n += 3;
        }
        if(output[n] == '\n' || output[n] == '\r') break;
        win_name[count] = output[n];
        ++count;
    }
    new_length = wc_size(win_name, count);
    XFillRectangle(dis, winbar, theme[0].gc, 0, 0, old_length, height);
    for(count=0;count<=text_length;++count) {
        print_text();
    }
    text_length = (old_length > new_length) ? old_length : new_length;
    XCopyArea(dis, winbar, barwin, theme[1].gc, 0, 0, text_length, height, 0, 0);
    XSync(dis, False);
    old_length = new_length;
    return;
}

void update_right() {
    unsigned int text_l=0, p_length, q=0, n;
    char bstring[256];
    char *root_name;
    j=1;m=0; bg = 0;

    if(!(XFetchName(dis, root, &root_name))) {
        strcpy(right, "&1bipolarbar ");
        text_l = 14;
    } else {
        while(root_name[text_l] != '\0' && text_l < 256) {
            right[text_l] = root_name[text_l];
            ++text_l;
        }
        right[text_l] = '\0';
    }
    XFree(root_name);

    XFillRectangle(dis, winbar, theme[0].gc, old_length, 0, width-old_length, height);
    text_start = old_length+font.width;
    // i=pos on screen q=pos in text
    for(counted=0;counted<=text_l;++counted) {
        if(right[counted] == '&' && (right[counted+1] == 'C' || right[counted+1] == 'R')) {
            counted += 2; q=0; m=0;
            for(n=counted;n<text_l;++n) {
                while(right[n] == '&') {
                    if(right[n+1]-'0' < 10 && right[n+1]-'0' >= 0) {
                        n += 2;
                    } else if(right[n+1] == 'B' && right[n+2]-'0' < 10 && right[n+2]-'0' >= 0) {
                        n += 3;
                    } else if(right[n+1] == 'R') {
                        break;
                    } else break;
                }
                bstring[q] = right[n]; ++q;
            }
            bstring[q] = '\0';
            p_length = wc_size(bstring, q);
            if(right[counted-1] == 'C')
                text_start = (width/2)-(p_length/2)+old_length;
            if(right[counted-1] == 'R')
                text_start = width-p_length;
        }
        print_right_text();
    }

    XCopyArea(dis, winbar, barwin, theme[1].gc, old_length, 0, width-old_length, height, old_length, 0);
    memset(right, '\0', 256);
    XSync(dis, False);
    return;
}

void print_right_text() {
    char bstring[256];
    unsigned int wsize, n=0;

    while(right[counted] == '&') {
        if(right[counted+1] == 'C' || right[counted+1] == 'R') {
            counted--;
            break;
        }
        if(right[counted+1]-'0' < 10 && right[counted+1]-'0' >= 0) {
            j = right[counted+1]-'0';
            counted += 2;
        } else if(right[counted+1] == 'B' && right[counted+2]-'0' < 10 && right[counted+2]-'0' >= 0) {
            bg = right[counted+2]-'0';
            counted += 3;
        } else break;
    }
    if(right[counted] == '&') {
        bstring[n] = right[counted];
        ++n;++counted;
    }
    while(right[counted] != '&' && right[counted] != '\0' && right[counted] != '\n' && right[counted] != '\r') {
        bstring[n] = right[counted];
        ++n;++counted;
    }
    if(n < 1) return;
    bstring[n] = '\0';
    wsize = wc_size(bstring, n);
    XFillRectangle(dis, winbar, theme[bg].gc, text_start+m, 0, wsize, height);
    if(font.fontset)
        XmbDrawString(dis, winbar, font.fontset, theme[j].gc, text_start+m, font.fh, bstring, n);
    else
        XDrawString(dis, winbar, theme[j].gc, text_start+m, font.fh, bstring, n);
    m += wsize;
    memset(bstring, '\0', n);
    counted--;
}

void print_text() {
    char astring[256];
    unsigned int wsize, n=0;

    while(output[count] == '&') {
        if(output[count+1]-'0' < 10 && output[count+1]-'0' >= 0) {
            j = output[count+1]-'0';
            count += 2;
        } else if(output[count+1] == 'B' && output[count+2]-'0' < 10 && output[count+2]-'0' >= 0) {
            bg = output[count+2]-'0';
            count += 3;
        } else break;
    }
    if(output[count] == '&') {
        astring[n] = output[count];
        ++n;++count;
    }
    while(output[count] != '&' && output[count] != '\0' && output[count] != '\n' && output[count] != '\r') {
        astring[n] = output[count];
        ++n;++count;
    }
    if(n < 1) return;
    astring[n] = '\0';
    wsize = wc_size(astring, n);
    XFillRectangle(dis, winbar, theme[bg].gc, k, 0, wsize, height);
    if(font.fontset)
        XmbDrawString(dis, winbar, font.fontset, theme[j].gc, k, font.fh, astring, n);
    else
        XDrawString(dis, winbar, theme[j].gc, k, font.fh, astring, n);
    k += wsize;
    memset(astring, '\0', n);
    count--;
}

int wc_size(char *string, unsigned int num) {
    XRectangle rect;

    if(font.fontset) {
        XmbTextExtents(font.fontset, string, num, NULL, &rect);
        return rect.width;
    } else {
        return XTextWidth(font.font, string, num);
    }
}

unsigned long getcolor(const char* color) {
    XColor c;
    Colormap map = DefaultColormap(dis,screen);

    if(!XAllocNamedColor(dis,map,color,&c,&c)) {
        fprintf(stderr, "\033[0;31mbipolarbar :: Error parsing color!");
        return 1;
    }
    return c.pixel;
}

int main(int argc, char ** argv){
    unsigned int i, y = 0;
    XEvent ev;
    XSetWindowAttributes attr;
	char *loc;
	fd_set readfds;
    struct timeval tv;

    dis = XOpenDisplay(NULL);
    if (!dis) {fprintf(stderr, "bipolarbar :: unable to connect to display");return 7;}

    root = DefaultRootWindow(dis);
    screen = DefaultScreen(dis);
    sw = XDisplayWidth(dis,screen);
    sh = XDisplayHeight(dis,screen);
    loc = setlocale(LC_ALL, "");
    if (!loc || !strcmp(loc, "C") || !strcmp(loc, "POSIX") || !XSupportsLocale())
        fprintf(stderr, "bipolarbar :: LOCALE FAILED\n");
    get_font();
    height = (BAR_HEIGHT > font.height) ? BAR_HEIGHT : font.height+2;
    font.fh = ((height - font.height)/2) + font.ascent;
    width = (BAR_WIDTH == 0) ? sw : BAR_WIDTH;
    if (TOP_BAR != 0) y = sh - height;

    for(i=0;i<10;++i)
        theme[i].color = getcolor(defaultcolor[i]);
    XGCValues values;

    for(i=0;i<10;++i) {
        values.background = theme[0].color;
        values.foreground = theme[i].color;
        values.line_width = 2;
        values.line_style = LineSolid;
        if(font.fontset) {
            theme[i].gc = XCreateGC(dis, root, GCBackground|GCForeground|GCLineWidth|GCLineStyle,&values);
        } else {
            values.font = font.font->fid;
            theme[i].gc = XCreateGC(dis, root, GCBackground|GCForeground|GCLineWidth|GCLineStyle|GCFont,&values);
        }
    }
    old_length = 0;

    winbar = XCreatePixmap(dis, root, width, height, DefaultDepth(dis, screen));
    XFillRectangle(dis, winbar, theme[0].gc, 0, 0, width, height);
    barwin = XCreateSimpleWindow(dis, root, 0, y, width, height, 0, theme[0].color,theme[0].color);
    attr.override_redirect = True;
    XChangeWindowAttributes(dis, barwin, CWOverrideRedirect, &attr);
    XSelectInput(dis,barwin,ExposureMask);
    XSelectInput(dis,root,PropertyChangeMask);
    XMapWindow(dis, barwin);
    int x11_fd = ConnectionNumber(dis);
    while(1){
       	FD_ZERO(&readfds);
        FD_SET(x11_fd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 200000;
        select(x11_fd+1, &readfds, NULL, NULL, &tv);

    	if (FD_ISSET(STDIN_FILENO, &readfds))
    	    update_output();
        while(XPending(dis) != 0) {
            XNextEvent(dis, &ev);
            switch(ev.type){
                case Expose:
                    XCopyArea(dis, winbar, barwin, theme[1].gc, 0, 0, width, height, 0, 0);
                    XSync(dis, False);
                    break;
                case PropertyNotify:
                    if(ev.xproperty.window == root && ev.xproperty.atom == XA_WM_NAME)
                        update_right();
                    break;
            }
        }
    }

    return (0);
}
