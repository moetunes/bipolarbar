##splitter_bar
####Aimed at, but not limited to, monsterwm users

Pipe monsterwm into splitter_bar to have it's output shown on the left.

Set the root windows' name with conky or a script and that will be shown on the right.

e.g. conky | while read -r; do xsetroot -name "$REPLY"; done &

###The nine colours are set at compile time so edit them to suit.

The first colour is the background and the second to ninth are for the text.

To change the colour mark the text with an ampersand and the desired colour number.

*e.g.* To use the second and last colours, mark the text like -

** *&2* some text *&9* some more text**

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
