#include <gccore.h>

u16 padsButtonsDown() {
	return (
		PAD_ButtonsDown(PAD_CHAN0) |
		PAD_ButtonsDown(PAD_CHAN1) |
		PAD_ButtonsDown(PAD_CHAN2) |
		PAD_ButtonsDown(PAD_CHAN3)
	);
}

u16 padsButtonsHeld() {
	return (
		PAD_ButtonsHeld(PAD_CHAN0) |
		PAD_ButtonsHeld(PAD_CHAN1) |
		PAD_ButtonsHeld(PAD_CHAN2) |
		PAD_ButtonsHeld(PAD_CHAN3)
	);
}

#ifdef HW_RVL
#include <wiiuse/wpad.h>

u32 wpadsButtonsDown() {
	return (
		WPAD_ButtonsDown(WPAD_CHAN0) |
		WPAD_ButtonsDown(WPAD_CHAN1) |
		WPAD_ButtonsDown(WPAD_CHAN2) |
		WPAD_ButtonsDown(WPAD_CHAN3)
	);
}

u32 wpadsButtonsHeld() {
	return (
		WPAD_ButtonsHeld(WPAD_CHAN0) |
		WPAD_ButtonsHeld(WPAD_CHAN1) |
		WPAD_ButtonsHeld(WPAD_CHAN2) |
		WPAD_ButtonsHeld(WPAD_CHAN3)
	);
}
#endif
