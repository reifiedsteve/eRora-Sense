#pragma once

#include <string>
#include <vector>
#include <sstream>


class MQTTTopic
{
public:

    MQTTTopic();
    explicit MQTTTopic(const std::string& topic);
    explicit MQTTTopic(const char* topic);
    MQTTTopic(const MQTTTopic& rhs);

    bool isValid() const;
    size_t count() const;

    const std::string& operator[](int index) const;

    bool matches(const MQTTTopic& rhs) const;

    template <typename _Inserter>
    bool matches(const MQTTTopic& rhs, _Inserter inserter) const {
        std::vector<_Part> matchedParts;
        return _matches(rhs, inserter);
    }

    bool operator< (const MQTTTopic& rhs) const;

    MQTTTopic& operator=(const MQTTTopic& rhs);

    bool operator==(const MQTTTopic& rhs) const;
    bool operator!=(const MQTTTopic& rhs) const;

    MQTTTopic operator+(const MQTTTopic& rhs) const;

    std::string str() const;

private:

    typedef std::string _Part;
    typedef std::vector<_Part> _Parts;

    template <typename _Inserter>
    bool _matches(const MQTTTopic& rhs, _Inserter inserter) const
    {
        // '+' is a (whole) part wildcard (several allowed).
        // '*' is (possibly) multi-part trailing sub-path.

        bool matches(false);

        size_t n1(_parts.size());
        size_t n2(rhs._parts.size());

        if ((n1 <= n2) && (n1 > 0) && (n2 > 0))
        {
            int n(std::min(n1, n2));

            if (n1 < n2) {
                matches = _parts[n1-1] == "#";    // TODO: is setting matches true here the correct thing to do? Prob not. Check.
                //Log.verboseln("MTQQTopic::_matches - wildcard present");
            } else {
                matches = true;
            }

            int i = 0;

            while (matches && (i < n))
            {
                if (_parts[i] == "+") {
                    //Log.verboseln("MTQQTopic::_matches - matched part is \"%s\"", rhs._parts[i].c_str());
                    *inserter++ = rhs._parts[i];
                }
                
                else if (_parts[i] == "#") {
                    //Log.verboseln("MTQQTopic::_matches - matches and wildcard present, so extracting trailing match");
                    std::ostringstream os;
                    int last = rhs._parts.size()  - 1;
                    while (i <= last) {
                        os << rhs._parts[i];
                        if (i < last) os << "/";
                        ++i;
                    }
                    //Log.verboseln("MTQQTopic::_matches - matched part is \"%s\"", os.str().c_str());
                    *inserter++ = os.str();
                }
                
                else {
                    matches = _parts[i] == rhs._parts[i];
                    //Log.verboseln("MTQQTopic::_matches - parts \"%s\" and \"%s\" do %smatch", _parts[i].c_str(), rhs._parts[i].c_str(), matches ? "" : "not ");
                }

                ++i;
            }
        }

        return matches;
    }

    bool  _tryParse(const std::string& str, _Parts& parts);

    static std::string _render(const _Parts& parts);

    _Parts _parts;
};


