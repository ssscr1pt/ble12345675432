#pragma once

#include "_base.h"

typedef enum {
    ContinuityTypeProximityPair,
    ContinuityTypeAppleAction,
    ContinuityTypeAppleCustom,
} ContinuityType;

extern const BleSpamProtocol ble_spam_protocol_continuity;
