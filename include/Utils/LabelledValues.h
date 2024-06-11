#pragma once

#include <map>
#include <iterator>

/// @brief Really just an abstraction of std::map that has more direct semantics 
/// and hides the need to use iterators. All inline so lightweight.
class LabelledValues
{
public:

    inline void add(const std::string& label, const std::string& value) __attribute__((always_inline)) {
        _map[label] = value;
    }

    inline bool has(const std::string& label) const __attribute__((always_inline)) {
        return _map.find(label) != _map.end();
    }

    inline const std::string& get(const std::string& label) const __attribute__((always_inline)) {
        _MapConstIter iter(_map.find(label));
        return (iter == _map.end()) ? _none : iter->second;
    }

    inline const std::string& label(int i) const __attribute__((always_inline)) {
        _MapConstIter iter(_map.begin());
        std::advance(iter, i);
        return (iter == _map.end()) ? _none : iter->first;
    }

    inline const std::string& value(int i) const __attribute__((always_inline)) {
        _MapConstIter iter(_map.begin());
        std::advance(iter, i);
        return (iter == _map.end()) ? _none : iter->second;
    }

    inline size_t size() const __attribute__((always_inline)) {
        return _map.size();
    }

private:

    typedef std::map<std::string, std::string> _Map;
    typedef _Map::const_iterator _MapConstIter;
    typedef _Map::iterator _MapIter;

    static const std::string _none;

    _Map _map;
};