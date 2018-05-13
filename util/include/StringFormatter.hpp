#ifndef SISOP2_UTIL_STRINGFORMATTER_HPP
#define SISOP2_UTIL_STRINGFORMATTER_HPP

#include <string>
#include <sstream>

class StringFormatter {
public:
    StringFormatter() = default;
    ~StringFormatter() = default;

    StringFormatter(const StringFormatter&) = delete;
    StringFormatter& operator= (StringFormatter&) = delete;

    template <typename Type>
    StringFormatter& operator<< (const Type& value) {
        stream_ << value;
        return *this;
    }

    operator std::string() const {
        return stream_.str();
    }

private:
    std::stringstream stream_;
};

#endif // SISOP2_UTIL_STRINGFORMATTER_HPP
