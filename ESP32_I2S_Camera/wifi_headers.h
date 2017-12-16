#ifndef WIFI_HEADERS_H
#define WIFI_HEADERS_H

// https://www.safaribooksonline.com/library/view/80211-wireless-networks/0596100523/ch04.html
// https://mrncciew.files.wordpress.com/2014/10/cwap-mgmt-beacon-01.png

#define WIFI_TYPE_CONTROL 0
#define WIFI_TYPE_MANAGEMENT 1
#define WIFI_TYPE_DATA 2
#define WIFI_SUBTYPE_PROBE 4
#define WIFI_SUBTYPE_PROBE_RESPONSE 5
#define WIFI_SUBTYPE_BEACON 8

struct FrameControl {
  uint8_t version:2;
  uint8_t type:2;
  uint8_t subtype:4;
  bool to_ds:1;
  bool from_ds:1;
  bool more:1;
  bool retry:1;
  bool power_mgmt:1;
  bool more_data:1;
  bool WEP:1;
  bool order:1;
} __attribute__((packed));


struct Sequence {
  uint8_t fragment_number:4;
  uint16_t sequence_number:12;
} __attribute__((packed));

struct WiFiHeader {
  FrameControl frame_control;    // Frame Control (2 bytes)
  uint16_t duration;             // Duration/ID   (2 bytes) Set by SDK
  uint8_t address_1[6];          // Address 1     (6 bytes) (to)
  uint8_t address_2[6];          // Address 2     (6 bytes) (from)
  uint8_t address_3[6];          // Address 3     (6 bytes) (Origin/BSSID)
  Sequence sequence;             // Squence       (2 bytes)

  WiFiHeader() {
    memset(this, 0, sizeof(WiFiHeader));
  }

  WiFiHeader(uint8_t type, uint8_t subtype) {
    memset(this, 0, sizeof(WiFiHeader));
    frame_control.type = type;
    frame_control.subtype = subtype;
  }
} __attribute__((packed));

struct CapabilityInfo { // 16 bits
  bool ESS:1;            // is infrastructure
  bool IBSS:1;           // is IBSS network (ad-hock)
  bool cf_pollable:1;    // Contention-free polling bits
  bool cf_poll_request:1;// Contention-free polling bits
  bool privacy:1;        // WEP encryption.
  bool short_preamable:1;// short preamble. Always 1 for g, optional for b.
  bool PBCC:1;           // Fast bitrate encoding for 802.11b or g
  bool channel_agility:1;// High rate for 802.11b
  unsigned:2;
  bool short_slot_time:1;// for 802.11g
  unsigned:2;
  bool DSSS_OFDM:1;      // ?
  unsigned:2;

  CapabilityInfo() {
    memset(this, 0, sizeof(CapabilityInfo));
  }
} __attribute__((packed));

struct BeaconHeader {
  uint64_t   timestamp;
  uint16_t   interval;  // how often is this sent? in ms
  CapabilityInfo capabilities;
};

template<typename T, uint8_t ID>
struct TagParameter {
  uint8_t element_id;
  uint8_t length;
  T data;

  TagParameter(const T& d)
  : element_id(ID)
  , length(sizeof(T))
  , data(d)
  {}

  TagParameter()
  : element_id(ID)
  , length(sizeof(T))
  {}
} __attribute__((packed));

template<typename T, int NUM, uint8_t ID>
struct TagParameterArray {
  uint8_t element_id;
  uint8_t length;
  T data[NUM];

  TagParameterArray()
  : element_id(ID)
  , length(sizeof(T) * NUM)
  {}
} __attribute__((packed));

struct SSIDZero { // Element ID 0
  uint8_t element_id;
  uint8_t length;
  SSIDZero() : element_id(0), length(0) {}
} __attribute__((packed));

struct SSIDTag {
  uint8_t element_id;
  uint8_t length;
  char ssid[1];
} __attribute__((packed));

typedef TagParameterArray<uint8_t, 8, 1> SupportedRates;


// RxControl struct from ESP8266 SDK Programming Guide 1.5 (Page 191)
//struct RxControl { // Size = 12 bytes
//    signed   rssi:8;              // signal intensity of packet
//    unsigned rate:4;
//    unsigned is_group:1;
//    unsigned:1;
//    unsigned sig_mode:2;      // 0:is 11n packet; 1:is not 11n packet;
//    unsigned legacy_length:12;// if not 11n packet, shows length of packet.
//    unsigned damatch0:1;
//    unsigned damatch1:1;
//    unsigned bssidmatch0:1;
//    unsigned bssidmatch1:1;
//    unsigned MCS:7;           // if is 11n packet, shows the modulation and code used (range from 0 to 76)
//    unsigned CWB:1;           // if is 11n packet, shows if is HT40 packet or not
//    unsigned HT_length:16;    // if is 11n packet, shows length of packet.
//    unsigned Smoothing:1;
//    unsigned Not_Sounding:1;
//    unsigned:1;
//    unsigned Aggregation:1;
//    unsigned STBC:2;
//    unsigned FEC_CODING:1;    // if is 11n packet, shows if is LDPC packet or not.
//    unsigned SGI:1;
//    unsigned rxend_state:8;
//    unsigned ampdu_cnt:8;
//    unsigned channel:4;       // which channel this packet in.
//    unsigned:12;
//} __attribute__((packed));

#endif
