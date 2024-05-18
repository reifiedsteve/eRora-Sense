#pragma once

#include <inttypes.h>
#include <Arduino.h> // Just for String.

// Every read of a file is a slow process, which if one os reading several
// files can be so slow that it triggers the watchdog. So this option will
// store all settings in a single file as a monolith. 

#define USER_SETTINGS_DIAGNOSTICS
#define USER_SETTINGS_ARE_MONOLITH

#ifdef USER_SETTINGS_ARE_MONOLITH
#include <map>
#endif

#include <mutex>

class Settings
{
private:

    typedef std::mutex _Mutex;
    typedef std::lock_guard<_Mutex> _ScopedLock;

public:

#   ifdef USER_SETTINGS_ARE_MONOLITH
    enum class CommitMode : uint8_t {
        Manual = 0,
        Auto = 1
    };
#   endif

#   ifdef USER_SETTINGS_ARE_MONOLITH
    inline void setCommitMode(CommitMode mode) {
        _ScopedLock lock(_mutex);
        _commitMode = mode;
    }
#   endif

#   ifdef USER_SETTINGS_ARE_MONOLITH
    bool commit() {
        _ScopedLock lock(_mutex);
        bool commit(_dirty);
        if (commit) {
            _saveCache();
            _dirty = false;
        }
        return commit;
    }
#   endif

protected:

    Settings(const char* settingsNamespace)
      : _namespace(settingsNamespace)
    {}

    void _putSetting(const char* key, const char* str);
    void _putSetting(const char* key, const std::string& str);
    void _putSetting(const char* key, int value);
    void _putSetting(const char* key, bool state);

    std::string _getSetting(const char* key, const char* defaultStr) const;
    std::string _getSetting(const char* key, const std::string& defaultStr) const;
    int _getSetting(const char* key, int defaultValue) const;
    bool _getSetting(const char* key, bool defaultState) const;

    void _putStringLegacy(const char* key, const String& str);
    String _getStringLegacy(const char* key, const char* defaultStr) const;

    static std::string _appendSuffix(const std::string& str, int suffix);

    static std::string _toString(const std::string& str, int value);
    static std::string _toString(int value);

private:

    const char* _namespace;

#   ifdef USER_SETTINGS_ARE_MONOLITH

    typedef std::map<std::string, std::string> _Cache;
    typedef _Cache::iterator _CacheIter;
    typedef _Cache::const_iterator _CacheConstIter;
    typedef _Cache::value_type _CacheItem;

    // TODO: much of that below should NOT be static.
    // TODO: is the _dirty flag use properly throughout? Check.
    
    static bool _initialised;
    static CommitMode _commitMode;

    static _Mutex _mutex;

    static _Cache _cache;
    static bool _dirty;

    static std::string _makeCacheEntryName(const std::string& storeName, const char* key);

    static void _put(const std::string& settingName, bool state);
    static void _put(const std::string& settingName, int value);
    static void _put(const std::string& settingName, const std::string& str);

    static bool _get(const std::string& settingName, bool& state);
    static bool _get(const std::string& settingName, int& value);
    static bool _get(const std::string& settingName, std::string& str);
    
    static void _putToCache(const std::string& settingName, const std::string& str);
    static bool _getFromCache(const std::string& settingName, std::string& str);

    static bool _tryGetFromCache(const std::string& settingName, std::string& str);

    static void _prologue();

    static void _loadCache();
    static void _saveCache();

#   endif
};


