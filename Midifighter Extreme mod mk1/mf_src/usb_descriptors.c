// Describe the Midifighter device to the host USB system.
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
// 2009-05-15: formatted and edited by Robin Green
// 2009-11-23: updated to LUFA 20090924

#include "usb_descriptors.h"

// Device descriptor structure. This descriptor, located in FLASH memory,
// describes the overall device characteristics, including the supported USB
// version, control endpoint size and the number of device
// configurations. The descriptor is read out by the USB host when the
// enumeration process begins.
//
// The Vendor ID and Product ID are how the host knows which divers to
// assign to this device. VID and PID pairs must be registered with the
// USB-IF (www.usb.org) if you want to be able to submit your device for
// testing so it can be labeled as "USB Certified". These particular values
// belong to Atmel, and are kindly donated to products like ours that use
// Atmel parts. For more on this see this FAQ:
//
//    http://support.atmel.no/bin/customer?=&action=viewKbEntry&id=220
//
USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
    .Header                 = { .Size = sizeof(USB_Descriptor_Device_t),
                                .Type = DTYPE_Device },
    .USBSpecification       = VERSION_BCD(01.10),
    .Class                  = 0x00,
    .SubClass               = 0x00,
    .Protocol               = 0x00,
    .Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,
    .VendorID               = 0x03EB,  // these belong to Atmel, do not abuse.
    .ProductID              = 0x2048,  // these belong to Atmel, do not abuse.
    .ReleaseNumber          = VERSION_BCD(00.02),
    .ManufacturerStrIndex   = 0x01,
    .ProductStrIndex        = 0x02,
    .SerialNumStrIndex      = NO_DESCRIPTOR,
    .NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

// Configuration descriptor structure. This descriptor, located in FLASH
// memory, describes the usage of the device in one of its supported
// configurations, including information about any device interfaces and
// endpoints. The descriptor is read out by the USB host during the
// enumeration process when selecting a configuration so that the host may
// correctly communicate with the USB device.
//

USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor = {
    .Config = {
        .Header                   = { .Size = sizeof(USB_Descriptor_Configuration_Header_t),
                                      .Type = DTYPE_Configuration },
        .TotalConfigurationSize   = sizeof(USB_Descriptor_Configuration_t),
        .TotalInterfaces          = 2,
        .ConfigurationNumber      = 1,
        .ConfigurationStrIndex    = NO_DESCRIPTOR,
        .ConfigAttributes         = (USB_CONFIG_ATTR_BUSPOWERED | USB_CONFIG_ATTR_SELFPOWERED),
        .MaxPowerConsumption      = USB_CONFIG_POWER_MA(100)
    },

    .Audio_ControlInterface = {
        .Header                   = { .Size = sizeof(USB_Descriptor_Interface_t),
                                      .Type = DTYPE_Interface },
        .InterfaceNumber          = 0,
        .AlternateSetting         = 0,
        .TotalEndpoints           = 0,
        .Class                    = 0x01,
        .SubClass                 = 0x01,
        .Protocol                 = 0x00,
        .InterfaceStrIndex        = NO_DESCRIPTOR
    },

    .Audio_ControlInterface_SPC = {
        .Header                   = { .Size = sizeof(USB_Audio_Descriptor_Interface_AC_t),
                                      .Type = DTYPE_CSInterface },
        .Subtype                  = AUDIO_DSUBTYPE_CSInterface_Header,
        .ACSpecification          = VERSION_BCD(01.00),
        .TotalLength              = sizeof(USB_Audio_Descriptor_Interface_AC_t),
        .InCollection             = 1,
        .InterfaceNumber          = 1,
    },

    .Audio_StreamInterface = {
        .Header                   = { .Size = sizeof(USB_Descriptor_Interface_t),
                                      .Type = DTYPE_Interface },
        .InterfaceNumber          = 1,
        .AlternateSetting         = 0,
        .TotalEndpoints           = 2,
        .Class                    = 0x01,
        .SubClass                 = 0x03,
        .Protocol                 = 0x00,
        .InterfaceStrIndex        = NO_DESCRIPTOR
    },

    .Audio_StreamInterface_SPC = {
        .Header                   = { .Size = sizeof(USB_MIDI_Descriptor_AudioInterface_AS_t),
                                      .Type = DTYPE_CSInterface },
        .Subtype                  = AUDIO_DSUBTYPE_CSInterface_General,
        .AudioSpecification       = VERSION_BCD(01.00),
        .TotalLength              = (sizeof(USB_Descriptor_Configuration_t) -
                                     offsetof(USB_Descriptor_Configuration_t, Audio_StreamInterface_SPC))
    },

    .MIDI_In_Jack_Emb = {
        .Header                   = { .Size = sizeof(USB_MIDI_Descriptor_InputJack_t),
                                      .Type = DTYPE_CSInterface },
        .Subtype                  = AUDIO_DSUBTYPE_CSInterface_InputTerminal,
        .JackType                 = MIDI_JACKTYPE_Embedded,
        .JackID                   = 0x01,
        .JackStrIndex             = NO_DESCRIPTOR
    },

    .MIDI_In_Jack_Ext = {
        .Header                   = { .Size = sizeof(USB_MIDI_Descriptor_InputJack_t),
                                      .Type = DTYPE_CSInterface },
        .Subtype                  = AUDIO_DSUBTYPE_CSInterface_InputTerminal,
        .JackType                 = MIDI_JACKTYPE_External,
        .JackID                   = 0x02,
        .JackStrIndex             = NO_DESCRIPTOR
    },

    .MIDI_Out_Jack_Emb = {
        .Header                   = { .Size = sizeof(USB_MIDI_Descriptor_OutputJack_t),
                                      .Type = DTYPE_CSInterface },
        .Subtype                  = AUDIO_DSUBTYPE_CSInterface_OutputTerminal,
        .JackType                 = MIDI_JACKTYPE_Embedded,
        .JackID                   = 0x03,
        .NumberOfPins             = 1,
        .SourceJackID             = {0x02},
        .SourcePinID              = {0x01},
        .JackStrIndex             = NO_DESCRIPTOR
    },

    .MIDI_Out_Jack_Ext = {
        .Header                   = { .Size = sizeof(USB_MIDI_Descriptor_OutputJack_t),
                                      .Type = DTYPE_CSInterface },
        .Subtype                  = AUDIO_DSUBTYPE_CSInterface_OutputTerminal,
        .JackType                 = MIDI_JACKTYPE_External,
        .JackID                   = 0x04,
        .NumberOfPins             = 1,
        .SourceJackID             = {0x01},
        .SourcePinID              = {0x01},
        .JackStrIndex             = NO_DESCRIPTOR
    },

    .MIDI_In_Jack_Endpoint = {
        .Endpoint = {
            .Header              = { .Size = sizeof(USB_Audio_Descriptor_StreamEndpoint_Std_t),
                                     .Type = DTYPE_Endpoint },
            .EndpointAddress     = (ENDPOINT_DESCRIPTOR_DIR_OUT | MIDI_STREAM_OUT_EPNUM),
            .Attributes          = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
            .EndpointSize        = MIDI_STREAM_EPSIZE,
            .PollingIntervalMS   = 0
        },
        .Refresh                  = 0,
        .SyncEndpointNumber       = 0
        },

    .MIDI_In_Jack_Endpoint_SPC = {
        .Header                   = { .Size = sizeof(USB_MIDI_Descriptor_Jack_Endpoint_t),
                                      .Type = DTYPE_CSEndpoint },
        .Subtype                  = AUDIO_DSUBTYPE_CSEndpoint_General,
        .TotalEmbeddedJacks       = 0x01,
        .AssociatedJackID         = {0x01}
    },

    .MIDI_Out_Jack_Endpoint = {
        .Endpoint = {
            .Header              = { .Size = sizeof(USB_Audio_Descriptor_StreamEndpoint_Std_t),
                                     .Type = DTYPE_Endpoint },
            .EndpointAddress     = (ENDPOINT_DESCRIPTOR_DIR_IN | MIDI_STREAM_IN_EPNUM),
            .Attributes          = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
            .EndpointSize        = MIDI_STREAM_EPSIZE,
            .PollingIntervalMS   = 0
        },
        .Refresh                  = 0,
        .SyncEndpointNumber       = 0
    },

    .MIDI_Out_Jack_Endpoint_SPC = {
        .Header                   = { .Size = sizeof(USB_MIDI_Descriptor_Jack_Endpoint_t),
                                      .Type = DTYPE_CSEndpoint },
        .Subtype                  = AUDIO_DSUBTYPE_CSEndpoint_General,
        .TotalEmbeddedJacks       = 0x01,
        .AssociatedJackID         = {0x03}
    }
};

// Language descriptor structure. This descriptor, located in FLASH memory,
// is returned when the host requests the string descriptor with index 0
// (the first index). It is actually an array of 16-bit integers, which
// indicate via the language ID table available at USB.org what languages
// the device supports for its string descriptors.
//
USB_Descriptor_String_t PROGMEM LanguageString =
{
    .Header                 = { .Size = USB_STRING_LEN(1),
                                .Type = DTYPE_String },
    .UnicodeString          = {LANGUAGE_ID_ENG}
};

// Manufacturer descriptor string. This is a Unicode string containing the
// manufacturer's details in human readable form, and is read out upon
// request by the host when the appropriate string ID is requested, listed
// in the Device Descriptor.
//
USB_Descriptor_String_t PROGMEM ManufacturerString =
{
    .Header                 = { .Size = USB_STRING_LEN(37),
                                .Type = DTYPE_String },
    .UnicodeString          = L"www.djtechtools.com powered by Atmel®"
};

// Product descriptor string. This is a Unicode string containing the
// product's details in human readable form, and is read out upon request by
// the host when the appropriate string ID is requested, listed in the
// Device Descriptor.
//
USB_Descriptor_String_t PROGMEM ProductString =
{
    .Header                 = { .Size = USB_STRING_LEN(11),
                                .Type = DTYPE_String },
    .UnicodeString          = L"Midifighter"
};

// This function is called by the library when in device mode, and must be
// overridden (see library "USB Descriptors" documentation) by the
// application code so that the address and size of a requested descriptor
// can be given to the USB library. When the device receives a Get
// Descriptor request on the control endpoint, this function is called so
// that the descriptor details can be passed back and the appropriate
// descriptor sent back to the USB host.
//
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    const void** const DescriptorAddress)
{
    const uint8_t  DescriptorType   = (wValue >> 8);
    const uint8_t  DescriptorNumber = (wValue & 0xFF);
    const void* Address = NULL;
    uint16_t    Size    = NO_DESCRIPTOR;

    switch (DescriptorType) {
    case DTYPE_Device:
        Address = &DeviceDescriptor;
        Size    = sizeof(USB_Descriptor_Device_t);
        break;
    case DTYPE_Configuration:
        Address = &ConfigurationDescriptor;
        Size    = sizeof(USB_Descriptor_Configuration_t);
        break;
    case DTYPE_String:
        switch (DescriptorNumber) {
        case 0x00:
            Address = &LanguageString;
            Size    = pgm_read_byte(&LanguageString.Header.Size);
            break;
        case 0x01:
            Address = &ManufacturerString;
            Size    = pgm_read_byte(&ManufacturerString.Header.Size);
            break;
        case 0x02:
            Address = &ProductString;
            Size    = pgm_read_byte(&ProductString.Header.Size);
            break;
        }
        break;
    }
    *DescriptorAddress = Address;
    return Size;
}
