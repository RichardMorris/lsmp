1,$s/^$/ /
1,$s/"/\\"/g
1,$s/^.*$/	"&",/
1i\
#include <forms.h>\
char helptext[][80] = {
$a\
"\\0"};\
\
load_help(FL_OBJECT *browser)\
{\
	int i;\
	fl_clear_browser(browser);\
\
	for(i=0;helptext[i][0] != '\\0';++i)\
		fl_add_browser_line(browser,helptext[i]);\
};
