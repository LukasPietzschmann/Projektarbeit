#pragma once

#define FOOTER_HEIGHT 1
#define HEADER_HEIGHT 1
#define QUEUE_WIDTH 25
#define POPUP_WIDTH 40
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