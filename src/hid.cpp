#include "hid.hpp"

#include "usb_descriptors.h"

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

/**
 * Callbacks
 */

static kbdleds_callback_t _kbdleds_cb = NULL;
static led_callback_t _led_cb = NULL;

// Invoked when device is mounted
void tud_mount_cb(void) {
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len) {
    // TODO: Not implemented
    (void) instance;
    (void) len;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    // TODO: Not Implemented
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT && report_id == REPORT_ID_KEYBOARD) {
        // Set keyboard LED e.g Capslock, Numlock etc...
        // bufsize should be (at least) 1
        if ( bufsize < 1 ) return;

        uint8_t const kbd_leds = buffer[0];
        if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
            // Capslock On: disable blink, turn led on
            blink_interval_ms = 0;
            if (_led_cb) _led_cb(1);
        } else {
            // Caplocks Off: back to normal blink
            if (_led_cb) _led_cb(0);
            blink_interval_ms = BLINK_MOUNTED;
        }
        if (_kbdleds_cb) _kbdleds_cb(kbd_leds);
    }
}

void hid_setup(void) {
    tusb_init();
}

void hid_set_kbdleds_callback(kbdleds_callback_t callback) {
    _kbdleds_cb = callback;
}
void hid_clear_kbdleds_callback(void) {
    _kbdleds_cb = NULL;
}

void hid_set_led_callback(led_callback_t callback) {
    _led_cb = callback;
}
void hid_clear_led_callback(led_callback_t callback) {
    _led_cb = NULL;
}

void hid_loop(void) {
    tud_task();

    static absolute_time_t start_timestamp = get_absolute_time();
    static absolute_time_t now_timestamp = nil_time;

    if (blink_interval_ms) {
        // Blink every interval ms
        now_timestamp = get_absolute_time();
        if (absolute_time_diff_us(start_timestamp, now_timestamp) / 1000 >= blink_interval_ms) {
            start_timestamp = now_timestamp;
            if (_led_cb) _led_cb(2); // toggle
        }
    }

}

/**
 * HID Reports
 */

void send_hid_keyboard_report(uint8_t buf[6], uint8_t modifier = 0) {
    if (!tud_hid_ready()) return;
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, buf);
}
void send_empty_hid_keyboard_report(void) {
    send_hid_keyboard_report(NULL);
}

void send_hid_mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t vertical = 0, int8_t horizontal = 0) {
    if (!tud_hid_ready()) return;
    tud_hid_mouse_report(REPORT_ID_MOUSE, buttons, dx, dy, vertical, horizontal);
}

void send_hid_cc_report(uint16_t control_addr) {
    if (!tud_hid_ready()) return;
    tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &control_addr, 2);
}
void send_empty_hid_cc_report(void) {
    send_hid_cc_report(0);
}
