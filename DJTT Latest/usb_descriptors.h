// USB descriptors for Midifighter
//
//   Copyright 2009  Dean Camera (dean [at] fourwalledcubicle [dot] com)
//
//   Permission to use, copy, modify, and distribute this software
//   and its documentation for any purpose and without fee is hereby
//   granted, provided that the above copyright notice appear in all
//   copies and that both that the copyright notice and this
//   permission notice and warranty disclaimer appear in supporting
//   documentation, and that the name of the author not be used in
//   advertising or publicity pertaining to distribution of the
//   software without specific, written prior permission.
//
//   The author disclaim all warranties with regard to this
//   software, including all implied warranties of merchantability
//   and fitness.  In no event shall the author be liable for any
//   special, indirect or consequential damages or any damages
//   whatsoever resulting from loss of use, data or profits, whether
//   in an action of contract, negligence or other tortious action,
//   arising out of or in connection with the use or performance of
//   this software.
//
// 2009-05-15: edited by Robin Green
// 2009-11-24: Updated to LUFA20090924

#ifndef _USB_DESCRIPTORS_H_INCLUDED
#define _USB_DESCRIPTORS_H_INCLUDED

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/USB/Class/MIDI.h>

#include <avr/pgmspace.h>


// USB Constants --------------------------------------------------------------

// The endpoint number for USB OUT messages (out from the host, into the
// Midifighter)
#define MIDI_STREAM_OUT_EPNUM 1

// The endpoint number for USB IN messages (in to the host, out from the
// Midifighter)
#define MIDI_STREAM_IN_EPNUM 2

// The size of a USB endpoint, 64 bytes, which will fit 16 normal USB-MIDI
// packets.
#define MIDI_STREAM_EPSIZE 64


// USB Descriptor -------------------------------------------------------------

// Type for the device configuration descriptor structure.
//
// This must be defined in the application code as this configuration
// contains several sub-descriptors which are unique to this device and are
// needed to describe the device to the host.
//
typedef struct {
    USB_Descriptor_Configuration_Header_t     Config;
    USB_Descriptor_Interface_t                Audio_ControlInterface;
    USB_Audio_Descriptor_Interface_AC_t       Audio_ControlInterface_SPC;
    USB_Descriptor_Interface_t                Audio_StreamInterface;
    USB_MIDI_Descriptor_AudioInterface_AS_t   Audio_StreamInterface_SPC;
    USB_MIDI_Descriptor_InputJack_t           MIDI_In_Jack_Emb;
    USB_MIDI_Descriptor_InputJack_t           MIDI_In_Jack_Ext;
    USB_MIDI_Descriptor_OutputJack_t          MIDI_Out_Jack_Emb;
    USB_MIDI_Descriptor_OutputJack_t          MIDI_Out_Jack_Ext;
    USB_Audio_Descriptor_StreamEndpoint_Std_t MIDI_In_Jack_Endpoint;
    USB_MIDI_Descriptor_Jack_Endpoint_t       MIDI_In_Jack_Endpoint_SPC;
    USB_Audio_Descriptor_StreamEndpoint_Std_t MIDI_Out_Jack_Endpoint;
    USB_MIDI_Descriptor_Jack_Endpoint_t       MIDI_Out_Jack_Endpoint_SPC;
} USB_Descriptor_Configuration_t;


// This function, required by LUFA, has a return value that may be
// unused. The macro at the end of this declaration is telling the compiler
// not to freak out about this.
//
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress)
        ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);


#endif  // _USB_DESCRIPTORS_H_INCLUDED
