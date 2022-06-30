/**
 * PicoADB
 * 2022 D Cooper Dalrymple - me@dcdalrymple.com
 * GPL v3 License
 *
 * Package: picoadb
 * File: main.cpp
 * Title: Main
 * Version: 0.1.0
 * Since: 0.1.0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#define LED PICO_DEFAULT_LED_PIN
#define UART uart0
#define UART_TX PICO_DEFAULT_UART_TX_PIN
#define UART_RX PICO_DEFAULT_UART_RX_PIN

#include "adb.hpp" // Must define hardware settings before
#include "keymap.h"
#include "hid.hpp"

/**
 * ADB to USB
 * ==========
 */

#define DEBUG
#define LOCKING_CAPS // comment this out if not using a keyboard with locking Caps Lock

static bool has_media_keys = false;
static bool is_iso_layout = false;
uint8_t hidmod = 0; // modifiers
uint8_t hidbuf[6] = { 0 }; // key buffer
#ifdef LOCKING_CAPS
bool capsOn = false;
#endif

inline static void register_key(uint8_t key) {
    uint8_t tmp[6];
    if (key & 0x80) {
        //matrix[row] &= ~(1<<col);
        switch (key & 0x7F) {
            case 0x36: // LCTRL
                hidmod &= ~(1<<0);
                break;
            case 0x37: // LGUI
                hidmod &= ~(1<<3);
                break;
            case 0x38: // LSHIFT
                hidmod &= ~(1<<1);
                break;
            case 0x3A: // LALT
                hidmod &= ~(1<<2);
                break;
            case 0x7B: // RSHIFT
                hidmod &= ~(1<<5);
                break;
            case 0x7C: // RALT
                hidmod &= ~(1<<6);
                break;
            case 0x7D: // RCTRL
                hidmod &= ~(1<<4);
                break;
            case 0x7E: // RGUI
                hidmod &= ~(1<<7);
                break;
#ifdef LOCKING_CAPS
            case 57: // CAPS (locking)
                if (!capsOn) return;
                capsOn = false;
                for (uint i = 0; i < 6; i++) {
                    if (hidbuf[i] == 0) {
                        hidbuf[i] = 57;
                        break;
                    } else if (hidbuf[i] == 57) {
                        break;
                    }
                }
                send_hid_keyboard_report(hidbuf, hidmod);
#endif
            default:
                for (int i = 0; i < 6; i++) tmp[i] = 0;
                for (int i = 0; i < 6; i++) {
                    if (hidbuf[i] != adb_to_usb[key & 0x7F]) {
                        tmp[i] = hidbuf[i];
                    }
                }
                for (int i = 0; i < 6; i++) hidbuf[i] = tmp[i];
                break;
        }
    } else {
        //matrix[row] |=  (1<<col);
#ifdef LOCKING_CAPS
        if (key == 57 && capsOn) return;
#endif
        switch (key) {
            case 0x36: // LCTRL
                hidmod |= (1<<0);
                break;
            case 0x37: // LGUI
                hidmod |= (1<<3);
                break;
            case 0x38: // LSHIFT
                hidmod |= (1<<1);
                break;
            case 0x3A: // LALT
                hidmod |= (1<<2);
                break;
            case 0x7B: // RSHIFT
                hidmod |= (1<<5);
                break;
            case 0x7C: // RALT
                hidmod |= (1<<6);
                break;
            case 0x7D: // RCTRL
                hidmod |= (1<<4);
                break;
            case 0x7E: // RGUI
                hidmod |= (1<<7);
                break;
            default:
                for (int i = 0; i < 6; i++) {
                    if (hidbuf[i] == 0) {
                        hidbuf[i] = adb_to_usb[key];
                        break;
                    } else if (hidbuf[i] == adb_to_usb[key]) {
                        break;
                    }
                }
        }
#ifdef LOCKING_CAPS
        if (key == 57) {
            capsOn = true;
            send_hid_keyboard_report(hidbuf, hidmod);
            for (int i = 0; i < 6; i++) tmp[i] = 0;
            for (int i = 0; i < 6; i++) {
                if (hidbuf[i] != 57) {
                    tmp[i] = hidbuf[i];
                }
            }
            for (int i = 0; i < 6; i++) hidbuf[i] = tmp[i];
        }
#endif
    }

    send_hid_keyboard_report(hidbuf, hidmod);
}

void led_set(uint8_t usb_led) {
    adb_host_kbd_led(ADB_ADDR_KEYBOARD, ~usb_led);
}

static void device_scan(void) {
    //xprintf("\nScan:\n");
    for (uint8_t addr = 0; addr < 16; addr++) {
        uint16_t reg3 = adb_host_talk(addr, ADB_REG_3);
        if (reg3) {
            //xprintf(" addr:%d, reg3:%04X\n", addr, reg3);
        }
    }
}

void usb_kbdleds(uint8_t leds) {
    adb_host_kbd_led(ADB_ADDR_KEYBOARD, leds);
}
void usb_led(uint8_t state) {
    if (state & 0x02) {
        gpio_xor_mask(1 << LED); // toggle
    } else {
        gpio_put(LED, state);
    }
}

void adb_setup() {
#ifdef DEBUG
    // UART Debugging
    uart_init(UART, 9600);
    gpio_set_function(UART_TX, GPIO_FUNC_UART);
    gpio_set_function(UART_RX, GPIO_FUNC_UART);
    uart_puts(UART, "start\n");
#endif

    // LED
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_put(LED, 1);

    // ADB
    adb_host_init();
    sleep_ms(1000);
    device_scan();
#ifdef DEBUG
    uart_puts(UART, "\nKeyboard:\n");
#endif
    // Determine ISO keyboard by handler id
    // http://lxr.free-electrons.com/source/drivers/macintosh/adbhid.c?v=4.4#L815
    uint8_t handler_id = (uint8_t) adb_host_talk(ADB_ADDR_KEYBOARD, ADB_REG_3);
    switch (handler_id) {
    case 0x04: case 0x05: case 0x07: case 0x09: case 0x0D:
    case 0x11: case 0x14: case 0x19: case 0x1D: case 0xC1:
    case 0xC4: case 0xC7:
        is_iso_layout = true;
        break;
    default:
        is_iso_layout = false;
        break;
    }
#ifdef DEBUG
    uart_puts(UART, "handler: ");
    char *hexstr;
    sprintf(hexstr, "%02X", handler_id);
    uart_puts(UART, hexstr); // Hex?
    uart_puts(UART, ", ISO: ");
    uart_puts(UART, is_iso_layout ? "yes" : "no");
    uart_putc(UART, '\n');
#endif
    has_media_keys = (0x02 == (adb_host_talk(ADB_ADDR_APPLIANCE, ADB_REG_3) & 0xff));
#ifdef DEBUG
    if (has_media_keys) {
        uart_puts(UART, "Media keys\n");
    }
#endif
    // Enable keyboard left/right modifier distinction
    // Listen Register3
    //  upper byte: reserved bits 0000, keyboard address 0010
    //  lower byte: device handler 00000011
    adb_host_listen(ADB_ADDR_KEYBOARD, ADB_REG_3, ADB_ADDR_KEYBOARD, ADB_HANDLER_EXTENDED_KEYBOARD);

    gpio_put(LED, 0);
    device_scan();

    hid_set_kbdleds_callback(&usb_kbdleds);
    hid_set_led_callback(&usb_led);
}

void send_media_code(uint8_t code) {
    uint16_t control = 0;

    switch (code & 0x7f ) {
        case 0x00:  // Mic
            // TODO: Not sure which code to use
            break;
        case 0x01:  // Mute
            control = HID_USAGE_CONSUMER_MUTE;
            break;
        case 0x02:  // Volume down
            control = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
            break;
        case 0x03:  // Volume Up
            control = HID_USAGE_CONSUMER_VOLUME_INCREMENT;
            break;
        case 0x7F:  // no code
            break;
        default:
            //xprintf("ERROR: media key1\n");
            return;
    }

    if (control) send_hid_cc_report(control);
}

void adb_loop() {
    /* extra_key is volatile and more convoluted than necessary because gcc refused
    to generate valid code otherwise. Making extra_key uint8_t and constructing codes
    here via codes = extra_key<<8 | 0xFF; would consistently fail to even LOAD
    extra_key from memory, and leave garbage in the high byte of codes. I tried
    dozens of code variations and it kept generating broken assembly output. So
    beware if attempting to make extra_key code more logical and efficient. */
    static volatile uint16_t extra_key = 0xFFFF;
    uint16_t codes;
    uint8_t key0, key1;

    /* tick of last polling */
    static uint16_t tick_ms;

    codes = extra_key;
    extra_key = 0xFFFF;

    if (codes == 0xFFFF) {
        // polling with 12ms interval
        //if (timer_elapsed(tick_ms) < 12) return 0;
        //tick_ms = timer_read();

        codes = adb_host_kbd_recv(ADB_ADDR_KEYBOARD);

        // Adjustable keyboard media keys
        if (codes == 0 && has_media_keys && (codes = adb_host_kbd_recv(ADB_ADDR_APPLIANCE))) {
            send_media_code(codes & 0x7f); // key1
            send_media_code((codes >> 8) & 0x7f); // key0
        }
    }

    key0 = codes>>8;
    key1 = codes&0xFF;

    //if (debug_matrix && codes) {
    //print("adb_host_kbd_recv: "); phex16(codes); print("\n");
    //}

    if (codes == 0) {                           // no keys
        return;
    } else if (codes == 0x7F7F) {   // power key press
        send_media_code(HID_USAGE_CONSUMER_POWER);
    } else if (codes == 0xFFFF) {   // power key release
        //register_key(0xFF);
    } else {
        // Macally keyboard sends keys inversely against ADB protocol
        // https://deskthority.net/workshop-f7/macally-mk96-t20116.html
        if (key0 == 0xFF) {
            key0 = key1;
            key1 = 0xFF;
        }

        /* Swap codes for ISO keyboard
         * https://github.com/tmk/tmk_keyboard/issues/35
         *
         * ANSI
         * ,-----------    ----------.
         * | *a|  1|  2     =|Backspa|
         * |-----------    ----------|
         * |Tab  |  Q|     |  ]|   *c|
         * |-----------    ----------|
         * |CapsLo|  A|    '|Return  |
         * |-----------    ----------|
         * |Shift   |      Shift     |
         * `-----------    ----------'
         *
         * ISO
         * ,-----------    ----------.
         * | *a|  1|  2     =|Backspa|
         * |-----------    ----------|
         * |Tab  |  Q|     |  ]|Retur|
         * |-----------    -----`    |
         * |CapsLo|  A|    '| *c|    |
         * |-----------    ----------|
         * |Shif| *b|      Shift     |
         * `-----------    ----------'
         *
         *         ADB scan code   USB usage
         *         -------------   ---------
         * Key     ANSI    ISO     ANSI    ISO
         * ---------------------------------------------
         * *a      0x32    0x0A    0x35    0x35
         * *b      ----    0x32    ----    0x64
         * *c      0x2A    0x2A    0x31    0x31(or 0x32)
         */
        if (is_iso_layout) {
            if ((key0 & 0x7F) == 0x32) {
                key0 = (key0 & 0x80) | 0x0A;
            } else if ((key0 & 0x7F) == 0x0A) {
                key0 = (key0 & 0x80) | 0x32;
            }
        }
        register_key(key0);
        if (key1 != 0xFF) {             // key1 is 0xFF when no second key.
            extra_key = key1<<8 | 0xFF; // process in a separate call
        }
    }
    sleep_ms(12);
}

/**
 * Main Loop
 * =========
 */

int main() {
    stdio_init_all();

    bi_decl(bi_program_description("PicoADB"));
    bi_decl(bi_1pin_with_name(LED, "On-board LED"));
    bi_decl(bi_2pins_with_names(UART_RX, "UART Rx", UART_TX, "UART Tx"));
    bi_decl_if_func_used(bi_2pins_with_func(PICO_DEFAULT_UART_RX_PIN, PICO_DEFAULT_UART_TX_PIN, GPIO_FUNC_UART));
    bi_decl(bi_1pin_with_name(ADB_DATA, "ADB Data Pin"));
#ifdef ADB_PSW
    bi_decl(bi_1pin_with_name(ADB_PSW, "ADB Power Switch Pin"));
#endif

    hid_setup();
    adb_setup();
    while (1) {
        hid_loop();
        adb_loop();
    }

    return 0;
}
