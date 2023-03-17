#include <iostream>
#include <cstring>

class String {
private:
    size_t len = 0, capacity = 0;
    char* str = nullptr;
    
    void swap(String& other) {
        std::swap(len, other.len);
        std::swap(capacity, other.capacity);
        std::swap(str, other.str);
    }

    void reserve(size_t newcap) {
        if (newcap <= capacity) {
            return;
        }
        char* newstr = new char[newcap];
        memcpy(newstr, str, len);
        delete[] str;
        str = newstr;
        capacity = newcap;
    }

    bool isSubstring(size_t start, const String& substr) const {
        for (size_t i = 0; i < substr.length(); ++i) {
            if (str[i + start] != substr[i]) {
                return false;
            }
        }
        return true;
    }

public:
    String() {}

    String(char c) : String(1, c) {}

    String(const char* string) : len(strlen(string)), capacity(strlen(string)) {
        str = new char[capacity];
        memcpy(str, string, len);
    }

    String(size_t n, char c) : len(n), capacity(n) {
        str = new char[capacity];
        memset(str, c, n);
    }

    String(const String& s) : len(s.len), capacity(s.len) {
        str = new char[capacity];
        memcpy(str, s.str, len);
    }

    String& operator=(const String& s) {
        String ss(s);
        swap(ss);
        return *this;
    }

    ~String() {
        delete[] str;
    }

    size_t length() const {
        return len;
    }

    bool empty() const {
        return len == 0;
    }

    void clear() {
        delete[] str;
        str = nullptr;
        len = 0;
        capacity = 0;
    }

    void push_back(char c) {
        if (len == capacity) {
            reserve(capacity == 0 ? 1 : 2 * capacity);
        }
        str[len++] = c;
    }

    void pop_back() {
        --len;
    }

    char& operator[](size_t ind) {
        return str[ind];
    }
    
    const char& operator[](size_t ind) const {
        return str[ind];
    }

    char& front() {
        return str[0];
    }
    
    const char& front() const {
        return str[0];
    }

    char& back() {
        return str[len - 1];
    }

    const char& back() const {
        return str[len - 1];
    }

    String& operator += (const String& rhs) {
        if (capacity == 0) {
            reserve(1);
        }
        while (capacity < len + rhs.len) {
            reserve(capacity * 2);
        }
        memcpy(str + len, rhs.str, rhs.len);
        len += rhs.len;
        return *this;
    }

    String substr(size_t start, size_t count) const {
        String res(count, '\0');
        memcpy(res.str, str + start, count);
        return res;
    }

    bool operator == (const String& rhs) const {
        if (length() != rhs.length()) {
            return false;
        }
        return isSubstring(0, rhs);
    }

    size_t find(const String& substring) const {
        for (size_t i = 0; i + substring.length() <= len; ++i) {
            if (isSubstring(i, substring)) {
                return i;
            }
        }
        return len;
    }

    size_t rfind(const String& substring) const {
        for (int i = static_cast<int>(len) - substring.length(); i >= 0; --i) {
            if (isSubstring(i, substring)) {
                return i;
            }
        }
        return len;
    }
};

std::ostream& operator << (std::ostream& out, const String& s) {
    for (size_t i = 0; i < s.length(); ++i) {
        out << s[i];
    }
    return out;
}

std::istream& operator >> (std::istream& in, String& str) {
    str.clear();
    char c;
    do {
        c = in.get();
    } while (isspace(c));
    while (!in.eof() && !isspace(c)) {
        str.push_back(c);
        c = in.get();
    }
    return in;
}

String operator + (const String& lhs, const String& rhs) {
    String ans = lhs;
    ans += rhs;
    return ans;
}

