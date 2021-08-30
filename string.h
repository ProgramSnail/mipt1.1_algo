#include <iostream>
#include <cstring>

class String {
private:
    static const size_t MIN_SIZE;
    static const uint SIZE_K; 
    size_t count;
    size_t sz;
    char* str = nullptr;
    void resize(size_t newSz) {
        newSz = std::max(std::max(MIN_SIZE, newSz), count + 1);
        if (newSz == sz) {
            return;
        }
        char* newStr = new char[newSz];
        memcpy(newStr, str, count);
        sz = newSz;
        std::swap(str, newStr);
        delete[] newStr;
    }
public:
    String() : count(0), sz(MIN_SIZE), str(new char[sz]) {}
    
    String(size_t n, char ch = 0) : count(ch == 0 ? 0 : n), 
        sz(std::max(MIN_SIZE, SIZE_K * count)), str(new char[sz]) {
        if (ch != 0 && count > 0) {
            memset(str, ch, count);
        }
    }

    String(char ch) : count(ch == 0 ? 0 : 1), 
        sz(std::max(MIN_SIZE, SIZE_K * count)), str(new char[sz]) {
        if (ch != 0) {
            str[0] = ch;
        }
    }
    
    String(const char* s) {
        for (count = 0; s[count] != 0; ++count) {}
        sz = std::max(MIN_SIZE, SIZE_K * count);
        str = new char[sz];
        memcpy(str, s, count);
    }
    
    String(const String& s) : count(s.size()), 
        sz(std::max(MIN_SIZE, SIZE_K * count)), str(new char[sz]) {
        memcpy(str, s.str, count);
    } 
    
    void pop_back() {
        if (count <= 0) {
            return;
        }
        count--;
        if (sz > MIN_SIZE && count * SIZE_K < sz / SIZE_K) {
            resize(sz / SIZE_K);
        }    
    }
    
    void push_back(char ch) {
        if (count == sz) {
            resize(sz * SIZE_K);
        }
        str[count] = ch;
        count++;
    }
    
    char& operator[](size_t i) {
        return str[i];
    }
    
    char operator[](size_t i) const {
        return str[i];
    }
    
    char& front() {
        return str[0];
    }
    
    char front() const {
        return str[0];
    }
    
    char& back() {
        return (count > 0) ? str[count - 1] : str[0];
    }
    
    char back() const {
        return (count > 0) ? str[count - 1] : str[0];
    }
    
    String substr(size_t pos, size_t num) const {
        if (pos + num > count) {
            return String();
        }
        String s(num);
        memcpy(s.str, str + pos, num);
        s.count = num;
        return s;
    }
    
    void swap(String& s) {
        std::swap(count, s.count);
        std::swap(sz, s.sz);
        std::swap(str, s.str);
    }
    
    String& operator=(const String& s) {
        String sCopy(s);
        swap(sCopy);
        return *this;
    }
    
    String& operator+=(const String& s) { 
        if (sz < s.size() + count) {
            resize(std::max(s.size() + count, SIZE_K * sz));
        }
        memcpy(str + count, s.str, s.size());
        count += s.size();
        return *this;
    }
    
    String& operator+=(char ch) {
        push_back(ch);
        return *this;
    }
    
    size_t find(const String& s, bool reverse = false) const {
        bool b = false;
        for (size_t i = 0; i + s.size() <= count; ++i) {
            b = true;
            for (size_t j = 0; j < s.size(); ++j) {
                if (s[reverse ? (s.size() - j - 1) : j] != 
                    str[reverse ? (count - i - j - 1) : (i + j)]) {
                    b = false;
                    break;
                }
            }
            if (b) {
                return reverse ? (count - i - s.size()) : i;
            }
        }
        return count;
    }
    
    size_t rfind(const String& s) const {
        return find(s, true);
    }
    
    size_t size() const {
        return count;
    }
    
    size_t length() const {
        return count;
    }
    
    bool empty() const {
        return count == 0;
    }
    
    void clear() {
        count = 0;
        resize(0);
    }
    
    ~String() {
        delete[] str;
    }
};

String operator+(const String& a, const String& b) {
    String aCopy(a);
    aCopy += b;
    return aCopy;
}

bool operator==(const String& a, const String& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& out, const String& s) {
    for (size_t i = 0; i < s.size(); ++i) {
        out << s[i];
    }
    return out;
}

std::istream& operator>>(std::istream& in, String& s) {
    char ch;
    bool sInput = false;
    s.clear();
    for (;;) {
        in.get(ch);
        if (in.gcount() == 0) {
            break;
        }
        if (ch == ' ' || ch == '\n' || ch == '\r') {
            if (sInput) {
                break;
            }
        } else {
            sInput = true;
            s.push_back(ch);
        }
    }
    return in;
}

const size_t String::MIN_SIZE = 4;
const uint String::SIZE_K = 2;
