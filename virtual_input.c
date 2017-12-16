#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/uinput.h>
#include "virtual_input.h"

/*
* virtual_device_config
*/
#ifndef strcasecmp
#define strcasecmp strcmp
#endif

#define LINE_LEN 1024

#define ID_NONE 0
#define ID_CODES 1

static int config_line;
static int config_parse_error;
const char *whitespace = " \t";

#define NAME_ELEMENT(element) [element] = #element

static const char * const keys[KEY_MAX + 1] = {
	[0 ... KEY_MAX] = NULL,
	NAME_ELEMENT(KEY_RESERVED),		NAME_ELEMENT(KEY_ESC),
	NAME_ELEMENT(KEY_1),			NAME_ELEMENT(KEY_2),
	NAME_ELEMENT(KEY_3),			NAME_ELEMENT(KEY_4),
	NAME_ELEMENT(KEY_5),			NAME_ELEMENT(KEY_6),
	NAME_ELEMENT(KEY_7),			NAME_ELEMENT(KEY_8),
	NAME_ELEMENT(KEY_9),			NAME_ELEMENT(KEY_0),
	NAME_ELEMENT(KEY_MINUS),		NAME_ELEMENT(KEY_EQUAL),
	NAME_ELEMENT(KEY_BACKSPACE),		NAME_ELEMENT(KEY_TAB),
	NAME_ELEMENT(KEY_Q),			NAME_ELEMENT(KEY_W),
	NAME_ELEMENT(KEY_E),			NAME_ELEMENT(KEY_R),
	NAME_ELEMENT(KEY_T),			NAME_ELEMENT(KEY_Y),
	NAME_ELEMENT(KEY_U),			NAME_ELEMENT(KEY_I),
	NAME_ELEMENT(KEY_O),			NAME_ELEMENT(KEY_P),
	NAME_ELEMENT(KEY_LEFTBRACE),		NAME_ELEMENT(KEY_RIGHTBRACE),
	NAME_ELEMENT(KEY_ENTER),		NAME_ELEMENT(KEY_LEFTCTRL),
	NAME_ELEMENT(KEY_A),			NAME_ELEMENT(KEY_S),
	NAME_ELEMENT(KEY_D),			NAME_ELEMENT(KEY_F),
	NAME_ELEMENT(KEY_G),			NAME_ELEMENT(KEY_H),
	NAME_ELEMENT(KEY_J),			NAME_ELEMENT(KEY_K),
	NAME_ELEMENT(KEY_L),			NAME_ELEMENT(KEY_SEMICOLON),
	NAME_ELEMENT(KEY_APOSTROPHE),		NAME_ELEMENT(KEY_GRAVE),
	NAME_ELEMENT(KEY_LEFTSHIFT),		NAME_ELEMENT(KEY_BACKSLASH),
	NAME_ELEMENT(KEY_Z),			NAME_ELEMENT(KEY_X),
	NAME_ELEMENT(KEY_C),			NAME_ELEMENT(KEY_V),
	NAME_ELEMENT(KEY_B),			NAME_ELEMENT(KEY_N),
	NAME_ELEMENT(KEY_M),			NAME_ELEMENT(KEY_COMMA),
	NAME_ELEMENT(KEY_DOT),			NAME_ELEMENT(KEY_SLASH),
	NAME_ELEMENT(KEY_RIGHTSHIFT),		NAME_ELEMENT(KEY_KPASTERISK),
	NAME_ELEMENT(KEY_LEFTALT),		NAME_ELEMENT(KEY_SPACE),
	NAME_ELEMENT(KEY_CAPSLOCK),		NAME_ELEMENT(KEY_F1),
	NAME_ELEMENT(KEY_F2),			NAME_ELEMENT(KEY_F3),
	NAME_ELEMENT(KEY_F4),			NAME_ELEMENT(KEY_F5),
	NAME_ELEMENT(KEY_F6),			NAME_ELEMENT(KEY_F7),
	NAME_ELEMENT(KEY_F8),			NAME_ELEMENT(KEY_F9),
	NAME_ELEMENT(KEY_F10),			NAME_ELEMENT(KEY_NUMLOCK),
	NAME_ELEMENT(KEY_SCROLLLOCK),		NAME_ELEMENT(KEY_KP7),
	NAME_ELEMENT(KEY_KP8),			NAME_ELEMENT(KEY_KP9),
	NAME_ELEMENT(KEY_KPMINUS),		NAME_ELEMENT(KEY_KP4),
	NAME_ELEMENT(KEY_KP5),			NAME_ELEMENT(KEY_KP6),
	NAME_ELEMENT(KEY_KPPLUS),		NAME_ELEMENT(KEY_KP1),
	NAME_ELEMENT(KEY_KP2),			NAME_ELEMENT(KEY_KP3),
	NAME_ELEMENT(KEY_KP0),			NAME_ELEMENT(KEY_KPDOT),
	NAME_ELEMENT(KEY_ZENKAKUHANKAKU), 	NAME_ELEMENT(KEY_102ND),
	NAME_ELEMENT(KEY_F11),			NAME_ELEMENT(KEY_F12),
	NAME_ELEMENT(KEY_RO),			NAME_ELEMENT(KEY_KATAKANA),
	NAME_ELEMENT(KEY_HIRAGANA),		NAME_ELEMENT(KEY_HENKAN),
	NAME_ELEMENT(KEY_KATAKANAHIRAGANA),	NAME_ELEMENT(KEY_MUHENKAN),
	NAME_ELEMENT(KEY_KPJPCOMMA),		NAME_ELEMENT(KEY_KPENTER),
	NAME_ELEMENT(KEY_RIGHTCTRL),		NAME_ELEMENT(KEY_KPSLASH),
	NAME_ELEMENT(KEY_SYSRQ),		NAME_ELEMENT(KEY_RIGHTALT),
	NAME_ELEMENT(KEY_LINEFEED),		NAME_ELEMENT(KEY_HOME),
	NAME_ELEMENT(KEY_UP),			NAME_ELEMENT(KEY_PAGEUP),
	NAME_ELEMENT(KEY_LEFT),			NAME_ELEMENT(KEY_RIGHT),
	NAME_ELEMENT(KEY_END),			NAME_ELEMENT(KEY_DOWN),
	NAME_ELEMENT(KEY_PAGEDOWN),		NAME_ELEMENT(KEY_INSERT),
	NAME_ELEMENT(KEY_DELETE),		NAME_ELEMENT(KEY_MACRO),
	NAME_ELEMENT(KEY_MUTE),			NAME_ELEMENT(KEY_VOLUMEDOWN),
	NAME_ELEMENT(KEY_VOLUMEUP),		NAME_ELEMENT(KEY_POWER),
	NAME_ELEMENT(KEY_KPEQUAL),		NAME_ELEMENT(KEY_KPPLUSMINUS),
	NAME_ELEMENT(KEY_PAUSE),		NAME_ELEMENT(KEY_KPCOMMA),
	NAME_ELEMENT(KEY_HANGUEL),		NAME_ELEMENT(KEY_HANJA),
	NAME_ELEMENT(KEY_YEN),			NAME_ELEMENT(KEY_LEFTMETA),
	NAME_ELEMENT(KEY_RIGHTMETA),		NAME_ELEMENT(KEY_COMPOSE),
	NAME_ELEMENT(KEY_STOP),			NAME_ELEMENT(KEY_AGAIN),
	NAME_ELEMENT(KEY_PROPS),		NAME_ELEMENT(KEY_UNDO),
	NAME_ELEMENT(KEY_FRONT),		NAME_ELEMENT(KEY_COPY),
	NAME_ELEMENT(KEY_OPEN),			NAME_ELEMENT(KEY_PASTE),
	NAME_ELEMENT(KEY_FIND),			NAME_ELEMENT(KEY_CUT),
	NAME_ELEMENT(KEY_HELP),			NAME_ELEMENT(KEY_MENU),
	NAME_ELEMENT(KEY_CALC),			NAME_ELEMENT(KEY_SETUP),
	NAME_ELEMENT(KEY_SLEEP),		NAME_ELEMENT(KEY_WAKEUP),
	NAME_ELEMENT(KEY_FILE),			NAME_ELEMENT(KEY_SENDFILE),
	NAME_ELEMENT(KEY_DELETEFILE),		NAME_ELEMENT(KEY_XFER),
	NAME_ELEMENT(KEY_PROG1),		NAME_ELEMENT(KEY_PROG2),
	NAME_ELEMENT(KEY_WWW),			NAME_ELEMENT(KEY_MSDOS),
	NAME_ELEMENT(KEY_COFFEE),		NAME_ELEMENT(KEY_DIRECTION),
	NAME_ELEMENT(KEY_CYCLEWINDOWS),		NAME_ELEMENT(KEY_MAIL),
	NAME_ELEMENT(KEY_BOOKMARKS),		NAME_ELEMENT(KEY_COMPUTER),
	NAME_ELEMENT(KEY_BACK),			NAME_ELEMENT(KEY_FORWARD),
	NAME_ELEMENT(KEY_CLOSECD),		NAME_ELEMENT(KEY_EJECTCD),
	NAME_ELEMENT(KEY_EJECTCLOSECD),		NAME_ELEMENT(KEY_NEXTSONG),
	NAME_ELEMENT(KEY_PLAYPAUSE),		NAME_ELEMENT(KEY_PREVIOUSSONG),
	NAME_ELEMENT(KEY_STOPCD),		NAME_ELEMENT(KEY_RECORD),
	NAME_ELEMENT(KEY_REWIND),		NAME_ELEMENT(KEY_PHONE),
	NAME_ELEMENT(KEY_ISO),			NAME_ELEMENT(KEY_CONFIG),
	NAME_ELEMENT(KEY_HOMEPAGE),		NAME_ELEMENT(KEY_REFRESH),
	NAME_ELEMENT(KEY_EXIT),			NAME_ELEMENT(KEY_MOVE),
	NAME_ELEMENT(KEY_EDIT),			NAME_ELEMENT(KEY_SCROLLUP),
	NAME_ELEMENT(KEY_SCROLLDOWN),		NAME_ELEMENT(KEY_KPLEFTPAREN),
	NAME_ELEMENT(KEY_KPRIGHTPAREN), 	NAME_ELEMENT(KEY_F13),
	NAME_ELEMENT(KEY_F14),			NAME_ELEMENT(KEY_F15),
	NAME_ELEMENT(KEY_F16),			NAME_ELEMENT(KEY_F17),
	NAME_ELEMENT(KEY_F18),			NAME_ELEMENT(KEY_F19),
	NAME_ELEMENT(KEY_F20),			NAME_ELEMENT(KEY_F21),
	NAME_ELEMENT(KEY_F22),			NAME_ELEMENT(KEY_F23),
	NAME_ELEMENT(KEY_F24),			NAME_ELEMENT(KEY_PLAYCD),
	NAME_ELEMENT(KEY_PAUSECD),		NAME_ELEMENT(KEY_PROG3),
	NAME_ELEMENT(KEY_PROG4),		NAME_ELEMENT(KEY_SUSPEND),
	NAME_ELEMENT(KEY_CLOSE),		NAME_ELEMENT(KEY_PLAY),
	NAME_ELEMENT(KEY_FASTFORWARD),		NAME_ELEMENT(KEY_BASSBOOST),
	NAME_ELEMENT(KEY_PRINT),		NAME_ELEMENT(KEY_HP),
	NAME_ELEMENT(KEY_CAMERA),		NAME_ELEMENT(KEY_SOUND),
	NAME_ELEMENT(KEY_QUESTION),		NAME_ELEMENT(KEY_EMAIL),
	NAME_ELEMENT(KEY_CHAT),			NAME_ELEMENT(KEY_SEARCH),
	NAME_ELEMENT(KEY_CONNECT),		NAME_ELEMENT(KEY_FINANCE),
	NAME_ELEMENT(KEY_SPORT),		NAME_ELEMENT(KEY_SHOP),
	NAME_ELEMENT(KEY_ALTERASE),		NAME_ELEMENT(KEY_CANCEL),
	NAME_ELEMENT(KEY_BRIGHTNESSDOWN),	NAME_ELEMENT(KEY_BRIGHTNESSUP),
	NAME_ELEMENT(KEY_MEDIA),		NAME_ELEMENT(KEY_UNKNOWN),
	NAME_ELEMENT(KEY_OK),
	NAME_ELEMENT(KEY_SELECT),		NAME_ELEMENT(KEY_GOTO),
	NAME_ELEMENT(KEY_CLEAR),		NAME_ELEMENT(KEY_POWER2),
	NAME_ELEMENT(KEY_OPTION),		NAME_ELEMENT(KEY_INFO),
	NAME_ELEMENT(KEY_TIME),			NAME_ELEMENT(KEY_VENDOR),
	NAME_ELEMENT(KEY_ARCHIVE),		NAME_ELEMENT(KEY_PROGRAM),
	NAME_ELEMENT(KEY_CHANNEL),		NAME_ELEMENT(KEY_FAVORITES),
	NAME_ELEMENT(KEY_EPG),			NAME_ELEMENT(KEY_PVR),
	NAME_ELEMENT(KEY_MHP),			NAME_ELEMENT(KEY_LANGUAGE),
	NAME_ELEMENT(KEY_TITLE),		NAME_ELEMENT(KEY_SUBTITLE),
	NAME_ELEMENT(KEY_ANGLE),		NAME_ELEMENT(KEY_ZOOM),
	NAME_ELEMENT(KEY_MODE),			NAME_ELEMENT(KEY_KEYBOARD),
	NAME_ELEMENT(KEY_SCREEN),		NAME_ELEMENT(KEY_PC),
	NAME_ELEMENT(KEY_TV),			NAME_ELEMENT(KEY_TV2),
	NAME_ELEMENT(KEY_VCR),			NAME_ELEMENT(KEY_VCR2),
	NAME_ELEMENT(KEY_SAT),			NAME_ELEMENT(KEY_SAT2),
	NAME_ELEMENT(KEY_CD),			NAME_ELEMENT(KEY_TAPE),
	NAME_ELEMENT(KEY_RADIO),		NAME_ELEMENT(KEY_TUNER),
	NAME_ELEMENT(KEY_PLAYER),		NAME_ELEMENT(KEY_TEXT),
	NAME_ELEMENT(KEY_DVD),			NAME_ELEMENT(KEY_AUX),
	NAME_ELEMENT(KEY_MP3),			NAME_ELEMENT(KEY_AUDIO),
	NAME_ELEMENT(KEY_VIDEO),		NAME_ELEMENT(KEY_DIRECTORY),
	NAME_ELEMENT(KEY_LIST),			NAME_ELEMENT(KEY_MEMO),
	NAME_ELEMENT(KEY_CALENDAR),		NAME_ELEMENT(KEY_RED),
	NAME_ELEMENT(KEY_GREEN),		NAME_ELEMENT(KEY_YELLOW),
	NAME_ELEMENT(KEY_BLUE),			NAME_ELEMENT(KEY_CHANNELUP),
	NAME_ELEMENT(KEY_CHANNELDOWN),		NAME_ELEMENT(KEY_FIRST),
	NAME_ELEMENT(KEY_LAST),			NAME_ELEMENT(KEY_AB),
	NAME_ELEMENT(KEY_NEXT),			NAME_ELEMENT(KEY_RESTART),
	NAME_ELEMENT(KEY_SLOW),			NAME_ELEMENT(KEY_SHUFFLE),
	NAME_ELEMENT(KEY_BREAK),		NAME_ELEMENT(KEY_PREVIOUS),
	NAME_ELEMENT(KEY_DIGITS),		NAME_ELEMENT(KEY_TEEN),
	NAME_ELEMENT(KEY_TWEN),			NAME_ELEMENT(KEY_DEL_EOL),
	NAME_ELEMENT(KEY_DEL_EOS),		NAME_ELEMENT(KEY_INS_LINE),
	NAME_ELEMENT(KEY_DEL_LINE),
	NAME_ELEMENT(KEY_VIDEOPHONE),		NAME_ELEMENT(KEY_GAMES),
	NAME_ELEMENT(KEY_ZOOMIN),		NAME_ELEMENT(KEY_ZOOMOUT),
	NAME_ELEMENT(KEY_ZOOMRESET),		NAME_ELEMENT(KEY_WORDPROCESSOR),
	NAME_ELEMENT(KEY_EDITOR),		NAME_ELEMENT(KEY_SPREADSHEET),
	NAME_ELEMENT(KEY_GRAPHICSEDITOR), 	NAME_ELEMENT(KEY_PRESENTATION),
	NAME_ELEMENT(KEY_DATABASE),		NAME_ELEMENT(KEY_NEWS),
	NAME_ELEMENT(KEY_VOICEMAIL),		NAME_ELEMENT(KEY_ADDRESSBOOK),
	NAME_ELEMENT(KEY_MESSENGER),		NAME_ELEMENT(KEY_DISPLAYTOGGLE),
#ifdef KEY_SPELLCHECK
	NAME_ELEMENT(KEY_SPELLCHECK),
#endif
#ifdef KEY_LOGOFF
	NAME_ELEMENT(KEY_LOGOFF),
#endif
#ifdef KEY_DOLLAR
	NAME_ELEMENT(KEY_DOLLAR),
#endif
#ifdef KEY_EURO
	NAME_ELEMENT(KEY_EURO),
#endif
#ifdef KEY_FRAMEBACK
	NAME_ELEMENT(KEY_FRAMEBACK),
#endif
#ifdef KEY_FRAMEFORWARD
	NAME_ELEMENT(KEY_FRAMEFORWARD),
#endif
#ifdef KEY_CONTEXT_MENU
	NAME_ELEMENT(KEY_CONTEXT_MENU),
#endif
#ifdef KEY_MEDIA_REPEAT
	NAME_ELEMENT(KEY_MEDIA_REPEAT),
#endif
#ifdef KEY_10CHANNELSUP
	NAME_ELEMENT(KEY_10CHANNELSUP),
#endif
#ifdef KEY_10CHANNELSDOWN
	NAME_ELEMENT(KEY_10CHANNELSDOWN),
#endif
#ifdef KEY_IMAGES
	NAME_ELEMENT(KEY_IMAGES),
#endif
	NAME_ELEMENT(KEY_DEL_EOL),		NAME_ELEMENT(KEY_DEL_EOS),
	NAME_ELEMENT(KEY_INS_LINE),	 	NAME_ELEMENT(KEY_DEL_LINE),
	NAME_ELEMENT(KEY_FN),			NAME_ELEMENT(KEY_FN_ESC),
	NAME_ELEMENT(KEY_FN_F1),		NAME_ELEMENT(KEY_FN_F2),
	NAME_ELEMENT(KEY_FN_F3),		NAME_ELEMENT(KEY_FN_F4),
	NAME_ELEMENT(KEY_FN_F5),		NAME_ELEMENT(KEY_FN_F6),
	NAME_ELEMENT(KEY_FN_F7),		NAME_ELEMENT(KEY_FN_F8),
	NAME_ELEMENT(KEY_FN_F9),		NAME_ELEMENT(KEY_FN_F10),
	NAME_ELEMENT(KEY_FN_F11),		NAME_ELEMENT(KEY_FN_F12),
	NAME_ELEMENT(KEY_FN_1),			NAME_ELEMENT(KEY_FN_2),
	NAME_ELEMENT(KEY_FN_D),			NAME_ELEMENT(KEY_FN_E),
	NAME_ELEMENT(KEY_FN_F),			NAME_ELEMENT(KEY_FN_S),
	NAME_ELEMENT(KEY_FN_B),
	NAME_ELEMENT(KEY_BRL_DOT1),		NAME_ELEMENT(KEY_BRL_DOT2),
	NAME_ELEMENT(KEY_BRL_DOT3),		NAME_ELEMENT(KEY_BRL_DOT4),
	NAME_ELEMENT(KEY_BRL_DOT5),		NAME_ELEMENT(KEY_BRL_DOT6),
	NAME_ELEMENT(KEY_BRL_DOT7),		NAME_ELEMENT(KEY_BRL_DOT8),
	NAME_ELEMENT(KEY_BRL_DOT9),		NAME_ELEMENT(KEY_BRL_DOT10),
#ifdef KEY_NUMERIC_0
	NAME_ELEMENT(KEY_NUMERIC_0),		NAME_ELEMENT(KEY_NUMERIC_1),
	NAME_ELEMENT(KEY_NUMERIC_2),		NAME_ELEMENT(KEY_NUMERIC_3),
	NAME_ELEMENT(KEY_NUMERIC_4),		NAME_ELEMENT(KEY_NUMERIC_5),
	NAME_ELEMENT(KEY_NUMERIC_6),		NAME_ELEMENT(KEY_NUMERIC_7),
	NAME_ELEMENT(KEY_NUMERIC_8),		NAME_ELEMENT(KEY_NUMERIC_9),
	NAME_ELEMENT(KEY_NUMERIC_STAR),		NAME_ELEMENT(KEY_NUMERIC_POUND),
#endif
	NAME_ELEMENT(KEY_BATTERY),
	NAME_ELEMENT(KEY_BLUETOOTH),		NAME_ELEMENT(KEY_BRIGHTNESS_CYCLE),
	NAME_ELEMENT(KEY_BRIGHTNESS_ZERO),
#ifdef KEY_DASHBOARD
	NAME_ELEMENT(KEY_DASHBOARD),
#endif
	NAME_ELEMENT(KEY_DISPLAY_OFF),		NAME_ELEMENT(KEY_DOCUMENTS),
	NAME_ELEMENT(KEY_FORWARDMAIL),		NAME_ELEMENT(KEY_NEW),
	NAME_ELEMENT(KEY_KBDILLUMDOWN),		NAME_ELEMENT(KEY_KBDILLUMUP),
	NAME_ELEMENT(KEY_KBDILLUMTOGGLE), 	NAME_ELEMENT(KEY_REDO),
	NAME_ELEMENT(KEY_REPLY),		NAME_ELEMENT(KEY_SAVE),
#ifdef KEY_SCALE
	NAME_ELEMENT(KEY_SCALE),
#endif
	NAME_ELEMENT(KEY_SEND),
	NAME_ELEMENT(KEY_SCREENLOCK),		NAME_ELEMENT(KEY_SWITCHVIDEOMODE),
#ifdef KEY_UWB
	NAME_ELEMENT(KEY_UWB),
#endif
#ifdef KEY_VIDEO_NEXT
	NAME_ELEMENT(KEY_VIDEO_NEXT),
#endif
#ifdef KEY_VIDEO_PREV
	NAME_ELEMENT(KEY_VIDEO_PREV),
#endif
#ifdef KEY_WIMAX
	NAME_ELEMENT(KEY_WIMAX),
#endif
#ifdef KEY_WLAN
	NAME_ELEMENT(KEY_WLAN),
#endif
#ifdef KEY_RFKILL
	NAME_ELEMENT(KEY_RFKILL),
#endif
#ifdef KEY_MICMUTE
	NAME_ELEMENT(KEY_MICMUTE),
#endif
#ifdef KEY_CAMERA_FOCUS
	NAME_ELEMENT(KEY_CAMERA_FOCUS),
#endif
#ifdef KEY_WPS_BUTTON
	NAME_ELEMENT(KEY_WPS_BUTTON),
#endif
#ifdef KEY_TOUCHPAD_TOGGLE
	NAME_ELEMENT(KEY_TOUCHPAD_TOGGLE),
	NAME_ELEMENT(KEY_TOUCHPAD_ON),
	NAME_ELEMENT(KEY_TOUCHPAD_OFF),
#endif
#ifdef KEY_CAMERA_ZOOMIN
	NAME_ELEMENT(KEY_CAMERA_ZOOMIN),	NAME_ELEMENT(KEY_CAMERA_ZOOMOUT),
	NAME_ELEMENT(KEY_CAMERA_UP),		NAME_ELEMENT(KEY_CAMERA_DOWN),
	NAME_ELEMENT(KEY_CAMERA_LEFT),		NAME_ELEMENT(KEY_CAMERA_RIGHT),
#endif
#ifdef KEY_ATTENDANT_ON
	NAME_ELEMENT(KEY_ATTENDANT_ON),		NAME_ELEMENT(KEY_ATTENDANT_OFF),
	NAME_ELEMENT(KEY_ATTENDANT_TOGGLE),	NAME_ELEMENT(KEY_LIGHTS_TOGGLE),
#endif
/*
	NAME_ELEMENT(BTN_0),			NAME_ELEMENT(BTN_1),
	NAME_ELEMENT(BTN_2),			NAME_ELEMENT(BTN_3),
	NAME_ELEMENT(BTN_4),			NAME_ELEMENT(BTN_5),
	NAME_ELEMENT(BTN_6),			NAME_ELEMENT(BTN_7),
	NAME_ELEMENT(BTN_8),			NAME_ELEMENT(BTN_9),
	NAME_ELEMENT(BTN_LEFT),			NAME_ELEMENT(BTN_RIGHT),
	NAME_ELEMENT(BTN_MIDDLE),		NAME_ELEMENT(BTN_SIDE),
	NAME_ELEMENT(BTN_EXTRA),		NAME_ELEMENT(BTN_FORWARD),
	NAME_ELEMENT(BTN_BACK),			NAME_ELEMENT(BTN_TASK),
	NAME_ELEMENT(BTN_TRIGGER),		NAME_ELEMENT(BTN_THUMB),
	NAME_ELEMENT(BTN_THUMB2),		NAME_ELEMENT(BTN_TOP),
	NAME_ELEMENT(BTN_TOP2),			NAME_ELEMENT(BTN_PINKIE),
	NAME_ELEMENT(BTN_BASE),			NAME_ELEMENT(BTN_BASE2),
	NAME_ELEMENT(BTN_BASE3),		NAME_ELEMENT(BTN_BASE4),
	NAME_ELEMENT(BTN_BASE5),		NAME_ELEMENT(BTN_BASE6),
	NAME_ELEMENT(BTN_DEAD),			NAME_ELEMENT(BTN_C),
#ifdef BTN_SOUTH
	NAME_ELEMENT(BTN_SOUTH),		NAME_ELEMENT(BTN_EAST),
	NAME_ELEMENT(BTN_NORTH),		NAME_ELEMENT(BTN_WEST),
#else
	NAME_ELEMENT(BTN_A),			NAME_ELEMENT(BTN_B),
	NAME_ELEMENT(BTN_X),			NAME_ELEMENT(BTN_Y),
#endif
	NAME_ELEMENT(BTN_Z),			NAME_ELEMENT(BTN_TL),
	NAME_ELEMENT(BTN_TR),			NAME_ELEMENT(BTN_TL2),
	NAME_ELEMENT(BTN_TR2),			NAME_ELEMENT(BTN_SELECT),
	NAME_ELEMENT(BTN_START),		NAME_ELEMENT(BTN_MODE),
	NAME_ELEMENT(BTN_THUMBL),		NAME_ELEMENT(BTN_THUMBR),
	NAME_ELEMENT(BTN_TOOL_PEN),		NAME_ELEMENT(BTN_TOOL_RUBBER),
	NAME_ELEMENT(BTN_TOOL_BRUSH),		NAME_ELEMENT(BTN_TOOL_PENCIL),
	NAME_ELEMENT(BTN_TOOL_AIRBRUSH),	NAME_ELEMENT(BTN_TOOL_FINGER),
	NAME_ELEMENT(BTN_TOOL_MOUSE),		NAME_ELEMENT(BTN_TOOL_LENS),
	NAME_ELEMENT(BTN_TOUCH),		NAME_ELEMENT(BTN_STYLUS),
	NAME_ELEMENT(BTN_STYLUS2),		NAME_ELEMENT(BTN_TOOL_DOUBLETAP),
	NAME_ELEMENT(BTN_TOOL_TRIPLETAP),
#ifdef BTN_TOOL_QUADTAP
	NAME_ELEMENT(BTN_TOOL_QUADTAP),
#endif
	NAME_ELEMENT(BTN_GEAR_DOWN),
	NAME_ELEMENT(BTN_GEAR_UP),

#ifdef BTN_DPAD_UP
	NAME_ELEMENT(BTN_DPAD_UP),		NAME_ELEMENT(BTN_DPAD_DOWN),
	NAME_ELEMENT(BTN_DPAD_LEFT),		NAME_ELEMENT(BTN_DPAD_RIGHT),
#endif
#ifdef KEY_ALS_TOGGLE
	NAME_ELEMENT(KEY_ALS_TOGGLE),
#endif
#ifdef KEY_BUTTONCONFIG
	NAME_ELEMENT(KEY_BUTTONCONFIG),
#endif
#ifdef KEY_TASKMANAGER
	NAME_ELEMENT(KEY_TASKMANAGER),
#endif
#ifdef KEY_JOURNAL
	NAME_ELEMENT(KEY_JOURNAL),
#endif
#ifdef KEY_CONTROLPANEL
	NAME_ELEMENT(KEY_CONTROLPANEL),
#endif
#ifdef KEY_APPSELECT
	NAME_ELEMENT(KEY_APPSELECT),
#endif
#ifdef KEY_SCREENSAVER
	NAME_ELEMENT(KEY_SCREENSAVER),
#endif
#ifdef KEY_VOICECOMMAND
	NAME_ELEMENT(KEY_VOICECOMMAND),
#endif
#ifdef KEY_BRIGHTNESS_MIN
	NAME_ELEMENT(KEY_BRIGHTNESS_MIN),
#endif
#ifdef KEY_BRIGHTNESS_MAX
	NAME_ELEMENT(KEY_BRIGHTNESS_MAX),
#endif
#ifdef KEY_KBDINPUTASSIST_PREV
	NAME_ELEMENT(KEY_KBDINPUTASSIST_PREV),
#endif
#ifdef KEY_KBDINPUTASSIST_NEXT
	NAME_ELEMENT(KEY_KBDINPUTASSIST_NEXT),
#endif
#ifdef KEY_KBDINPUTASSIST_PREVGROUP
	NAME_ELEMENT(KEY_KBDINPUTASSIST_PREVGROUP),
#endif
#ifdef KEY_KBDINPUTASSIST_NEXTGROUP
	NAME_ELEMENT(KEY_KBDINPUTASSIST_NEXTGROUP),
#endif
#ifdef KEY_KBDINPUTASSIST_ACCEPT
	NAME_ELEMENT(KEY_KBDINPUTASSIST_ACCEPT),
#endif
#ifdef KEY_KBDINPUTASSIST_CANCEL
	NAME_ELEMENT(KEY_KBDINPUTASSIST_CANCEL),
#endif
#ifdef BTN_TRIGGER_HAPPY
	NAME_ELEMENT(BTN_TRIGGER_HAPPY1),	NAME_ELEMENT(BTN_TRIGGER_HAPPY11),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY2),	NAME_ELEMENT(BTN_TRIGGER_HAPPY12),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY3),	NAME_ELEMENT(BTN_TRIGGER_HAPPY13),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY4),	NAME_ELEMENT(BTN_TRIGGER_HAPPY14),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY5),	NAME_ELEMENT(BTN_TRIGGER_HAPPY15),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY6),	NAME_ELEMENT(BTN_TRIGGER_HAPPY16),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY7),	NAME_ELEMENT(BTN_TRIGGER_HAPPY17),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY8),	NAME_ELEMENT(BTN_TRIGGER_HAPPY18),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY9),	NAME_ELEMENT(BTN_TRIGGER_HAPPY19),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY10),	NAME_ELEMENT(BTN_TRIGGER_HAPPY20),

	NAME_ELEMENT(BTN_TRIGGER_HAPPY21),	NAME_ELEMENT(BTN_TRIGGER_HAPPY31),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY22),	NAME_ELEMENT(BTN_TRIGGER_HAPPY32),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY23),	NAME_ELEMENT(BTN_TRIGGER_HAPPY33),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY24),	NAME_ELEMENT(BTN_TRIGGER_HAPPY34),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY25),	NAME_ELEMENT(BTN_TRIGGER_HAPPY35),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY26),	NAME_ELEMENT(BTN_TRIGGER_HAPPY36),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY27),	NAME_ELEMENT(BTN_TRIGGER_HAPPY37),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY28),	NAME_ELEMENT(BTN_TRIGGER_HAPPY38),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY29),	NAME_ELEMENT(BTN_TRIGGER_HAPPY39),
	NAME_ELEMENT(BTN_TRIGGER_HAPPY30),	NAME_ELEMENT(BTN_TRIGGER_HAPPY40),
#endif
#ifdef BTN_TOOL_QUINTTAP
	NAME_ELEMENT(BTN_TOOL_QUINTTAP),
#endif*/
};

void fprint_namespace(void)
{
	int i;
	for (i = 0; i < KEY_MAX; i++)
		if (keys[i] != NULL)
			fprintf(stdout, "%s\n", keys[i]);
}

char *s_strdup(char *string)
{
	char *ptr;
	if (!(ptr = strdup(string))) {
		fprintf(stderr, "Error %s (%d) %s(): out of memory\n", __FILE__, __LINE__, __FUNCTION__);
		config_parse_error = 1;
		return (NULL);
	}
	return (ptr);
}

int s_strtoi(char *val)
{
	char *endptr;
	long n;
	int h;

	n = strtol(val, &endptr, 0);
	h = (int)n;
	if (!*val || *endptr || n != ((long)h)) {
		fprintf(stderr, "Error %s (%d) %s(): in configfile line %d\n", __FILE__, __LINE__, __FUNCTION__, config_line);
		fprintf(stdout, "\"%s\": must be a valid (int) number", val);
		config_parse_error = 1;
		return (0);
	}
	return (h);
}

int get_input_code(const char *key)
{
	int i;
	for (i = 0; i < KEY_MAX; i++)
		if (keys[i] != NULL && !strcasecmp(keys[i], key))
			return i;
	return -1;
}

const char *get_input_name(int code)
{
	if (!code || code > KEY_MAX)
		return 0;
	return keys[code];
}

// 0 - already exist
// 1 - added
int vd_config_add_button(struct vd_config *config, char *key, int scancode)
{
	struct vk_node *node;

	node = config->vks;
	while (node != NULL) {
		if (node->scancode == scancode)
			return 0;
		node = node->next;
	}

	node = malloc(sizeof(struct vk_node));
	node->key = key;
	node->scancode = scancode;
	// insert at index 0
	node->next = config->vks;
	config->vks = node;

	return 1;
}

int vd_config_read(FILE * f, struct vd_config *config)
{
	char buf[LINE_LEN + 1], *key, *val, *val2;
	int len, argc, cur;

	cur = ID_NONE;
	config_line = 0;
	config_parse_error = 0;

	while (fgets(buf, LINE_LEN, f) != NULL) {
		config_line++;
		len = strlen(buf);
		if (len == LINE_LEN && buf[len - 1] != '\n') {
			fprintf(stderr, "Error %s (%d) %s(): line %d too long in config file\n", __FILE__, __LINE__, __FUNCTION__, config_line);
			config_parse_error = 1;
			break;
		}

		if (len > 0) {
			len--;
			if (buf[len] == '\n')
				buf[len] = 0;
		}
		if (len > 0) {
			len--;
			if (buf[len] == '\r')
				buf[len] = 0;
		}
		/* ignore comments */
		if (buf[0] == '#') {
			continue;
		}
		key = strtok(buf, whitespace);
		/* ignore empty lines */
		if (key == NULL)
			continue;
		val = strtok(NULL, whitespace);
		if (val != NULL) {
			val2 = strtok(NULL, whitespace);
			if (strcasecmp("name", key) == 0) {
				if (config->name == NULL)
					config->name = s_strdup(val);
			} else if (strcasecmp("input", key) == 0) {
				if (config->input == NULL)
					config->input = s_strdup(val);
			} else if (strcasecmp("begin", key) == 0 && strcasecmp("codes", val) == 0) {
				cur = ID_CODES;
			} else if (strcasecmp("end", key) == 0 && strcasecmp("codes", val) == 0) {
				cur = ID_NONE;
			} else {
				switch (cur) {
				case ID_CODES:
					if (get_input_code(key) != 0) {
						vd_config_add_button(config, s_strdup(key), s_strtoi(val));
					} else {
						fprintf(stderr, "Error %s (%d) %s(): in configfile line %d, button %s not exist in list\n", __FILE__, __LINE__, __FUNCTION__, config_line, key);
					}
				}
			}
		} else {
			fprintf(stderr, "Error %s (%d) %s(): in configfile line %d\n", __FILE__, __LINE__, __FUNCTION__, config_line);
			config_parse_error = 1;
			break;
		}
		if (config_parse_error) {
			break;
		}
	}

	return config_parse_error;
}

int vd_config_save(const char *filename, struct vd_config *config)
{
	FILE *fout;
	struct vk_node *node;

	if ((fout = fopen(filename, "w")) == NULL) {
		fprintf(stderr, "Error %s (%d) %s(): save config to %s failed.\n", __FILE__, __LINE__, __FUNCTION__, filename);
		return -1;
	}

	fprintf(fout, "name %s\n", config->name);
	fprintf(fout, "input %s\n", config->input);

	node = config->vks;
	fprintf(fout, "begin codes\n");
	while (node != NULL) {
		fprintf(fout, "  %-20s 0x%08X\n", node->key, node->scancode);
		node = node->next;
	}
	fprintf(fout, "end codes\n");
	fflush(fout);
	fclose(fout);
	return 0;
}

void vd_config_table_rebuild(struct vd_config *config)
{
	unsigned int size;
	struct vk_node *node;

	if (config == NULL)
		return;

	if (config->table != NULL)
		free(config->table);

	if ((node = config->vks) == NULL)
		return;

	// calc min max
	config->min = INT_MAX;
	while (node != NULL)
	{
		if (node->scancode < config->min)
			config->min = node->scancode;
		else if (node->scancode > config->max)
			config->max = node->scancode;
		node = node->next;
	}

	// calc table size and allocate in memory
	size = config->max - config->min;
	if (size > 0xFFFF) {
		fprintf(stderr, "Error %s (%d) %s(): big size of keys table, %d\n", __FILE__, __LINE__, __FUNCTION__, size);
		return;
	}
	size = size * sizeof(config->table);
	config->table = malloc(sizeof(config->table));
	memset(config->table, 0, size);

	node = config->vks;
	while (node != NULL)
	{
		config->table[node->scancode - config->min] = get_input_code(node->key);
		node = node->next;
	}
}

/*
* virtual_device
*/
int vd_create(struct vd_config *config)
{
	int fd, keycode;
	struct vk_node *node;
	struct uinput_user_dev vd_uinput;

	if ((fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK)) < 0) {
		fprintf(stderr, "Error %s (%d) %s(): could not open %s\n", __FILE__, __LINE__, __FUNCTION__, "/dev/uinput");
		return -1;
	}

	if (ioctl(fd, UI_SET_EVBIT, EV_KEY) == -1) {
		fprintf(stderr, "Error %s (%d) %s(): ioctl(fd, UI_SET_EVBIT, EV_KEY)\n", __FILE__, __LINE__, __FUNCTION__);
		close(fd);
		return -1;
	}

	if (ioctl(fd, UI_SET_EVBIT, EV_SYN) == -1) {
		fprintf(stderr, "Error %s (%d) %s(): ioctl(fd, UI_SET_EVBIT, EV_SYN)\n", __FILE__, __LINE__, __FUNCTION__);
		close(fd);
		return -1;
	}

	node = config->vks;
	while (node != NULL)
	{
		if ((keycode = get_input_code(node->key)) != -1) {
			if (ioctl(fd, UI_SET_KEYBIT, keycode) == -1) {
				fprintf(stderr, "Error %s (%d) %s(): ioctl(fd, UI_SET_KEYBIT, %d)\n", __FILE__, __LINE__, __FUNCTION__, keycode);
				close(fd);
				return -1;
			}
		} else {
			fprintf(stderr, "Error %s (%d) %s(): unknown key name %s, %d\n", __FILE__, __LINE__, __FUNCTION__, node->key, node->scancode);
		}
		node = node->next;
	}

	memset(&vd_uinput, 0, sizeof(struct uinput_user_dev));
	strncpy(vd_uinput.name, config->name, UINPUT_MAX_NAME_SIZE);
	vd_uinput.id.bustype	= BUS_USB;
	vd_uinput.id.vendor		= 0x99a; /* dummy vendor */
	vd_uinput.id.product	= 0x7501; /* dummy product */
	vd_uinput.id.version	= 0x100;
	//if (config->phys != NULL)
	//	vd_uinput.id.phys = "sunxi_ir_keyboard/input0";

	if (write(fd, &vd_uinput, sizeof(vd_uinput)) < 0) {
		fprintf(stderr, "Error %s (%d) %s(): setup virtual device\n", __FILE__, __LINE__, __FUNCTION__);
		close(fd);
		return -1;
	}

	if (ioctl(fd, UI_DEV_CREATE) == -1) {
		fprintf(stderr, "Error %s (%d) %s(): create virtual device\n", __FILE__, __LINE__, __FUNCTION__);
		close(fd);
		return -1;
	}

	sleep(1);

	return fd;
}

void vd_send_event(int fd, int type, int code, int value)//, long tv_sec, long tv_usec)
{
	struct input_event ev;
	ev.type = type;
	ev.code = code;
	ev.value = value;
	//ev.time.tv_sec = 0;
	//ev.time.tv_usec = 0;
	if (write(fd, &ev, sizeof(ev)) < 0)
		fprintf(stderr, "Error %s (%d) %s(): write()\n", __FILE__, __LINE__, __FUNCTION__);
}

void vd_destroy(int fd)
{
	if (ioctl(fd, UI_DEV_DESTROY) == -1)
		fprintf(stderr, "Error %s (%d) %s(): destroy virtual device\n", __FILE__, __LINE__, __FUNCTION__);
	close(fd);
}

/*
* virtual_device_input_event
*/
static volatile sig_atomic_t stop = 0;

static void interrupt_handler(int sig)
{
	stop = 1;
}

int test_grab(int fd, int grab_flag)
{
	int rc;

	rc = ioctl(fd, EVIOCGRAB, (void*)1);

	if (rc == 0 && !grab_flag)
		ioctl(fd, EVIOCGRAB, (void*)0);

	return rc;
}

int input_event_open(const char *phys)
{
	int fd;

	if ((fd = open(phys, O_RDONLY)) < 0) {
		fprintf(stderr, "Error %s (%d) %s(): open input device %s\n", __FILE__, __LINE__, __FUNCTION__, phys);
		if (errno == EACCES && getuid() != 0)
			fprintf(stderr, "You do not have access to %s. Try "
					"running as root instead.\n",
					phys);
		return -1;
	}

	if (!isatty(fileno(stdout)))
		setbuf(stdout, NULL);

	return fd;
}

int input_event_read(int fd, struct input_event *ev, size_t size, struct timeval *timeout)
{
	int rd, ret;
	fd_set rdfs;

	FD_ZERO(&rdfs);
	FD_SET(fd, &rdfs);
	ret = select(fd + 1, &rdfs, NULL, NULL, timeout);

	// !todo
	if (ret <= 0 || (!ret && timeout))
		return ret;

	//if (!FD_ISSET(fd, &rdfs)) {
	//	fprintf(stderr, "Error %s (%d) %s(): FD_ISSET() == 0\n", __FILE__, __LINE__, __FUNCTION__);
	//	return -1;
	//}
	// odot!

	if ((rd = read(fd, ev, size)) < size) {//(int)sizeof(struct input_event)) {
		fprintf(stderr, "Error %s (%d) %s(): failed to read input event\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	//fprintf(stdout, "event: %02X, %04X, %i", ev->type, ev->code, ev->value);

	return 1;
}

void input_event_close(int fd)
{
	if (fd != -1)
        close(fd);
}

/*
* virtual_device_main
*/
char *read_stdin()
{
	char *ptr, buffer[1024];
	memset(buffer, 0, 1024);
	ptr = fgets(buffer, 1023, stdin);
	if (ptr != buffer) {
		fprintf(stderr, "Error %s (%d) %s(): fgets() failed\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	if (ptr[0] == '\n')
		return 0;
	buffer[strlen(buffer) - 1] = 0;
	return ptr;
}

int main(int argc, const char *argv[])
{
	char *string;
	int ret, i, vd_fd = -1, sunxi_ir_event_fd = -1;
	struct vd_config config = {0, 0, 0, 0, INT_MAX, 0};

	struct timeval timeout;
	struct input_event ev;
	int key_code, scancode_index;

	char create_config = 0;
	FILE *config_file;
	const char *config_path = NULL;

	for (i = 1; i < argc; i++) {
		if (strcasecmp("--list", argv[i]) == 0) {
			fprint_namespace();
			return 0;
		} else if (strcasecmp("--name", argv[i]) == 0) {
			config.name = argv[++i];
		} else if (strcasecmp("--input", argv[i]) == 0) {
			config.input = argv[++i];
		} else if (strcasecmp("--config", argv[i]) == 0) {
			config_path = argv[++i];
		} else if (strcasecmp("--create", argv[i]) == 0) {
			printf("Creating new config.\n");
			create_config = 1;
		}
	}

load_config:
	if (config_path != NULL) {
		if ((config_file = fopen(config_path, "r")) == NULL) {
			if (create_config == 0) {
				fprintf(stderr, "Error %s (%d) %s(): open config file %s\n", __FILE__, __LINE__, __FUNCTION__, config_path);
				config_path = NULL;
			}
		} else if (vd_config_read(config_file, &config)) {
			fprintf(stderr, "Error %s (%d) %s(): reading config file %s\n", __FILE__, __LINE__, __FUNCTION__, config_path);
			fclose(config_file);
			config_path = NULL;
		} else {
			fclose(config_file);
		}
	}

open_input_device:
	if (config.input != NULL && (sunxi_ir_event_fd = input_event_open(config.input)) >= 0) {
		if (test_grab(sunxi_ir_event_fd, 1)) {
			fprintf(stdout, "***********************************************\n");
			fprintf(stdout, "  This device is grabbed by another process.\n");
			fprintf(stdout, "  No events are available to %s while the\n"
							"  other grab is active.\n", argv[0]);
			fprintf(stdout, "  In most cases, this is caused by an X driver,\n"
							"  try VT-switching and re-run evtest again.\n");
			fprintf(stdout, "  Run the following command to see processes with\n"
							"  an open fd on this device\n"
							" \"fuser -v %s\"\n", config.input);
			fprintf(stdout, "***********************************************\n");
		}
	}

	while (create_config) {
		if (config_path == NULL) {
			printf("\nPlease enter the config file path (example: /etc/virtual_input.conf)\n");
			string = read_stdin();
			if (!string) break;
			if (strchr(string, ' ') || strchr(string, '\t')) {
				printf("The path must not contain any whitespace.\n");
				printf("Please try again.\n");
				continue;
			}
			config_path = s_strdup(string);
			goto load_config;
		}

		if (config.name == NULL) {
			printf("\nPlease enter the device name (example: IR-Keyboard)\n");
			string = read_stdin();
			if (!string) break;
			if (strchr(string, ' ') || strchr(string, '\t')) {
				printf("The name must not contain any whitespace.\n");
				printf("Please try again.\n");
				continue;
			}
			config.name = s_strdup(string);
			continue;
		}

		if (config.input == NULL) {
			printf("\nPlease enter the input device path (example: /dev/input/event6)\n");
			string = read_stdin();
			if (!string) break;
			if (strchr(string, ' ') || strchr(string, '\t')) {
				printf("The path must not contain any whitespace.\n");
				printf("Please try again.\n");
				continue;
			}
			config.input = s_strdup(string);
			goto open_input_device;
		}

		if (sunxi_ir_event_fd < 0)
		{
			printf("Could not opend input device.\n");
			printf("Please try again.\n");
			config.input = NULL;
			continue;
		} else {
			printf("\nPlease enter the name for the button (or press <ENTER> to complete the setting)\n");
			string = read_stdin();
			if (!string) break;
			if (!get_input_code(string)) {
				printf("The button name must contain any button from the list, %s --list.\n", argv[0]);
				printf("Please try again.\n");
				continue;
			}
			string = s_strdup(string);
			printf("\nNow click the button for \"%s\", you have 30 seconds for this.\n", string);
read_ev:
			timeout.tv_usec = 0;
			timeout.tv_sec = 30;
			memset(&ev, 0, sizeof(struct input_event));
			ret = input_event_read(sunxi_ir_event_fd, &ev, sizeof(struct input_event), &timeout);
			if (ret == 1) {
				if (ev.type == EV_MSC && (ev.code == MSC_RAW || ev.code == MSC_SCAN)) {
					if (vd_config_add_button(&config, string, ev.value))
						printf("New button %-20s 0x%08X added.\n", string, (unsigned int)ev.value);
					else
						printf("Button %-20s 0x%08X already exist.\n", string, (unsigned int)ev.value);
				} else {
					goto read_ev;
				}
			} else if (ret == 0) {
				printf("Timeout...\n");
				break;
			} else {
				printf("Error?\n");
				printf("input_event_read(sunxi_ir_event_fd, &ev, sizeof(struct input_event), &timeout) != 1 != 0\n");
				break;
			}
			continue;
		}
	}

	if (create_config) {
		printf("New config file saved to: %s\n", config_path);
		vd_config_save(config_path, &config);
	}

	if (config_path != NULL) {
		if (sunxi_ir_event_fd >= 0) {
			vd_config_table_rebuild(&config);
			if ((vd_fd = vd_create(&config)) >= 0) {
				stop = 0;

				signal(SIGINT, interrupt_handler);
				signal(SIGTERM, interrupt_handler);

				signal(SIGABRT, interrupt_handler);
				signal(SIGQUIT, interrupt_handler);
				signal(SIGHUP, interrupt_handler);

				while (!stop) {
					memset(&ev, 0, sizeof(struct input_event));
					if (input_event_read(sunxi_ir_event_fd, &ev, sizeof(struct input_event), NULL)) {
						if (ev.type == EV_MSC && (ev.code == MSC_RAW || ev.code == MSC_SCAN)) {
							scancode_index = ev.value;
							if (scancode_index >= config.min && scancode_index <= config.max) {
								scancode_index = scancode_index - config.min;
								if ((key_code = config.table[scancode_index]) != 0) {
									vd_send_event(vd_fd, EV_KEY, key_code, 1);
									vd_send_event(vd_fd, EV_SYN, SYN_REPORT, 0);
									usleep(16);
									vd_send_event(vd_fd, EV_KEY, key_code, 0);
									vd_send_event(vd_fd, EV_SYN, SYN_REPORT, 0);
								}
							}
						}
					} else {
						fprintf(stderr, "Error %s (%d) %s(): input_event_read() != 1\n", __FILE__, __LINE__, __FUNCTION__);
						break;
					}
				}
				vd_destroy(vd_fd);
			}
			ioctl(sunxi_ir_event_fd, EVIOCGRAB, (void*)0); // no need if arg2 == 0 in test_grab()
			input_event_close(sunxi_ir_event_fd);
		}
	} else {
		printf("Usage: github.com/rubitwa/virtual_input_for_ir\n");
	}

	return 0;
}
