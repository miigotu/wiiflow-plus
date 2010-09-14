#define WBTN_A (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)
#define WBTN_B (WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B)
#define WBTN_1 (WPAD_BUTTON_1 | WPAD_CLASSIC_BUTTON_Y)
#define WBTN_2 (WPAD_BUTTON_2 | WPAD_CLASSIC_BUTTON_X)
#define WBTN_PLUS (WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS)
#define WBTN_MINUS (WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS)
#define WBTN_HOME (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)
#define WBTN_UP (WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP)
#define WBTN_RIGHT (WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT)
#define WBTN_DOWN (WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN)
#define WBTN_LEFT (WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT)

#define BTN_A (PAD_BUTTON_A)
#define BTN_B (PAD_BUTTON_B)
#define BTN_1 (PAD_BUTTON_Y)
#define BTN_2 (PAD_BUTTON_X)
#define BTN_PLUS (PAD_TRIGGER_R)
#define BTN_MINUS (PAD_TRIGGER_L)
#define BTN_HOME (PAD_BUTTON_MENU)
#define BTN_UP (PAD_BUTTON_UP)
#define BTN_RIGHT (PAD_BUTTON_RIGHT)
#define BTN_DOWN (PAD_BUTTON_DOWN)
#define BTN_LEFT (PAD_BUTTON_LEFT)

#define BTN_UP_PRESSED ((wii_btnsPressed & WBTN_UP)||(gc_btnsPressed & BTN_UP)!=0)
#define BTN_RIGHT_PRESSED ((wii_btnsPressed & WBTN_RIGHT)||(gc_btnsPressed & BTN_RIGHT)!=0)
#define BTN_DOWN_PRESSED ((wii_btnsPressed & WBTN_DOWN)||(gc_btnsPressed & BTN_DOWN)!=0)
#define BTN_LEFT_PRESSED ((wii_btnsPressed & WBTN_LEFT)||(gc_btnsPressed & BTN_LEFT)!=0)
#define BTN_A_PRESSED ((wii_btnsPressed & WBTN_A)||(gc_btnsPressed & BTN_A)!=0)
#define BTN_B_PRESSED ((wii_btnsPressed & WBTN_B)||(gc_btnsPressed & BTN_B)!=0)
#define BTN_MINUS_PRESSED ((wii_btnsPressed & WBTN_MINUS)||(gc_btnsPressed & BTN_MINUS)!=0)
#define BTN_PLUS_PRESSED ((wii_btnsPressed & WBTN_PLUS)||(gc_btnsPressed & BTN_PLUS)!=0)
#define BTN_HOME_PRESSED ((wii_btnsPressed & WBTN_HOME)||(gc_btnsPressed & BTN_HOME)!=0)
#define BTN_1_PRESSED ((wii_btnsPressed & WBTN_1)||(gc_btnsPressed & BTN_1)!=0)
#define BTN_2_PRESSED ((wii_btnsPressed & WBTN_2)||(gc_btnsPressed & BTN_2)!=0)

#define WBTN_UP_HELD (wii_btnsHeld & WBTN_UP)
#define WBTN_RIGHT_HELD (wii_btnsHeld & WBTN_RIGHT)
#define WBTN_DOWN_HELD (wii_btnsHeld & WBTN_DOWN)
#define WBTN_LEFT_HELD (wii_btnsHeld & WBTN_LEFT)
#define WBTN_A_HELD (wii_btnsHeld & WBTN_A)
#define WBTN_B_HELD (wii_btnsHeld & WBTN_B)
#define WBTN_MINUS_HELD (wii_btnsHeld & WBTN_MINUS)
#define WBTN_PLUS_HELD (wii_btnsHeld & WBTN_PLUS)
#define WBTN_HOME_HELD (wii_btnsHeld & WBTN_HOME)
#define WBTN_1_HELD (wii_btnsHeld & WBTN_1)
#define WBTN_2_HELD (wii_btnsHeld & WBTN_2)

#define BTN_UP_HELD ((wii_btnsHeld & WBTN_UP)||(gc_btnsHeld & BTN_UP)!=0)
#define BTN_RIGHT_HELD ((wii_btnsHeld & WBTN_RIGHT)||(gc_btnsHeld & BTN_RIGHT)!=0)
#define BTN_DOWN_HELD ((wii_btnsHeld & WBTN_DOWN)||(gc_btnsHeld & BTN_DOWN))
#define BTN_LEFT_HELD ((wii_btnsHeld & WBTN_LEFT)||(gc_btnsHeld & BTN_LEFT)!=0)
#define BTN_A_HELD ((wii_btnsHeld & WBTN_A)||(gc_btnsHeld & BTN_A)!=0)
#define BTN_B_HELD ((wii_btnsHeld & WBTN_B)||(gc_btnsHeld & BTN_B)!=0)
#define BTN_MINUS_HELD ((wii_btnsHeld & WBTN_MINUS)||(gc_btnsHeld & BTN_MINUS)!=0)
#define BTN_PLUS_HELD ((wii_btnsHeld & WBTN_PLUS)||(gc_btnsHeld & BTN_PLUS)!=0)
#define BTN_HOME_HELD ((wii_btnsHeld & WBTN_HOME)||(gc_btnsHeld & BTN_HOME)!=0)
#define BTN_1_HELD ((wii_btnsHeld & WBTN_1)||(gc_btnsHeld & BTN_1)!=0)
#define BTN_2_HELD ((wii_btnsHeld & WBTN_2)||(gc_btnsHeld & BTN_2)!=0)

#define BTN_UP_REPEAT ((wii_repeat & WBTN_UP)||(gc_repeat & BTN_UP)!=0)
#define BTN_RIGHT_REPEAT ((wii_repeat & WBTN_RIGHT)||(gc_repeat & BTN_RIGHT)!=0)
#define BTN_DOWN_REPEAT ((wii_repeat & WBTN_DOWN)||(gc_repeat & BTN_DOWN)!=0)
#define BTN_LEFT_REPEAT ((wii_repeat & WBTN_LEFT)||(gc_repeat & BTN_LEFT)!=0)
#define BTN_A_REPEAT ((wii_repeat & WBTN_A)||(gc_repeat & BTN_A)!=0)
#define BTN_B_REPEAT ((wii_repeat & WBTN_B)||(gc_repeat & BTN_B)!=0)
#define BTN_MINUS_REPEAT ((wii_repeat & WBTN_MINUS)||(gc_repeat & BTN_MINUS)!=0)
#define BTN_PLUS_REPEAT ((wii_repeat & WBTN_PLUS)||(gc_repeat & BTN_PLUS)!=0)
#define BTN_HOME_REPEAT ((wii_repeat & WBTN_HOME)||(gc_repeat & BTN_HOME)!=0)
#define BTN_1_REPEAT ((wii_repeat & WBTN_1)||(gc_repeat & BTN_1)!=0)
#define BTN_2_REPEAT ((wii_repeat & WBTN_2)||(gc_repeat & BTN_2)!=0)

#define LEFT_STICK_UP ((LEFT_STICK_ANG_UP && left_stick_mag[wmote] > 0.15) || PAD_StickY(wmote) > 20)
#define LEFT_STICK_RIGHT ((LEFT_STICK_ANG_RIGHT && left_stick_mag[wmote] > 0.15) || PAD_StickX(wmote) > 20)
#define LEFT_STICK_DOWN ((LEFT_STICK_ANG_DOWN && left_stick_mag[wmote] > 0.15) || PAD_StickY(wmote) < -20)
#define LEFT_STICK_LEFT ((LEFT_STICK_ANG_LEFT && left_stick_mag[wmote] > 0.15) || PAD_StickX(wmote) < -20)

#define LEFT_STICK_ANG_UP ((left_stick_angle[wmote] >= 300 && left_stick_angle[wmote] <= 360) \
		|| (left_stick_angle[wmote] >= 0 && left_stick_angle[wmote] <= 60))
#define LEFT_STICK_ANG_RIGHT (left_stick_angle[wmote] >= 30 && left_stick_angle[wmote] <= 150)
#define LEFT_STICK_ANG_DOWN (left_stick_angle[wmote] >= 120 && left_stick_angle[wmote] <= 240)
#define LEFT_STICK_ANG_LEFT (left_stick_angle[wmote] >= 210 && left_stick_angle[wmote] <= 330)

#define RIGHT_STICK_UP ((RIGHT_STICK_ANG_UP && right_stick_mag[wmote] > 0.15 && right_stick_skip[wmote] == 0) || PAD_SubStickY(wmote) > 20)
#define RIGHT_STICK_RIGHT ((RIGHT_STICK_ANG_RIGHT && right_stick_mag[wmote] > 0.15 && right_stick_skip[wmote] == 0) || PAD_SubStickX(wmote) > 20)
#define RIGHT_STICK_DOWN ((RIGHT_STICK_ANG_DOWN && right_stick_mag[wmote] > 0.15 && right_stick_skip[wmote] == 0) || PAD_SubStickY(wmote) < -20)
#define RIGHT_STICK_LEFT ((RIGHT_STICK_ANG_LEFT && right_stick_mag[wmote] > 0.15 && right_stick_skip[wmote] == 0) || PAD_SubStickX(wmote) < -20)

#define RIGHT_STICK_ANG_UP ((right_stick_angle[wmote] >= 300 && right_stick_angle[wmote] <= 360) \
		|| (right_stick_angle[wmote] >= 0 && right_stick_angle[wmote] <= 60))
#define RIGHT_STICK_ANG_RIGHT (right_stick_angle[wmote] >= 30 && right_stick_angle[wmote] <= 150)
#define RIGHT_STICK_ANG_DOWN (right_stick_angle[wmote] >= 120 && right_stick_angle[wmote] <= 240)
#define RIGHT_STICK_ANG_LEFT (right_stick_angle[wmote] >= 210 && right_stick_angle[wmote] <= 330)

#define WROLL_LEFT  (WBTN_B_HELD && (wmote_roll[wmote] < -5) && wmote_roll_skip[wmote] == 0)
#define WROLL_RIGHT (WBTN_B_HELD && (wmote_roll[wmote] > 5)  && wmote_roll_skip[wmote] == 0)

/*
//Button values reference//
WPAD_BUTTON_2							0x0001
PAD_BUTTON_LEFT							0x0001

WPAD_BUTTON_1							0x0002
PAD_BUTTON_RIGHT						0x0002

WPAD_BUTTON_B							0x0004
PAD_BUTTON_DOWN							0x0004

WPAD_BUTTON_A							0x0008
PAD_BUTTON_UP							0x0008

WPAD_BUTTON_MINUS						0x0010
PAD_TRIGGER_Z							0x0010

PAD_TRIGGER_R							0x0020

PAD_TRIGGER_L							0x0040

WPAD_BUTTON_HOME						0x0080

WPAD_BUTTON_LEFT						0x0100
PAD_BUTTON_A							0x0100

WPAD_BUTTON_RIGHT						0x0200
PAD_BUTTON_B							0x0200

WPAD_BUTTON_DOWN						0x0400
PAD_BUTTON_X							0x0400

WPAD_BUTTON_UP							0x0800
PAD_BUTTON_Y							0x0800

WPAD_BUTTON_PLUS						0x1000
PAD_BUTTON_MENU							0x1000
PAD_BUTTON_START						0x1000

WPAD_NUNCHUK_BUTTON_Z					(0x0001<<16)
WPAD_CLASSIC_BUTTON_UP					(0x0001<<16)
WPAD_GUITAR_HERO_3_BUTTON_STRUM_UP		(0x0001<<16)

WPAD_NUNCHUK_BUTTON_C					(0x0002<<16)
WPAD_CLASSIC_BUTTON_LEFT				(0x0002<<16)

WPAD_CLASSIC_BUTTON_ZR					(0x0004<<16)

WPAD_CLASSIC_BUTTON_X					(0x0008<<16)
WPAD_GUITAR_HERO_3_BUTTON_YELLOW		(0x0008<<16)

WPAD_CLASSIC_BUTTON_A					(0x0010<<16)
WPAD_GUITAR_HERO_3_BUTTON_GREEN			(0x0010<<16)

WPAD_CLASSIC_BUTTON_Y					(0x0020<<16)
WPAD_GUITAR_HERO_3_BUTTON_BLUE			(0x0020<<16)

WPAD_CLASSIC_BUTTON_B					(0x0040<<16)
WPAD_GUITAR_HERO_3_BUTTON_RED			(0x0040<<16)

WPAD_CLASSIC_BUTTON_ZL					(0x0080<<16)
WPAD_GUITAR_HERO_3_BUTTON_ORANGE		(0x0080<<16)

WPAD_CLASSIC_BUTTON_FULL_R				(0x0200<<16)

WPAD_CLASSIC_BUTTON_PLUS				(0x0400<<16)
WPAD_GUITAR_HERO_3_BUTTON_PLUS			(0x0400<<16)

WPAD_CLASSIC_BUTTON_HOME				(0x0800<<16)

WPAD_CLASSIC_BUTTON_MINUS				(0x1000<<16)
WPAD_GUITAR_HERO_3_BUTTON_MINUS			(0x1000<<16)

WPAD_CLASSIC_BUTTON_FULL_L				(0x2000<<16)

WPAD_CLASSIC_BUTTON_DOWN				(0x4000<<16)
WPAD_GUITAR_HERO_3_BUTTON_STRUM_DOWN	(0x4000<<16)

WPAD_CLASSIC_BUTTON_RIGHT				(0x8000<<16)
*/