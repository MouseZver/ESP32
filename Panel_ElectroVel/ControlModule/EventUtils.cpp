#include "EventUtils.h"

// === int как name ===
template void sendEvent<int, int, decltype(nullptr)>(int, int, decltype(nullptr));
template void sendEvent<int, bool, decltype(nullptr)>(int, bool, decltype(nullptr));
template void sendEvent<int, const char*, decltype(nullptr)>(int, const char*, decltype(nullptr));
template void sendEvent<int, int, const char*>(int, int, const char*);

// === int как value ===
template void sendEvent<const char*, int, decltype(nullptr)>(const char*, int, decltype(nullptr));
template void sendEvent<const char*, int, const char*>(const char*, int, const char*);

// === uint8_t как value ===
template void sendEvent<const char*, uint8_t, decltype(nullptr)>(const char*, uint8_t, decltype(nullptr));
template void sendEvent<const char*, uint8_t, const char*>(const char*, uint8_t, const char*);

// === float как value ===
template void sendEvent<const char*, float, decltype(nullptr)>(const char*, float, decltype(nullptr));
template void sendEvent<const char*, float, const char*>(const char*, float, const char*);

// === bool как name и value ===
template void sendEvent<bool, bool, decltype(nullptr)>(bool, bool, decltype(nullptr));
template void sendEvent<bool, const char*, decltype(nullptr)>(bool, const char*, decltype(nullptr));

// === String как name и value ===
template void sendEvent<String, String, decltype(nullptr)>(String, String, decltype(nullptr));
template void sendEvent<String, int, decltype(nullptr)>(String, int, decltype(nullptr));
template void sendEvent<String, const char*, decltype(nullptr)>(String, const char*, decltype(nullptr));

// === const char* как name (обычный случай) ===
//template void sendEvent<const char*, int, decltype(nullptr)>(const char*, int, decltype(nullptr));
template void sendEvent<const char*, bool, decltype(nullptr)>(const char*, bool, decltype(nullptr));
template void sendEvent<const char*, const char*, decltype(nullptr)>(const char*, const char*, decltype(nullptr));
//template void sendEvent<const char*, int, const char*>(const char*, int, const char*);
template void sendEvent<const char*, bool, const char*>(const char*, bool, const char*);
template void sendEvent<const char*, const char*, const char*>(const char*, const char*, const char*);

// === int как name, const char* как value и event ===
template void sendEvent<int, const char*, const char*>(int, const char*, const char*);
//template void sendEvent<int, int, const char*>(int, int, const char*);
template void sendEvent<int, bool, const char*>(int, bool, const char*);
//template void sendEvent<int, const char*, decltype(nullptr)>(int, const char*, decltype(nullptr));

// === bool как name, const char* как value и event ===
template void sendEvent<bool, const char*, const char*>(bool, const char*, const char*);
//template void sendEvent<bool, bool, decltype(nullptr)>(bool, bool, decltype(nullptr));