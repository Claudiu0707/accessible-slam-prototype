#include <Arduino.h>

#define PACKAGE_HEADER_01 0xAA
#define PACKAGE_HEADER_02 0x55

#define INVALID_PACKAGE 0x00
#define VALID_PACKAGE 0x01


uint16_t check_package_valid();
void print_package();
void construct_package(uint8_t data);
void process_data();
void reset_data();


unsigned int data_package_index = 0;
typedef enum {NO_PACKAGE, POTENTIAL_PACKAGE, PACKAGE, POTENTIAL_PACKAGE_RESET} package_state;
package_state state = NO_PACKAGE, special_state = NO_PACKAGE;
uint8_t reset_package = 0;

uint8_t cat = 0;
uint8_t lsn = 0;
uint16_t catlsn = 0;

uint16_t fsa = 0, lsa = 0;
uint16_t cs = 0;

uint8_t data;
uint8_t buffered_data;

uint8_t  si_nibble_index = 0; // 0 => lower nibble | 1 = upper nibble
uint16_t si_index = 0;
uint16_t* si = nullptr;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1);
}

void loop() {
  process_data();
}

void process_data() {
  while (Serial1.available()) {
    data = Serial1.read();

    if (state == NO_PACKAGE) {
      if ((PACKAGE_HEADER_01 ^ data) == 0) {
        state = POTENTIAL_PACKAGE;
      }
    } else if (state == POTENTIAL_PACKAGE) {
      if ((PACKAGE_HEADER_02 ^ data) == 0) {
        state = PACKAGE;
      } else {
        state = NO_PACKAGE;
      }
    } else if (state == PACKAGE) {
      if ((PACKAGE_HEADER_01 ^ data) == 0 && special_state == NO_PACKAGE) {
        special_state = POTENTIAL_PACKAGE_RESET;
        buffered_data = data;
      } 
      else if (special_state == POTENTIAL_PACKAGE_RESET) {
        if ((PACKAGE_HEADER_02 ^ data) == 0) {
          state = PACKAGE;
          special_state = NO_PACKAGE;
          if (check_package_valid() == VALID_PACKAGE) {
            print_package();
          } 
          reset_data();
        } else {
          if ((PACKAGE_HEADER_01 ^ data) == 0 ) {
            construct_package(buffered_data);
          } else {
            special_state = NO_PACKAGE;
            construct_package(buffered_data);
            construct_package(data);
          }
        }
      } else {
        construct_package(data);
      }
    }
  }
}

void construct_package(uint8_t data) {
  switch (data_package_index) {
    case 0: // package type 
      cat = data;
      catlsn = data;
      data_package_index++;
      break;
    case 1: // sample count
      lsn = data;
      catlsn += (data << 8);
      data_package_index++;
      break;
    case 2: // first sample angle LSB
      fsa += data;
      data_package_index++;
      break;
    case 3: // first sample angle MSB
      fsa += (data << 8); 
      data_package_index++;
      break;
    case 4: // last sample angle LSB
      lsa += data;
      data_package_index++;
      break;
    case 5: // last sample angle MSB
      lsa += (data << 8); 
      data_package_index++;
      break;
    case 6: // checksum LSB
      cs += data;
      data_package_index++;
      break;
    case 7: // checksum MSB
      cs += (data << 8); 
      data_package_index++;
      break;
    default:
      if (si == nullptr) {
        si = (uint16_t *) calloc(lsn, sizeof(uint16_t));
        if (si == nullptr) {
          exit(1);
        }
      }
      if (si_nibble_index == 0) {
          si[si_index] += data; 
          si_nibble_index = 1;
        } else {
          si[si_index] += (data << 8);
          si[si_index] = (si[si_index] >> 2);
          si_nibble_index = 0;
          si_index++;
        }
      break;
  }
}

uint16_t check_package_valid() {
  uint16_t intermediary_cs = 0x55AA ^ catlsn ^ fsa ^ lsa;
  for (uint16_t i = 0; i < lsn; i++) {
    intermediary_cs ^= si[i];
  }
  return ((intermediary_cs ^ cs) == 0) ? VALID_PACKAGE : INVALID_PACKAGE; 
}

void reset_data() {
  if (si != nullptr) {
    free(si);
    si = nullptr;
  }
  cat = 0;
  lsn = 0;
  catlsn = 0;
  fsa = 0;
  lsa = 0;
  cs = 0;
  si_index = 0;
  si_nibble_index = 0;
  data_package_index = 0;
}


void print_package() {
  Serial.println("========== START PACKAGE DATA ==========");
  Serial.print("PACKAGE HEADER: ");      Serial.print(0x55AA, HEX); Serial.print(" | ");
  Serial.print("PACKAGE TYPE: ");        Serial.print(cat, HEX);    Serial.print(" | ");
  Serial.print("LiDAR SAMPLE NUMBER: "); Serial.print(lsn, HEX);    Serial.print(" | ");
  Serial.print("CATLSN: ");              Serial.print(catlsn, HEX); Serial.print(" | ");
  Serial.print("FIRST SAMPLE ANGLE: ");  Serial.print(fsa, HEX);    Serial.print(" | ");
  Serial.print("LAST SAMPLE ANGLE: ");   Serial.print(lsa, HEX);    Serial.print(" | ");
  Serial.print("CHECKSUM: ");            Serial.print(cs,  HEX);    Serial.print(" | ");
  Serial.print("MEASURED DISTANCES: ");         
  for (uint16_t i = 0; i < lsn; i++) {
    Serial.print(" "); Serial.print(si[i], HEX); Serial.print(" ");
  }
  Serial.println();
  Serial.println("========== END PACKAGE DATA ==========");
  Serial.println();Serial.println();
}