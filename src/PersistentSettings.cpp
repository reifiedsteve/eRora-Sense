#include "PersistentSettings.h"
#include <cmath>
#include <sstream>
#include "Diagnostics/HexTable.h"

bool PersistentSettings::set(const char* storeName, const char* key, bool state)
{
    uint8_t item(state ? 1 : 0);
    return (_writeBytes(storeName, key, (uint8_t*)&item, sizeof(item)));
}

bool PersistentSettings::get(const char* storeName, const char* key, bool defaultState)
{
    bool item(defaultState);
    uint8_t buffer;

    if (_readBytes(storeName, key, &buffer, 1)) {
        item = ((buffer == 0) ? false : true);
    }

    return item;
}

bool PersistentSettings::set(const char* storeName, const char* key, const std::string& str) {
    _writeTextToFile(storeName, key, str);
    return true;
}

bool PersistentSettings::set(const char* storeName, const char* key, const char* str) {
    return set(storeName, key, std::string(str));
}

std::string PersistentSettings::get(const char* storeName, const char* key, const std::string& defaultStr) {
    std::string item(_readTextFromFile(storeName, key, defaultStr));
    Log.verboseln("PersistentSettings: \"%s:%s\" returns \"%s\".", storeName, key, item.c_str());
    return item;
}

std::string PersistentSettings::get(const char* storeName, const char* key, const char* defaultStr) {
    return get(storeName, key, std::string(defaultStr));
}

bool PersistentSettings::exists(const char* storeName, const char* key) {
    std::string filename(_makePath(storeName, key));
    return SPIFFS.exists(filename.c_str());
}

void PersistentSettings::listAll(bool showContent, size_t maxBytes) 
{
    File root = SPIFFS.open("/");
    _FileCloser rootCloser(root);

    File file = root.openNextFile();

    while (file)
    {
        Log.infoln("FS: file \"%s\"", file.name());

        String pathStr(file.name());
        bool ignore(pathStr.endsWith(".html") || pathStr.endsWith(".js") || pathStr.endsWith(".png") || pathStr.endsWith(".css"));

        if (showContent && !ignore) {
            Log.infoln("FS: content follows...");
            std::array<uint8_t, 256> buffer;
            size_t max(std::min(maxBytes, (size_t)256));
            int n = _readBytes(file, &buffer[0], max);
            HexTable::show(&buffer[0], n);
        }

        file.close();
        file = root.openNextFile();
    }

    if (file) file.close();
}

void PersistentSettings::_writeTextToFile(const char* storeName, const char* key, const std::string& str) {
    _writeBytes(storeName, key, (uint8_t*)str.c_str(), str.length());
}

std::string PersistentSettings::_readTextFromFile(const char* storeName, const char* key, const std::string& defaultStr)
{
    std::string returned(defaultStr);

    std::string filename(_makePath(storeName, key));

    if (SPIFFS.exists(filename.c_str()))
    {
        File file(SPIFFS.open(filename.c_str(), FILE_READ));
        _FileCloser closer(file);

        // TODO: can we safely just replace the below with:
        //     String str = file.readString();
        //     return std::string(str.c_str());
        // Does it read the whole file?

        std::stringstream ss;
        
        uint8_t ch;

        while ((_readBytes(file, &ch, 1) > 0)) {
            ss << (char)ch;
        }

        returned = ss.str();
    }

    return returned;
}

bool PersistentSettings::_writeBytes(const char* storeName, const char* key, const uint8_t* bytes, size_t count)
{
    bool done(false);

    std::string filename(_makePath(storeName, key));

    bool requireWrite(true);

    if (SPIFFS.exists(filename.c_str()))
    {
        File file = SPIFFS.open(filename.c_str());
        _FileCloser closer(file);

        if (file && !file.isDirectory()) {
            requireWrite = !_verifyBytes(file, bytes, count); // to avoid unecessary writes.
        }
    }

    if (requireWrite) {
        File file(SPIFFS.open(filename.c_str(), FILE_WRITE));
        _FileCloser closer(file);
        done = _writeBytes(file, bytes, count);
    }

    return done;
}

bool PersistentSettings::_writeBytes(File& file, const uint8_t* bytes, size_t count)
{
    // Log.verboseln("PersistentSettings::_writeBytes entered. Writing %d bytes.", count);

    const uint8_t* ptr = bytes;
    size_t remaining = count;
    size_t n = 0;
    do {
        n = file.write(ptr, remaining);
        ptr += n;
        remaining -= n;
    } while ((n > 0) && (remaining > 0));

    return (remaining == 0);
}

size_t PersistentSettings::_readBytes(const char* storeName, const char* key, uint8_t* bytes, size_t maxCount)
{
    size_t noRead(0);

    std::string filename(_makePath(storeName, key));
    bool exists(SPIFFS.exists(filename.c_str()));

    if (exists)
    {
        File file(SPIFFS.open(filename.c_str()));
        _FileCloser closer(file);

        if (file && !file.isDirectory()) {
            noRead = _readBytes(file, bytes, maxCount);
        }
    }

    return noRead;
}

size_t PersistentSettings::_readBytes(File& file, uint8_t* bytes, size_t count)
{
    uint8_t* ptr = bytes;
    size_t remaining = count;

    bool ongoing(true);

    while (ongoing && (remaining > 0)) {
        size_t n = file.read(ptr, remaining);
        ptr += n;
        remaining -= n;
        ongoing = (n > 0);
    }

    return count - remaining;
}

bool PersistentSettings::_verifyBytes(File& file, const uint8_t* bytes, size_t count)
{
    const uint8_t* ptr = bytes;
    size_t remaining = count;

    bool ongoing(true);

    while (ongoing && (remaining > 0)) {
        uint8_t b;
        size_t n = file.read(&b, 1);
        ongoing = (b == *ptr);
        if (ongoing) {
            ptr += n;
            remaining -= n;
            ongoing = (n > 0);
        }
    }
    
    return (remaining == 0);
}

bool PersistentSettings::_sameBytes(const uint8_t* lhs, const uint8_t* rhs, size_t count) {
    return std::equal(lhs, lhs + count, rhs);
}

std::string PersistentSettings::_makePath(const char* storeName, const char* key) {
    return std::string("/") + std::string(storeName) + "/" + std::string(key);
}
