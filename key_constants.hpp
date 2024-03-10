#include <map>
#define KEY_ESC 0x29 // Keyboard ESCAPE
#define KEY_1 0x1e   // Keyboard 1 and !
#define KEY_2 0x1f   // Keyboard 2 and @
#define KEY_3 0x20   // Keyboard 3 and #
#define KEY_4 0x21   // Keyboard 4 and $
#define KEY_5 0x22   // Keyboard 5 and %
#define KEY_6 0x23   // Keyboard 6 and ^
#define KEY_7 0x24   // Keyboard 7 and &
#define KEY_8 0x25   // Keyboard 8 and *
#define KEY_9 0x26   // Keyboard 9 and (
#define KEY_0 0x27   // Keyboard 0 and )
#define KEY_MINUS 0x2d // Keyboard - and _

#define KEY_TAB 0x2b       // Keyboard Tab
#define KEY_Q 0x14         // Keyboard q and Q
#define KEY_W 0x1a         // Keyboard w and W
#define KEY_E 0x08         // Keyboard e and E
#define KEY_R 0x15         // Keyboard r and R
#define KEY_T 0x17         // Keyboard t and T
#define KEY_Y 0x1c         // Keyboard y and Y
#define KEY_U 0x18         // Keyboard u and U
#define KEY_I 0x0c         // Keyboard i and I
#define KEY_O 0x12         // Keyboard o and O
#define KEY_P 0x13         // Keyboard p and P
#define KEY_LEFTBRACE 0x2f // Keyboard [ and {

#define KEY_CAPSLOCK 0x39   // Keyboard Caps Lock
#define KEY_A 0x04          // Keyboard a and A
#define KEY_S 0x16          // Keyboard s and S
#define KEY_D 0x07          // Keyboard d and D
#define KEY_F 0x09          // Keyboard f and F
#define KEY_G 0x0a          // Keyboard g and G
#define KEY_H 0x0b          // Keyboard h and H
#define KEY_J 0x0d          // Keyboard j and J
#define KEY_K 0x0e          // Keyboard k and K
#define KEY_L 0x0f          // Keyboard l and L
#define KEY_SEMICOLON 0x33  // Keyboard ; and :
#define KEY_APOSTROPHE 0x34 // Keyboard ' and "

#define KEY_LEFTSHIFT 0xe1  // Keyboard Left Shift
#define KEY_RIGHTSHIFT 0xe5 // Keyboard Right Shift
#define KEY_Z 0x1d          // Keyboard z and Z
#define KEY_X 0x1b          // Keyboard x and X
#define KEY_C 0x06          // Keyboard c and C
#define KEY_V 0x19          // Keyboard v and V
#define KEY_B 0x05          // Keyboard b and B
#define KEY_N 0x11          // Keyboard n and N
#define KEY_M 0x10          // Keyboard m and M
#define KEY_DOT 0x37        // Keyboard . and >
#define KEY_COMMA 0x36      // Keyboard , and <
#define KEY_SLASH 0x38      // Keyboard / and ?
#define KEY_ENTER 0x28      // Keyboard Return (ENTER)

// keys used for commands
#define KEY_SPACE 0x2c // Keyboard Spacebar

std::map<int, int> HID_number_row_to_value = {
    {KEY_ESC, 0}, // we map the top left key to 0 to match the regular layout
    {KEY_1, 1},
    {KEY_2, 2},
    {KEY_3, 3},
    {KEY_4, 4},
    {KEY_5, 5},
    {KEY_6, 6},
    {KEY_7, 7},
    {KEY_8, 8},
    {KEY_9, 9},
};

std::map<int, int> HID_to_sequential_querty = {
    // number row
    {KEY_ESC, 36},
    {KEY_1, 37},
    {KEY_2, 38},
    {KEY_3, 39},
    {KEY_4, 40},
    {KEY_5, 41},
    {KEY_6, 42},
    {KEY_7, 43},
    {KEY_8, 44},
    {KEY_9, 45},
    {KEY_0, 46},
    {KEY_MINUS, 47},

    // qwer row
    {KEY_TAB, 24},
    {KEY_Q, 25},
    {KEY_W, 26},
    {KEY_E, 27},
    {KEY_R, 28},
    {KEY_T, 29},
    {KEY_Y, 30},
    {KEY_U, 31},
    {KEY_I, 32},
    {KEY_O, 33},
    {KEY_P, 34},
    {KEY_LEFTBRACE, 35},

    // asdf row
    {KEY_CAPSLOCK, 12},
    {KEY_A, 13},
    {KEY_S, 14},
    {KEY_D, 15},
    {KEY_F, 16},
    {KEY_G, 17},
    {KEY_H, 18},
    {KEY_J, 19},
    {KEY_K, 20},
    {KEY_L, 21},
    {KEY_SEMICOLON, 22},
    {KEY_APOSTROPHE, 23},

    // zxcv row
    {KEY_LEFTSHIFT, 0},
    {KEY_Z, 1},
    {KEY_X, 2},
    {KEY_C, 3},
    {KEY_V, 4},
    {KEY_B, 5},
    {KEY_N, 6},
    {KEY_M, 7},
    {KEY_COMMA, 8},
    {KEY_DOT, 9},
    {KEY_SLASH, 10},
    {KEY_RIGHTSHIFT, 11},
};
