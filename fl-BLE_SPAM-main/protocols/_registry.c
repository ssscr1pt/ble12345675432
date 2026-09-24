#include "_registry.h"

const BleSpamProtocol* ble_spam_protocols[] = {
    &protocol_continuity,
    &protocol_fastpair,
    &protocol_swiftpair,
};

const size_t ble_spam_protocols_count = COUNT_OF(ble_spam_protocols);
