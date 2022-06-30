#ifndef HID_H
#define HID_H

#include "tusb.h"

typedef void(* kbdleds_callback_t) (uint8_t leds);
typedef void(* led_callback_t) (uint8_t state);

void hid_setup(void);
void hid_set_kbdleds_callback(kbdleds_callback_t callback);
void hid_clear_kbdleds_callback(void);
void hid_set_led_callback(led_callback_t callback);
void hid_clear_led_callback(void);
void hid_loop(void);

void send_hid_keyboard_report(uint8_t buf[6], uint8_t modifier);
void send_empty_hid_keyboard_report(void);

void send_hid_mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t vertical, int8_t horizontal);

void send_hid_cc_report(uint16_t control_addr);
void send_empty_hid_cc_report(void);

#endif
