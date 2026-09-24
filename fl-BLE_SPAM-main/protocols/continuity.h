#pragma once

#include "_base.h"

typedef enum {
    ContinuityTypeProximityPair,
    ContinuityTypeAppleAction,
    ContinuityTypeAppleCustom,
} ContinuityType;

extern const BleSpamProtocol protocol_continuity;
