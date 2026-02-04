/* psgame
 * Input system
 */

#include <sys/types.h>

#include <libpad.h>
#include <libgte.h>
#include <libgpu.h>

#include "common.h"

#include "input.h"

static int _held_buttons[2] = { 0 };

static void _update(u_int i, u_int mask, u_int bits, u_int player);
static void _default_input_fn(int);

static InputFn _down_fns[2] = { _default_input_fn, _default_input_fn };
static InputFn _hold_fns[2] = { _default_input_fn, _default_input_fn };
static InputFn _up_fns[2] = { _default_input_fn, _default_input_fn };

PadData pad_data[2] = { 0 };

void input_init(void) {
	LOG("* INIT INPUT\n");

	PadInitDirect((u_char *)&pad_data[PLAYER_1], (u_char *)&pad_data[PLAYER_2]);
	PadStartCom();
}

void input_quit(void) {
	PadStopCom();
}

void input_update(void) {
	u_int p1_bits, p2_bits, i;

	p1_bits = (PAD_P1_RAW(2) | (PAD_P1_RAW(3) << 8)) ^ 0xffff;
	p2_bits = (PAD_P2_RAW(2) | (PAD_P2_RAW(3) << 8)) ^ 0xffff;

	for( i = 0; i < 16; ++i ) {
		u_int mask = (1 << i);
		_update(i, mask, p1_bits, PLAYER_1);
		_update(i, mask, p2_bits, PLAYER_2);
	}
}

void input_hook_down(InputFn fn, int player) {
	_down_fns[player] = fn;
}

void input_hook_hold(InputFn fn, int player) {
	_hold_fns[player] = fn;
}

void input_hook_up(InputFn fn, int player) {
	_up_fns[player] = fn;
}

void input_unhook_all(void) {
	u_int i;
	for( i = 0; i < 2; ++i ) {
		_down_fns[i] = _default_input_fn;
		_hold_fns[i] = _default_input_fn;
		_up_fns[i] = _default_input_fn;
	}
}

bool input_is_connected(int port) {
	return PadGetState(port) != PadStateDiscon;
}

static void _update(u_int i, u_int mask, u_int bits, u_int player) {
	if( bits & mask ) {
		_hold_fns[player](i);
		if( _held_buttons[player] & mask ) {
			return;
		}

		_held_buttons[player] |= mask;
		_down_fns[player](i);
	} else if( _held_buttons[player] & mask ) {
		_held_buttons[player] &= ~mask;
		_up_fns[mask](i);
	}
}

static void _default_input_fn(int button) {
	(void)button;
}
