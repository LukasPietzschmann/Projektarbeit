#pragma once

#define FOOTER_HEIGHT 1
#define HEADER_HEIGHT 1
#define QUEUE_WIDTH 20
#define POPUP_WIDTH 60
#define POPUP_HEIGHT 10

#define COLOR_GREY 50
#define COLOR_DARK_GREY 51
#define COLOR_LIGHT_GREY 52

#define STD_COLOR_PAIR 1
#define HEADER_COLOR_PAIR 99
#define FOOTER_COLOR_PAIR 98
#define QUEUE_COLOR_PAIR 97
#define HIGHLIGHT_EXPR_COLOR_PAR 96
#define MUTED_COLOR_PAIR 95
#define AMBIGUOUS_COLOR_PAIR 94
#define POPUP_COLOR_PAIR QUEUE_COLOR_PAIR

#define HIGHLIGHT_EXPR_ATTR (A_STANDOUT | COLOR_PAIR(HIGHLIGHT_EXPR_COLOR_PAR))

#define REPLACE_WITH_ID_THRESHOLD 15

#define KEY_ESCAPE 27
#define KEY_ARROW_UP KEY_UP
#define KEY_ARROW_DOWN KEY_DOWN

#define EVENT_COUNTER_TEXT "Event: %d"
#define FOOTER_QUICK_ACTIONS_TEXT "q: quit    n: next    p: previous    o: toggle opers    h: help"
#define FOOTER_WAIT_FOR_MARKER_TEXT "Input marker or press <esc>"
#define FOOTER_WAIT_FOR_SCROLLABLE_SELECTOR_TEXT "Input Scrollable-Selector a: archives b: queue o: opers) or press <esc>"

#define MAIN_VIEWPORT_SELECTOR 'a'
#define QUEUE_SELECTOR 'b'
#define OPER_POPUP_SELECTOR 'o'