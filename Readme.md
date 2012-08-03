##bipolarbar
###Aimed at, but not limited to, monsterwm users.

The bar can show two sets of information with different timings. e.g.

Pipe monsterwm into bipolarbar to have it's output shown on the left.

Set the root windows' name with conky or a script and it will be
 shown on the rest of the bar.

e.g. conky | while read -r; do xsetroot -name "$REPLY"; done &

####The ten colours are set at compile time so edit them to suit.
####All ten colours can be used for the background or text.

The first colour (colour0) is the default background and the second colour
is the default for the text, for both the piped in text and the text from xsetroot -name.

To change the text colour mark the text with an ampersand and the desired colour number.

*e.g.* To use the second and last colours, mark the text like -

** *&1* some text *&9* some more text**

To change the background colour mark the text with an ampersand and a capital B and the colour number.

*e.g.* To use the second and last colours, and change the background mark the text like -

** *&1* some text *&B2&9* some more text**

The text defaults to the right of the piped in text (monsterwm's text)
but can be set centered or on the right with &C and &R, which can't be
preceeded by a background or text colour.

*e.g.* To have some text next to the piped in text and some on the right

** *&2* some text *&R&B3&9* some right text**

Whether the bar is at *top or bottom* is set at compile time.

**#define TOP_BAR *0*        // 0=Bar at top, 1=Bar at bottom**

The height of the bar can be set at compile time.

**#define BAR_HEIGHT *16*    //**

The width of the bar can be set at compile time. 0 for fullscreen width or 
the number of pixels

**#define BAR_WIDTH *0*    //**

Fonts used are defined in a comma seperated list.
Font is set at compile time and the height of the bar is relevant to the font height 
if the bar height is too small.

*The window manager might need it's space for a bar adjusted.*

The bar has the override redirect flag set to true so it can be rebuilt and started again without a 'good' window manager trying to map it.
