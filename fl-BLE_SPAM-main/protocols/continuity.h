#pragma once

#include "_base.h"

typedef enum {
    ContinuityTypeProximityPair,
    ContinuityTypeAppleAction,
    ContinuityTypeAppleCustom, // Добавлено: идентификатор нашей кастомной атаки
} ContinuityType;

extern const BleSpamProtocol protocol_continuity;
