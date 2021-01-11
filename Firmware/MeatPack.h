/*
* MeatPack G-Code Compression
*
* Algorithm & Implementation: Scott Mudge - mail@scottmudge.com
* Date: Dec. 2020
*
* Specifically optimized for 3D printing G-Code, this is a zero-cost data compression method
* which packs ~180-190% more data into the same amount of bytes going to the CNC controller.
* As a majority of G-Code can be represented by a restricted alphabet, I performed histogram
* analysis on a wide variety of 3D printing gcode samples, and found ~93% of all gcode could
* be represented by the same 15-character alphabet.
*
* This allowed me to design a system of packing 2 8-bit characters into a single byte, assuming
* they fall within this limited 15-character alphabet. Using a 4-bit lookup table, these 8-bit
* characters can be represented by a 4-bit index.
*
* Combined with some logic to allow commingling of full-width characters outside of this 15-
* character alphabet (at the cost of an extra 8-bits per full-width character), and by stripping
* out unnecessary comments, the end result is gcode which is roughly half the original size.
*
* Why did I do this? I noticed micro-stuttering and other data-bottleneck issues while printing
* objects with high curvature, especially at high speeds. There is also the issue of the limited
* baud rate provided by Prusa's Atmega2560-based boards, over the USB serial connection. So soft-
* ware like OctoPrint would also suffer this same micro-stuttering and poor print quality issue.
* 
*/
#include "Configuration.h"
#include <stdint.h>

#ifndef MEATPACK_H_
#define MEATPACK_H_

#ifdef ENABLE_MEATPACK

#define MeatPack_SecondNotPacked    0b11110000
#define MeatPack_FirstNotPacked     0b00001111

// These are commands sent to MeatPack to control its behavior.
// They are sent by first sending 2x MeatPack_CommandByte (0xFF) in sequence,
//      followed by one of the command bytes below.
// Provided that 0xFF is an exceedingly rare character that is virtually never
// present in g-code naturally, it is safe to assume 2 in sequence should never
// happen naturally, and so it is used as a signal here.
//
// 0xFF *IS* used in "packed" g-code (used to denote that the next 2 characters are
// full-width), however 2 in a row will never occur, as the next 2 bytes will always
// some non-0xFF character.
enum MeatPack_Command {
    MPC_None            = 0b00000000,
    MPC_TogglePacking   = 0b11111101,
    MPC_EnablePacking   = 0b11111011,
    MPC_DisablePacking  = 0b11111010,
    MPC_ResetState      = 0b11111001,
    MPC_QueryState      = 0b11111000
};

// Pass in a character rx'd by SD card or serial. Automatically parses command/ctrl sequences,
// and will control state internally.
// @param out [in] Output pointer for unpacked/processed data.
// @return Number of characters returned. Range from 0 to 2.
extern char mp_handle_rx_char(const uint8_t c, char* const __restrict out);

// Reset MeatPack state.
extern void mp_reset_state();

// Manually trigger command
extern void mp_trigger_cmd(const MeatPack_Command cmd);

#endif

#endif // MEATPACK_H_
