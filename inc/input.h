#ifndef GUARD_PSGAME_INPUT_H_
#define GUARD_PSGAME_INPUT_H_

#include <sys/types.h>

#include "common.h"

#define PLAYER_1 (0)
#define PLAYER_2 (1)

#define PORT_1 (0x00)
#define PORT_2 (0x10)

#define PAD_P1(I) pad_data[PLAYER_1].raw[(I)]
#define PAD_P2(I) pad_data[PLAYER_2].raw[(I)]

/* The buttons on a PS1 gamepad */
typedef enum {
	BTN_SELECT = 0,
	BTN_L3 = 1,
	BTN_R3 = 2,
	BTN_START = 3,

	BTN_UP = 4,
	BTN_RIGHT = 5,
	BTN_DOWN = 6,
	BTN_LEFT = 7,

	BTN_L2 = 8,
	BTN_R2 = 9,
	BTN_L1 = 10,
	BTN_R1 = 11,

	BTN_TRIANGLE = 12,
	BTN_CIRCLE = 13,
	BTN_CROSS = 14,
	BTN_SQUARE = 15
} PadButton;

/* Callback function for inputs */
typedef void (*InputFn)(int);

/* Structure with controller data
 *
 * Reference:
 *   https://github.com/sparkletron/PSYQ_Examples/blob/master/psxController.md
 */
typedef union {
	struct {
		u_char status : 8;

		u_char recv_size : 4;
		u_char type : 4;

		u_char select : 1;
		u_char l3 : 1;
		u_char r3 : 1;
		u_char start : 1;
		u_char up : 1;
		u_char right : 1;
		u_char down : 1;
		u_char left : 1;

		u_char l2 : 1;
		u_char r2 : 1;
		u_char l1 : 1;
		u_char r1 : 1;
		u_char triangle : 1;
		u_char circle : 1;
		u_char cross : 1;
		u_char square : 1;
	} value;

	u_char raw[34];
} PadData;

extern PadData pad_data[2];

/* Initializes the input subsystem */
void input_init(void);

/* Quits the input subsystem */
void input_quit(void);

/* Updates the input handlers */
void input_update(void);

/* Hooks a callback function for button presses */
void input_hook_down(InputFn fn, int player);

/* Hooks a callback function for button long presses */
void input_hook_hold(InputFn fn, int player);

/* Hooks a callback function for button releases */
void input_hook_up(InputFn fn, int player);

/* Unhooks all callbacks */
void input_unhook_all(void);

/* Checks if a controller is plugged into a given port */
bool input_is_connected(int port);

#endif // !GUARD_PSGAME_INPUT_H_
