#include <iostream>
#include <vector>
#include <string>
#include <complex>

namespace fft {
    const long double PI = acosl(-1);
    using comp = std::complex<double>;
    using Polynom = std::vector<int64_t>;
    using CompPolynom = std::vector<comp>;

    CompPolynom toCompPolynom(const Polynom& p) {
        CompPolynom x(p.size());
        for (size_t i = 0; i < p.size(); ++i) {
            x[i] = p[i];
        }
        return x;
    }

    Polynom toPolynom(const CompPolynom& p) {
        Polynom x(p.size());
        for (size_t i = 0; i < p.size(); ++i) {
            x[i] = ((int)p[i].real() + 0.5);
        }
        return x;
    }

    comp getKCompRoot(size_t n, size_t k = 1) {
        double angle = (2 * PI * k) / n;
        return comp(cos(angle), sin(angle));
    }

    CompPolynom fft(const CompPolynom& p, comp w) {
        if (p.size() == 1) {
            return p;
        }
        size_t s = p.size() / 2;
        CompPolynom a(s);
        CompPolynom b(s);
        CompPolynom ans(p.size());
        for (size_t i = 0; i < p.size(); ++i) {
            (i % 2 == 0 ? a[i / 2] : b[i / 2]) = p[i];
        }
        a = fft(a, w * w);
        b = fft(b, w * w);
        for (size_t i = 0; i < s; ++i) {
            ans[i] = a[i] + w * b[i];
            ans[i + s] = a[i] - w * b[i];
        }
        return ans;
    }

    CompPolynom fft(const CompPolynom& p) {
        return fft(p, getKCompRoot(p.size(), 1));
    }

    CompPolynom fftBack(const CompPolynom& p) {
        return fft(p, getKCompRoot(p.size(), -1));
    } 

    Polynom mult(Polynom a, Polynom b) {
        size_t n = 1;
        while (n < a.size() || n < b.size()) {
            n *= 2;
        }
        n *= 2;
        a.resize(n, 0);
        b.resize(n, 0);
        CompPolynom ca = fft(toCompPolynom(a));
        CompPolynom cb = fft(toCompPolynom(b));
        CompPolynom ans(n);
        for (size_t i = 0; i < n; ++i) {
            ans[i] = ca[i] + cb[i];
        }
        return toPolynom(fftBack(ans));
    } 
}

class BigInteger {
private:
    using elemVec = std::vector<int64_t>;

    static const int64_t MEMBER_SIZE = 1000;
    static const int64_t MEMBER_POW = 3;
    static const int64_t BASIS = 10;
    bool isPositive = true;
    elemVec a;
    BigInteger& norm() {
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] >= MEMBER_SIZE) {
                if (i + 1 == a.size()) {
                    a.push_back(0);
                }
                a[i + 1] += a[i] / MEMBER_SIZE;
                a[i] %= MEMBER_SIZE;
            }
        }
        for (; a.size() > 0 && a.back() == 0;) {
            a.pop_back();
        }
        if (a.size() == 0) {
            setPositive();
        }
        return *this;
    }

    BigInteger& mult(int x) {
        if (x < 0) {
            isPositive = !isPositive;
        }
        x = abs(x);
        if (x == 0) {
            clear();
        } else {
            for (size_t i = 0; i < a.size(); ++i) {
                a[i] *= x;
            }
            norm();
        }
        return *this;
    }

    BigInteger multCpy(int x) const {
        BigInteger cp = *this;
        cp.mult(x);
        return cp;
    }

    std::pair<BigInteger, BigInteger> div(const BigInteger& x) const;

public:
    friend bool operator==(const BigInteger& x, const BigInteger& y);
    friend bool operator<(const BigInteger& x, const BigInteger& y);
    friend BigInteger mult(const BigInteger& x, const BigInteger& y);
    friend BigInteger fastMult(const BigInteger& x, const BigInteger& y);

    BigInteger() {}

    BigInteger(const BigInteger& x) {
        isPositive = x.isPositive;
        a = x.a;
    }

    BigInteger& fftMult(const BigInteger& x) {
        fft::Polynom p = fft::mult(this->a, x.a);
        a = p;
        norm();
        return *this;
    }
    
    BigInteger(int x) : isPositive(x >= 0) {
        x = abs(x);
        if (x != 0) {
            a.push_back(x);
            norm();
        }
    }

    BigInteger(int x, size_t pow) : isPositive(x >= 0 || pow % 2 == 0) {
        a.push_back(1);
        x = abs(x);
        if (x > 1) {
            for (size_t i = 0; i < pow; i++) {
                mult(x);
            }
        } else {
            if (x == 0 && pow != 0) {
                a.back() = 0;
            }
        }
    }

    BigInteger(const std::string& s) {
        if (s.size() == 0) {
            return;
        }
        int64_t powNum = 1;
        size_t member = 0;
        for (size_t i = s.size() - 1;; --i, powNum *= 10) {
            if (i == 0 && s[i] == '-') {
                isPositive = false;
                break;
            }
            if (powNum >= MEMBER_SIZE) {
                powNum /= MEMBER_SIZE;
                member++;
            }
            if (a.size() <= member) {
                a.push_back(0);
            }
            if (s[i] != '0') {
                a[member] += powNum * (s[i] - '0');
            }
            if (i == 0) {
                break;
            }
        }
        norm();
    }

    explicit operator bool() const {
        return a.size() != 0;
    }

    explicit operator double() const {
        double d = 0;
        if (a.size() > 0) {
            for (size_t i = a.size() - 1;; i--) {
                d *= MEMBER_SIZE;
                d += a[i];
                if (i == 0) {
                    break;
                }
            }
            d *= (isPositive?1:-1);
        }
        return d;
    }

    BigInteger& setPositive() {
        isPositive = true;
        return *this;
    }

    BigInteger getsetPositiveed() const {
        BigInteger cp = *this;
        cp.setPositive();
        return cp;
    }
    
    void swap(BigInteger& x) {
        std::swap(isPositive, x.isPositive);
        a.swap(x.a);
    }

    BigInteger& operator<<=(uint x) {
        if (a.size() == 0 || x == 0) {
            return *this;
        }
        a.resize(a.size() + x, 0);
        for (size_t i = a.size() - x - 1;; --i) {
            a[i + x] = a[i];
            a[i] = 0;
            if (i == 0) {
                break;
            }
        }
        return *this;
    }

    BigInteger& operator>>=(uint x) {
        if (x == 0) {
            return *this;
        }
        if (x >= a.size()) { 
            clear();
            return *this;
        }
        for (size_t i = x; i < a.size(); ++i) {
            a[i - x] = a[i];
        }
        a.resize(a.size() - x);
        return *this;
    }

    BigInteger operator<<(uint x) {
        BigInteger cp = *this;
        cp <<= x;
        return cp;
    }

    BigInteger operator>>(uint x) {
        BigInteger cp = *this;
        cp >>= x;
        return cp;
    }

    BigInteger& operator=(const BigInteger& x) {
        BigInteger y(x);
        swap(y);
        return *this;
    }

    BigInteger& operator+=(const BigInteger& x);
    
    BigInteger& operator-=(const BigInteger& x) {
        operator+=(-x);
        return *this;
    }
    
    BigInteger& operator*=(const BigInteger& x) {
        *this = fastMult(*this, x);
        norm();
        return *this;
    }
    
    BigInteger& operator/=(const BigInteger& x) {
        *this = div(x).first;
        return *this;
    }
    
    BigInteger& operator%=(const BigInteger& x) {
        *this = div(x).second;
        return *this;
    }
    
    BigInteger& operator++() {
        operator+=(1);
        return *this;
    }
    
    BigInteger operator++(int) {
        BigInteger cp = *this;
        operator++();
        return cp;
    }
    
    BigInteger& operator--() {
        operator+=(-1);
        return *this;
    }
    
    BigInteger operator--(int) {
        BigInteger cp = *this;
        operator--();
        return cp;
    }
    
    BigInteger operator-() const {
        BigInteger cp = *this;
        if (cp.a.size() > 0) {
            cp.isPositive = !isPositive;
        }
        return cp;
    }

    void clear() {
        setPositive();
        a.clear();
    }
    
    std::string toString() const {
        if (a.size() == 0) {
            return "0";
        }
        std::string ans;
        int64_t x;
        int64_t pos;
        bool numStarted = false;
        if (!isPositive) {
            ans += '-';
        }
        for (size_t i = a.size() - 1;; --i) {
            pos = MEMBER_SIZE;
            for (size_t j = MEMBER_POW; j > 0; --j) {
                pos /= BASIS;
                x = (a[i] / pos) % BASIS;
                if (i + 1 == a.size() && x == 0 && !numStarted) {
                    continue;
                }
                numStarted = true;
                ans += (x + '0');
            }
            if (i == 0) {
                break;
            }
        }
        return ans;
    }

    bool getPositivity() const {
        return isPositive;
    }

    void setPositivity(bool s) {
        if (a.size() > 0) {
            isPositive = s;
        }
    }

};

BigInteger operator+(const BigInteger& x, const BigInteger& y) {
    BigInteger cp = x;
    cp += y;
    return cp;
}

BigInteger operator-(const BigInteger& x, const BigInteger& y) {
    BigInteger cp = x;
    cp -= y;
    return cp;
}

BigInteger operator*(const BigInteger& x, const BigInteger& y) {
    BigInteger cp = x;
    cp *= y;
    return cp;
}

BigInteger operator/(const BigInteger& x, const BigInteger& y) {
    BigInteger cp = x;
    cp /= y;
    return cp;
}

BigInteger operator%(const BigInteger& x, const BigInteger& y) {
    BigInteger cp = x;
    cp %= y;
    return cp;
}

bool operator==(const BigInteger& x, const BigInteger& y) {
    if (x.a.size() != y.a.size() || x.isPositive != y.isPositive) {
        return false;
    }
    for (size_t i = 0; i < x.a.size(); ++i) {
        if (x.a[i] != y.a[i]) {
            return false;
        }
    }
    return true;
}

bool operator<(const BigInteger& x, const BigInteger& y) {
    if (x.isPositive != y.isPositive) {
        return y.isPositive;
    }
    if (x.a.size() != y.a.size()) {
        return x.isPositive == (x.a.size() < y.a.size());
    }
    if (x.a.size() == 0) {
        return false;
    }
    for (size_t i = x.a.size() - 1;; --i) {
        if (x.a[i] != y.a[i]) {
            return x.isPositive == (x.a[i] < y.a[i]);  
        }
        if (i == 0) {
            break;
        }
    }
    return false;
}

bool operator!=(const BigInteger& x, const BigInteger& y) {
    return !(x == y);
}

bool operator>=(const BigInteger& x, const BigInteger& y) {
    return !(x < y);
}

bool operator>(const BigInteger& x, const BigInteger& y) {
    return y < x;
}

bool operator<=(const BigInteger& x, const BigInteger& y) {
    return !(x > y);
}

std::pair<BigInteger, BigInteger> BigInteger::div(const BigInteger& x) const {
    BigInteger divRes; // /
    BigInteger num = this->getsetPositiveed(); // %
    BigInteger xsetPositiveed;
    int divBuf;
    size_t divPos;
    if (a.size() > 0 && x.a.size() > 0 && x.a.size() <= a.size()) {
        xsetPositiveed = x.getsetPositiveed();
        divRes.a.resize(a.size() - x.a.size() + 1, 0);
        for (size_t i = a.size() - 1;; --i) {
            divPos = (i + 1) - x.a.size(); 
            BigInteger y = (xsetPositiveed << divPos); 
            divBuf = std::max(static_cast<int>(static_cast<double>(num) / 
            static_cast<double>(y)), 0);
            num -= y.multCpy(divBuf);
            for (; num >= y;) {
                divBuf += 1;
                num -= y;
            }
            divRes.a[divPos] = divBuf;
            if (i < x.a.size()) {
                break;
            }
        }
    }
    num.norm();
    divRes.norm();
    num.setPositivity(isPositive);
    divRes.setPositivity(isPositive == x.isPositive);
    return {divRes, num};
}

BigInteger mult(const BigInteger& x, const BigInteger& y) {
    BigInteger multRes;
    if (x.a.size() == 0 || y.a.size() == 0) {
        return multRes;
    }
    multRes.a.resize(x.a.size() * y.a.size(), 0);
    for (size_t i = 0; i < x.a.size(); ++i) {
        for (size_t j = 0; j < y.a.size(); ++j) {
            multRes.a[i + j] += x.a[i] * y.a[j];
            for (size_t k = i + j; multRes.a[k] >= BigInteger::MEMBER_SIZE; ++k) {
                if (k + 1 == multRes.a.size()) {
                    multRes.a.push_back(0);
                }
                multRes.a[k + 1] += multRes.a[k] / BigInteger::MEMBER_SIZE;
                multRes.a[k] %= BigInteger::MEMBER_SIZE;
            }
        }
    }
    multRes.setPositivity(x.isPositive == y.isPositive);
    multRes.norm();
    return multRes;
}

std::istream& operator>>(std::istream& in, BigInteger& x) {
    std::string s;
    in >> s;
    x = s;
    return in;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& x) {
    out << x.toString();
    return out;
}

BigInteger fastMult(const BigInteger& xIn, const BigInteger& yIn) {
    const static size_t MIN_FAST_MULT_SIZE = 10;
    BigInteger result;
    if (xIn.a.size() == 0 || yIn.a.size() == 0) {
        return result;
    }
    if (xIn.a.size() <= MIN_FAST_MULT_SIZE && yIn.a.size() <= MIN_FAST_MULT_SIZE) {
        return mult(xIn, yIn);
    }
    BigInteger x = xIn;
    BigInteger y = yIn;
    if (!x.isPositive || !y.isPositive) {
        result = fastMult(x.setPositive(), y.setPositive());
        result.setPositivity(xIn.isPositive == yIn.isPositive);
        return result;
    }
    if (x.a.size() != y.a.size()) {
        size_t maxSize = std::max(x.a.size(), y.a.size());
        if (maxSize > x.a.size()) {
            x.a.resize(maxSize, 0);
        }
        if (maxSize > y.a.size()) {
            y.a.resize(maxSize, 0);
        }
    }
    size_t partSize = (x.a.size() + 1) / 2;
    BigInteger a;
    BigInteger b;
    BigInteger c;
    BigInteger d;
    a.a.resize(partSize);
    b.a.resize(partSize);
    c.a.resize(partSize);
    d.a.resize(partSize);
    for (size_t i = 0; i < partSize; ++i) {
        a.a[i] = x.a[i];
        c.a[i] = y.a[i];
    }
    a.norm();
    c.norm();
    for (size_t i = 0; i + partSize < x.a.size(); ++i) {
        b.a[i] = x.a[i + partSize];
        d.a[i] = y.a[i + partSize];
    }
    b.norm();
    d.norm();
    BigInteger ac = fastMult(a, c);
    BigInteger bd = fastMult(b, d);
    result = (bd << (2 * partSize)) + (((fastMult(a + b, c + d) - ac - bd).norm()) << partSize) + ac;
    result.norm();
    return result;
}

BigInteger& BigInteger::operator+=(const BigInteger& x) {
    BigInteger y;
    bool numChanged;
    if (getsetPositiveed() < x.getsetPositiveed()) {
        y = *this;
        operator=(x);
    } else {
        y = x;
    }
    numChanged = true;
    for (size_t i = 0; numChanged && i < a.size(); ++i) {
        if (i < y.a.size()) {
            a[i] += ((isPositive == y.isPositive) ? 1 : -1) * y.a[i];
            numChanged = true;
        }
        if (a[i] < 0) {
            if (i + 1 != a.size()) {
                a[i + 1] -= 1;
                a[i] += MEMBER_SIZE;
            }
            numChanged = true;
        }
        if (a[i] >= MEMBER_SIZE) {
            if (i + 1 == a.size()) {
                a.push_back(0);
            }
            a[i + 1] += a[i] / MEMBER_SIZE;
            a[i] %= MEMBER_SIZE;
            numChanged = true;
        }
    }
    norm();
    return *this;
}

BigInteger fftMult(const BigInteger& a, const BigInteger& b) {
    BigInteger c = a;
    c.fftMult(b);
    return c;
}

class Rational {
protected:
    BigInteger top;
    BigInteger bottom = 1;
    BigInteger& gcd(BigInteger& x, BigInteger& y) {
        if (x == 0) {
            return y;
        }
        y %= x;
        return gcd(y, x);
    }
    void norm() {
        BigInteger cpTop = top.getsetPositiveed();
        BigInteger cpBottom = bottom.getsetPositiveed();
        BigInteger& gcdRes = gcd(cpTop, cpBottom);
        top /= gcdRes;
        bottom /= gcdRes;
    }
public:
    friend bool operator==(const Rational& x, const Rational& y);
    friend bool operator<(const Rational& x, const Rational& y);

    Rational() {}
    Rational(const BigInteger& x) : top(x) {}
    Rational(const Rational& x) : top(x.top), bottom(x.bottom) {}
    Rational(int x) : top(x) {}

    explicit operator double() {
        return static_cast<double>(top) / static_cast<double>(bottom); 
    }

    Rational operator-() const {
        Rational cp = *this;
        cp.top.setPositivity(!top.getPositivity());
        return cp;
    }

    void swap(Rational& x) {
        top.swap(x.top);
        bottom.swap(x.bottom);
    }

    Rational& operator=(const Rational& x) {
        Rational cp = x;
        swap(cp);
        return *this;
    }

    Rational& operator+=(const Rational& x) {
        top *= x.bottom;
        top += bottom * x.top;
        bottom *= x.bottom;
        norm();
        return *this;
    }

    Rational& operator-=(const Rational& x) {
        operator+=(-x);
        return *this;
    }

    Rational& operator*=(const Rational& x) {
        top *= x.top;
        bottom *= x.bottom;
        norm();
        return *this;
    }

    Rational& operator/=(const Rational& x) {
        bottom *= (x.top.getPositivity() ? x.top : -x.top);
        top *= (x.top.getPositivity() ? x.bottom : -x.bottom);
        norm();
        return *this;
    }

    std::string toString() const {
        std::string s = top.toString();
        if (bottom != 1) {
            s += '/' + bottom.toString(); 
        }
        return s;
    }

    std::string asDecimal(size_t precision = 0) const {
        static const int DEC_BASIS = 10;
        std::string s = (top * BigInteger(DEC_BASIS, precision) / bottom).setPositive().toString();
        if (s.size() <= precision) {
            if (s.size() < precision) {
                std::string t(precision - s.size(), '0');
                s = t + s;
            }
            return (top.getPositivity()?"0.":"-0.") + s;        
        } else {
            std::string intPart = s.substr(0, s.size() - precision);
            std::string fractPart = "." + s.substr(s.size() - precision, precision);
            return (top.getPositivity() ? "" : "-") + intPart + (precision > 0 ? fractPart : "");
        }
    }

};

Rational operator+(const Rational& x, const Rational& y) {
    Rational cp = x;
    cp += y;
    return cp;
}

Rational operator-(const Rational& x, const Rational& y) {
    Rational cp = x;
    cp -= y;
    return cp;
}

Rational operator*(const Rational& x, const Rational& y) {
    Rational cp = x;
    cp *= y;
    return cp;
}

Rational operator/(const Rational& x, const Rational& y) {
    Rational cp = x;
    cp /= y;
    return cp;
}

bool operator==(const Rational& x, const Rational& y) {
    return x.top == y.top && x.bottom == y.bottom;
}

bool operator<(const Rational& x, const Rational& y) {
    return x.top * y.bottom < y.top * x.bottom;
}

bool operator!=(const Rational& x, const Rational& y) {
    return !(x == y);
}

bool operator>=(const Rational& x, const Rational& y) {
    return !(x < y);
}

bool operator>(const Rational& x, const Rational& y) {
    return y < x;
}

bool operator<=(const Rational& x, const Rational& y) {
    return !(x > y);
}

std::istream& operator>>(std::istream& in, Rational& x) {
    BigInteger b;
    in >> b;
    x = b;
    return in;
}

std::ostream& operator<<(std::ostream& out, const Rational& x) {
    out << x.toString();
    return out;
}
