#pragma once

#include <Preferences.h>
#include <functional>
#include <SPIFFS.h> // TODO: This has been deprecated - should be using LittleFS (but integrated with platformio?).

#include "Diagnostics/Logging.h"

/// @brief A mechanism for writing and reading user preferences
/// @brief using a [namespace, key] <==> value model of storage.
/// @brief The similar provision by the Arduino libraries is
/// @brief unfortunately too limited to be useful in our use case.
/// @brief The underlying FS, SPIFFS, is very slow to read and 
/// @brief ultra slow to write, hence we employ caching here so
/// @brief that other code can be free of these otherwise
/// @brief restrictions/considerations to performance.
class PersistentSettings
{
private:

    struct _FileCloser {
        _FileCloser(File& file) : _file(file) {}
        ~_FileCloser() { if (_file) _file.close(); }
        File& _file;
    };
    
public:

    template <typename T>
    static bool set(const char* storeName, const char* key, const T& item); 

    template <typename T>
    static T get(const char* storeName, const char* key, const T& defaultValue);

    static bool set(const char* storeName, const char* key, bool state);
    static bool get(const char* storeName, const char* key, bool defaultState);    
    
    static bool set(const char* storeName, const char* key, const std::string& str);
    static bool set(const char* storeName, const char* key, const char* str);

    static std::string get(const char* storeName, const char* key, const std::string& defaultStr);    
    static std::string get(const char* storeName, const char* key, const char* defaultStr);    
    
    static bool exists(const char* storeName, const char* key);

    static void listAll(bool showContent = false, size_t maxBytes = 256);

private:

    static void _writeTextToFile(const char* storeName, const char* key, const std::string& str);
    static std::string _readTextFromFile(const char* storeName, const char* key, const std::string& defaultStr);

    static bool _writeBytes(const char* storeName, const char* key, const uint8_t* bytes, size_t count);
    static bool _writeBytes(File& file, const uint8_t* bytes, size_t count);

    static size_t _readBytes(const char* storeName, const char* key, uint8_t* bytes, size_t count);
    static size_t _readBytes(File& file, uint8_t* bytes, size_t count);

    static bool _verifyBytes(File& file, const uint8_t* bytes, size_t count);

    static bool _sameBytes(const uint8_t* lhs, const uint8_t* rhs, size_t count);

    static std::string _makePath(const char* storeName, const char* key);

};

template <typename T>
bool PersistentSettings::set(const char* storeName, const char* key, const T& item) {
    Log.verboseln("PersistentSettings:: \"%s:%s\" set to %d", storeName, key, item);
    return (_writeBytes(storeName, key, (uint8_t*)&item, sizeof(T)) == sizeof(T));
}

template <typename T>
T PersistentSettings::get(const char* storeName, const char* key, const T& defaultValue)
{
    T item(defaultValue);
    std::array<uint8_t, sizeof(T)> buffer;

    if (_readBytes(storeName, key, &buffer[0], sizeof(buffer)) == sizeof(buffer)) {
        item = *((T*)&buffer[0]);
    }

    Log.verboseln("PersistentSettings: \"%s:%s\" returns %d.", storeName, key, item);

    return item;
}

