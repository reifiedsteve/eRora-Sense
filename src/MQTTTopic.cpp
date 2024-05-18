#include "MQTTTopic.h"

#include <sstream>
#include <algorithm>

MQTTTopic::MQTTTopic() 
    : _parts()
{}

MQTTTopic::MQTTTopic(const std::string& topic) 
    : _parts()
{
    if (!_tryParse(topic, _parts)) {
        _parts.clear();
    }
} 

MQTTTopic::MQTTTopic(const char* topic) 
    : _parts()
{
    if (!_tryParse(topic, _parts)) {
        _parts.clear();
    }
} 

MQTTTopic::MQTTTopic(const MQTTTopic& rhs) 
    : _parts(rhs._parts)
{}

bool MQTTTopic::isValid() const {
    return _parts.size() > 0;
}

size_t MQTTTopic::count() const {
    return _parts.size();
}

const std::string& MQTTTopic::operator[](int index) const {
    return _parts[index];
}

bool MQTTTopic::matches(const MQTTTopic& rhs) const {
    std::vector<_Part> matchedParts;
    return _matches(rhs, std::inserter(matchedParts, matchedParts.begin()));
}

bool MQTTTopic::operator< (const MQTTTopic& rhs) const {
    return _render(_parts) < _render(rhs._parts);  // ...TODO: better impl. as highly sub-optimal.
}

MQTTTopic& MQTTTopic::operator=(const MQTTTopic& rhs) {
    if (&rhs != this) {
        _parts = rhs._parts;
    }
    return *this;
}

bool MQTTTopic::operator==(const MQTTTopic& rhs) const {
    return (_parts.size() == rhs._parts.size()) && std::equal(_parts.begin(), _parts.end(), rhs._parts.begin());
}

bool MQTTTopic::operator!=(const MQTTTopic& rhs) const {
    return (_parts.size() != rhs._parts.size()) || !std::equal(_parts.begin(), _parts.end(), rhs._parts.begin());
}

MQTTTopic MQTTTopic::operator+(const MQTTTopic& rhs) const {
    MQTTTopic result(*this);
    for (auto part : rhs._parts)
    result._parts.push_back(part);
    return result;
}

std::string MQTTTopic::str() const {
    return _render(_parts);
}

bool MQTTTopic::_tryParse(const std::string& str, _Parts& parts) 
{
    bool parsed(false);

    std::istringstream is(str);
    _Part part;

    parts.clear();

    if (std::getline(is, part, '/'))  
    {
        if (part.length() > 0) {  //... a leading '/' is optional but insignificant.
            parts.push_back(part);
        }

        parsed = true;

        while (parsed && std::getline(is, part, '/')) {
            parsed = (part.length() > 0);
            if (parsed) {
                parts.push_back(part);
            }
        }
    }

    return parsed;
}

std::string MQTTTopic::_render(const _Parts& parts) {
    std::ostringstream os;
    size_t n(parts.size());
    if (n > 0) {
        os << parts[0];
        for (int i = 1; i < n; ++i) {
            os << '/' << parts[i];
        }
    }
    return os.str();
}
