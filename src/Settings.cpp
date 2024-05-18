#include <sstream>

#include "Settings.h"
#include "Diagnostics/Logging.h"

#include "PersistentSettings.h"

void Settings::_putSetting(const char* key, const char* str) {
    _putSetting(key, std::string(str));
}

void Settings::_putSetting(const char* key, const std::string& str) {
#   ifdef USER_SETTINGS_ARE_MONOLITH
    _put(_makeCacheEntryName(_namespace, key), str);
#   else
    PersistentSettings::set(storeName, key, str);
#   endif
}

void Settings::_putSetting(const char* key, int value) {
#   ifdef USER_SETTINGS_ARE_MONOLITH
    _put(_makeCacheEntryName(_namespace, key), value);
#   else
    PersistentSettings::set(storeName, key, value);
#   endif
}

void Settings::_putSetting(const char* key, bool state) {
#   ifdef USER_SETTINGS_ARE_MONOLITH
    _put(_makeCacheEntryName(_namespace, key), state);
#   else
    PersistentSettings::set(storeName, key, state);
#   endif
}

std::string Settings::_getSetting(const char* key, const char* defaultStr) const {
    return _getSetting(key, std::string(defaultStr));
}

std::string Settings::_getSetting(const char* key, const std::string& defaultStr) const {
#   ifdef USER_SETTINGS_ARE_MONOLITH
    std::string str;
    return _get(_makeCacheEntryName(_namespace, key), str) ? str : defaultStr;
#   else
    return PersistentSettings::get(storeName, key, defaultStr);
#   endif
}

int Settings::_getSetting(const char* key, int defaultValue) const {
#   ifdef USER_SETTINGS_ARE_MONOLITH
    int value;
    return _get(_makeCacheEntryName(_namespace, key), value) ? value : defaultValue;
#   else
    return PersistentSettings::get(storeName, key, defaultValue);
#   endif
}

bool Settings::_getSetting(const char* key, bool defaultState) const {
#   ifdef USER_SETTINGS_ARE_MONOLITH
    bool state;
    return _get(_makeCacheEntryName(_namespace, key), state) ? state : defaultState;
#   else
    return PersistentSettings::get(storeName, key, defaultState);
#   endif
}

void Settings::_putStringLegacy(const char* key, const String& str) {
    _putSetting(key, std::string(str.c_str()));
}

String Settings::_getStringLegacy(const char* key, const char* defaultStr) const {
    return String(_getSetting(key, defaultStr).c_str());
}

std::string Settings::_appendSuffix(const std::string& str, int suffix) {
    std::stringstream ss;
    ss << str << suffix;
    return ss.str();
}

#ifdef USER_SETTINGS_ARE_MONOLITH

std::string Settings::_makeCacheEntryName(const std::string& storeName, const char* key) {
    return std::string(storeName) + ":" + key;
}

void Settings::_put(const std::string& settingName, bool state) {
    _put(settingName, (state ? 1 : 0));
}

void Settings::_put(const std::string& settingName, int value) {
    std::ostringstream ss;
    ss << value;
    _put(settingName, ss.str());
}

void Settings::_put(const std::string& settingName, const std::string& str) {
    _putToCache(settingName, str);
}

bool Settings::_get(const std::string& settingName, bool& state) {
    int value;
    bool ok(_get(settingName, value));
    if (ok) state = (value != 0);
    return ok;
}

bool Settings::_get(const std::string& settingName, int& value) {
    std::string valueStr;
    bool ok(_getFromCache(settingName, valueStr));
    if (ok) {
        std::stringstream is(valueStr);
        is >> value;
    }
    return ok;
}

bool Settings::_get(const std::string& settingName, std::string& str) {
    return _getFromCache(settingName, str);
}

void Settings::_putToCache(const std::string& settingName, const std::string& str)
{
    _ScopedLock lock(_mutex);

    #ifdef USER_SETTINGS_DIAGNOSTICS
    Log.verboseln("Settings: writing \"%s\" = \"%s\".", settingName.c_str(), str.c_str());
    #endif

    std::string old;

    bool needToWrite(false);
    bool alreadyGot(_tryGetFromCache(settingName, old));

    if (!alreadyGot) {
        Log.verboseln("Settings: fresh value, so need to write.");
        needToWrite = true;
    } else {
        Log.verboseln("Settings: old value is '%s', new value is '%s', so....", old.c_str(), str.c_str());
        needToWrite = (old != str);
    }

    if (needToWrite) {
        Log.verboseln("Settings: writing to settings cache.");
        _cache[settingName] = str;
        _dirty = true;
    }

    if (_commitMode == CommitMode::Auto) {
        Log.verboseln("Settings: committing cache to non-volatile storage.");
        _saveCache();
    }
}

bool Settings::_getFromCache(const std::string& settingName, std::string& str) {
    _ScopedLock lock(_mutex);
    return _tryGetFromCache(settingName, str);
}

bool Settings::_tryGetFromCache(const std::string& settingName, std::string& str)
{
    _prologue();

    _CacheConstIter iter(_cache.find(settingName));
    bool got(iter != _cache.end());
    
    if (got) { str = iter->second; }

    #ifdef USER_SETTINGS_DIAGNOSTICS
    if (got) {
        //Log.verboseln("Settings: reading \"%s\" as \"%s\".", settingName.c_str(), str.c_str());
    } else {
        // Log.verboseln("Settings: reading \"%s\" as <no-value>.", settingName.c_str(), str.c_str());
    }
    #endif

    return got;
}

void Settings::_prologue() {
    if (!_initialised) {
        _loadCache();
        _initialised = true;
    }
}

void Settings::_loadCache()
{
    std::string blob(PersistentSettings::get("settings", "blob", ""));
    std::stringstream is(blob);

    do {
        std::string settingName, settingValue;
        if (!is.eof()) {
            getline(is, settingName);
            if (!is.eof()) {
                getline(is, settingValue);
                if (!is.eof()) {
                    Log.verboseln("UserSetting: loaded \"%s\" as \"%s\".", settingName.c_str(), settingValue.c_str());
                    _cache[settingName] = settingValue;
                }
            }
        }
    } while(!is.eof());
}

void Settings::_saveCache()
{
    std::stringstream ss;

    for (const _CacheItem& item : _cache) {
        ss << item.first << std::endl << item.second << std::endl;
    }
    
    std::string blob(ss.str());
    Log.verboseln("UserSetting: writing settings blob as \"%s\" (len=%d).", blob.c_str(), blob.size());
    PersistentSettings::set("settings", "blob", blob);
}

#endif

std::string Settings::_toString(const std::string& str, int value) {
    std::ostringstream ss;
    ss << str << value;
    return ss.str(); 
}

std::string Settings::_toString(int value) {
    std::ostringstream ss;
    ss << value;
    return ss.str(); 
}

#ifdef USER_SETTINGS_ARE_MONOLITH
bool Settings::_initialised(false);
Settings::CommitMode Settings::_commitMode(Settings::CommitMode::Auto);
Settings::_Cache Settings::_cache;
bool Settings::_dirty(false);
#endif

Settings::_Mutex Settings::_mutex;
