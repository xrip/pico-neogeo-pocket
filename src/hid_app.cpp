/*
 * USB HID keyboard for the NGPC emulator (TinyUSB host).
 * Based on the TinyUSB host HID example (MIT License,
 * Copyright (c) 2021, Ha Thach (tinyusb.org)): only boot-protocol
 * keyboards are handled; reports go to the same process_kbd_report()
 * that the PS/2 keyboard driver uses.
 */

#include "tusb.h"

void process_kbd_report(hid_keyboard_report_t const* report,
                        hid_keyboard_report_t const* prev_report);

static hid_keyboard_report_t prev_report = { 0, 0, { 0 } };

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance,
                      uint8_t const* desc_report, uint16_t desc_len) {
    (void)desc_report;
    (void)desc_len;
    if (tuh_hid_interface_protocol(dev_addr, instance) == HID_ITF_PROTOCOL_KEYBOARD)
        tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    (void)dev_addr;
    (void)instance;
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,
                                uint8_t const* report, uint16_t len) {
    if (tuh_hid_interface_protocol(dev_addr, instance) == HID_ITF_PROTOCOL_KEYBOARD &&
        len >= sizeof(hid_keyboard_report_t)) {
        process_kbd_report((hid_keyboard_report_t const*)report, &prev_report);
        prev_report = *(hid_keyboard_report_t const*)report;
    }
    tuh_hid_receive_report(dev_addr, instance);
}
