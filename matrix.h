#include <fstream>
#include <initializer_list>
#include <iostream>
#include <vector>
#include <string>
#include <complex>

// biginteger.h

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

// residue.h

namespace temphelp {
    template<unsigned N> 
    bool isTrue() {
        bool a[N - 1];
        return a[0] = true;
    } 

    template<unsigned N, unsigned K>
    static const unsigned accurate_sqrt = K * K >= N ? accurate_sqrt<N, K - 1> : (K + 1); 
    template<unsigned N>
    static const unsigned accurate_sqrt<N, 1> = N == 1 ? 1 : 2;

    template<unsigned N, unsigned K>
    static const unsigned near_sqrt = (K >= N || K * K >= N) ? near_sqrt<N, K / 2> : (K * 2); 
    template<unsigned N>
    static const unsigned near_sqrt<N, 1> = N == 1 ? 1 : 2;

    template<unsigned N>
    static const unsigned var_sqrt = N == 1 ? 1 : accurate_sqrt<N, near_sqrt<N, N>>;

    template<unsigned N, unsigned... Args>
    static const unsigned count_of = count_of<Args...> + 1;
    template<unsigned N>
    static const unsigned count_of<N> = 1;

    template<unsigned K, unsigned P>
    static const bool div_checker = (K % P == 0) && div_checker<K / P, P>;
    template<unsigned P>
    static const bool div_checker<1, P> = true;
    template<unsigned P>
    static const bool div_checker<0, P> = false;

    template<unsigned N, unsigned K>
    static const unsigned div_finder = ((var_sqrt<N> - K > 1) && N % (var_sqrt<N> - K) == 0) ? (var_sqrt<N> - K) : div_finder<N, K - 1>;
    template<unsigned N>
    static const unsigned div_finder<N, 0> = N % var_sqrt<N> == 0 ? var_sqrt<N> : N;

    template<unsigned N>
    static const bool is_pow_of_prime = div_checker<N, div_finder<N, var_sqrt<N>>>;

    template<unsigned N, unsigned K>
    static const bool gcd = gcd<K, N % K>;
    template<unsigned N>
    static const bool gcd<N, 0> = N;

    template<unsigned N, unsigned K>
    static const unsigned div_counter_helper = (N % K == 0) + div_counter_helper<N, K - 1>;
    template<unsigned N>
    static const unsigned div_counter_helper<N, 1> = 0;

    template<unsigned N>
    static const unsigned div_counter = div_counter_helper<N, var_sqrt<N>>;

    template<unsigned N, unsigned K>
    static const bool is_prime_helper = (K == N || N % K != 0) && 
        is_prime_helper<N, K - 1>;
    template<unsigned N>
    static const bool is_prime_helper<N, 1> = true;
    template<unsigned N>
    static const bool is_prime_helper<N, 0> = false;
}

template<unsigned N>
struct is_prime {
    static const bool value = N != 1 && temphelp::is_prime_helper<N, 
        static_cast<unsigned>(std::sqrt(N) + 0.5)>;
};

template<unsigned N>
struct has_primitive_root {
    static const bool value = N != 0 && N != 1 && (N == 2 
        || N == 4 || (N % 2 != 0 && temphelp::is_pow_of_prime<N>) 
        || (N % 2 == 0 && (N / 2) % 2 != 0 && temphelp::is_pow_of_prime<N / 2>));
};

template<unsigned N>
static const bool is_prime_v = is_prime<N>::value;

template<unsigned N>
static const bool has_primitive_root_v = has_primitive_root<N>::value;

template<unsigned N>
class Residue {
private:
    uint64_t x = 0;
    static unsigned eulerFuncV;
    static Residue<N> primitiveRoot;
    static const uint64_t MAX_UINT_LOG = 32;
    static unsigned gcd(unsigned a, unsigned b) {
        if (a == 0) {
            return b;
        }
        if (b == 0) {
            return a;
        }
        return gcd(b, a % b);
    }
public:
    static unsigned getEulerFunc() {
        if (eulerFuncV != 0) {
            return eulerFuncV;
        }
        unsigned ans = N - 1;
        bool forward = true;
        std::vector<unsigned> p;
        for (unsigned k = 2; k > 1; forward ? ++k : --k) {
            if (k * k > N) {
                forward = false;
                continue;
            }
            if (N % k == 0) {
                bool b = true;
                unsigned t = forward ? k : N / k;
                for (unsigned v : p) {
                    if (t % v == 0) {
                        b = false;
                        break;
                    }
                }
                if (b) {
                    ans -= (N / t - 1);
                    p.push_back(t);
                }
            }
        }
        return eulerFuncV = ans;
    }

    Residue() = default;

    Residue(int x) : x(x < 0 ? (N - ((-x) % N)) : x % N) {}

    explicit operator int() const {
        return x;
    }

    Residue<N>& powSelf(int) = delete;

    Residue<N>& powSelf(unsigned k) {
        uint64_t t = x;
        x = 1;
        for (uint64_t i = 0; i < MAX_UINT_LOG; ++i, t = (t * t) % N) {
            if (k&  (1ull << i)) {
                x *= t;
                x %= N;
            }
        }
        return *this;
    }

    Residue<N> pow(int) const = delete;

    Residue<N> pow(unsigned k) const {
        Residue<N> a = *this;
        a.powSelf(k);
        return a;
    }

    Residue<N> operator-() const {
        Residue<N> a;
        a.x = N - x;
        return a;
    }

    Residue<N>& operator+=(Residue<N> a) {
        x += a.x;
        x %= N;
        return *this;
    }

    Residue<N>& operator-=(Residue<N> a) {
        x += N - a.x;
        x %= N;
        return *this;
    }

    Residue<N>& operator*=(Residue<N> a) {
        x *= a.x;
        x %= N; 
        return *this;
    }

    Residue<N> getInverse() const {
        temphelp::isTrue<is_prime_v<N>>();
        Residue<N> a = this->pow(N - 2);
        return a;
    }

    Residue<N>& operator/=(Residue<N> a) {
        operator*=(a.getInverse());
        return *this;
    }

    bool operator==(Residue<N> a) const {
        return x == a.x;
    }

    bool operator!=(Residue<N> a) const {
        return !operator==(a);
    }

    static Residue<N> getPrimitiveRoot() {
        temphelp::isTrue<has_primitive_root_v<N>>();
        if (primitiveRoot != 0) {
            return primitiveRoot;
        }
        unsigned phi = getEulerFunc();
        Residue<N> r; 
        std::vector<unsigned> div;
        for (unsigned k = 2; k * k <= phi; ++k) {
            if (phi % k == 0) {
                div.push_back(k);
                if (k * k != phi) {
                    div.push_back(phi / k);
                }
            }         
        }
        for (unsigned k = 1; k < N; ++k) {
            if (phi != 1 && k == 1) {
                continue;
            }
            r = k;
            bool root = (gcd(k, N) == 1);
            for (size_t i = 0; i < div.size(); ++i) {
                if (r.pow(div[i]) == 1) { 
                    root = false;
                    break;
                } 
            }
            if (root) {
                return primitiveRoot = r;
            }
        }
        return 0;
    }

    unsigned order() const {
        if (gcd(x, N) != 1) {
            return 0;
        }
        for (unsigned k = 1; k * k < N; ++k) { 
            if (k * k < N) {
                if (pow(k) == 1) {
                    return k;
                }
            }
        }
        for (unsigned k = sqrt(N) + 1; k > 0; --k) {
            if (k * k < N) {
                if (pow((N - 1) / k) == 1) {
                    return (N - 1) / k;
                }
            }
        }
        return 0;
    }
};

template<unsigned N>
unsigned Residue<N>::eulerFuncV = 0;

template<unsigned N>
Residue<N> Residue<N>::primitiveRoot = 0;

template<unsigned N>
Residue<N> operator+(Residue<N> a, Residue<N> b) {
    a += b;
    return a;
}

template<unsigned N>
Residue<N> operator-(Residue<N> a, Residue<N> b) {
    a -= b;
    return a;
}

template<unsigned N>
Residue<N> operator*(Residue<N> a, Residue<N> b) {
    a *= b;
    return a;
}

template<unsigned N>
Residue<N> operator/(Residue<N> a, Residue<N> b) {
    a /= b;
    return a;
}

namespace temphelp {

    template<unsigned N>
    struct pow_log2 {
        static const unsigned v = pow_log2<N / 2>::v * 2; // ?? why can't be static ??
    };

    template<>
    struct pow_log2<1> {
        static const unsigned v = 1;
    };

    template<>
    struct pow_log2<0> {
        static const unsigned v = 0;
    };

    template<unsigned N>
    static const unsigned pow_log2_v = pow_log2<N>::v;

    template<unsigned N>
    static const unsigned pow_log2_up_v = pow_log2_v<N> * (pow_log2_v<N> < N ? 2 : 1);

    template<unsigned N, unsigned... Args>
    static const unsigned max_v = max_v<Args...> > N ? max_v<Args...> : N;

    template<unsigned N>
    static const unsigned max_v<N> = N;
}


template<unsigned N, unsigned M, typename Field = Rational>
class Matrix {
private:
    using ThisMatrix = Matrix<N, M, Field>;
    using Vec = std::vector<Field>;
    std::vector<Vec> a;

    // matrix result, back matrix (correct for correct matrix), det (correct for correct matrix)
    std::pair<std::pair<ThisMatrix, ThisMatrix>, Field> GaussAlgo() const {
        ThisMatrix cp = *this;
        ThisMatrix backMatrix;
        Field det = 1;
        unsigned t = std::min(N, M);
        for (size_t i = 0; i < t; ++i) {
            backMatrix[i][i] = 1;
        }
        size_t p = 0;
        for (size_t i = 0; i < M && p < N; ++i) {
            for (size_t j = p; j < N; ++j) {
                if (cp.a[j][i] != 0) {
                    if (j != p) {
                        cp.rswap(p, j);
                        backMatrix.rswap(p, j);
                    }
                    break;
                }
            }
            det *= cp.a[p][i];
            if (cp.a[p][i] == 0) {
                continue;
            }
            Field k = Field(1) / cp.a[p][i];
            cp.rmult(p, k);
            backMatrix.rmult(p, k);
            for (size_t j = 0; j < N; ++j) {
                if (j == p) {
                    continue;
                }
                k = cp.a[j][i];
                cp.radd(j, p, k);
                backMatrix.radd(j, p, k);
            }
            ++p;
        }
        return {{cp, backMatrix}, det};
    }
public:
    Matrix() : a(N, Vec(M, 0)) {}
    Matrix(const std::vector<Vec>& b) : a(N) { // check size
        for (size_t i = 0; i < N; ++i) {
            if (i < b.size()) {
                a[i] = b[i];
            }
            a[i].resize(M, 0);
        }
    }
    Matrix(std::initializer_list<Vec> x) : a(N) { // check size
        size_t i = 0;
        for (Vec v : x) {
            a[i] = v;
            a[i].resize(M, 0);
            ++i;
        }
        for (; i < N; ++i) {
            a[i].resize(M, 0);
        }
    }

    void rswap(unsigned x, unsigned y) {
        std::swap(a[x], a[y]);
    }

    void radd(unsigned x, unsigned y, Field k) {
        for (size_t i = 0; i < M; ++i) {
            a[x][i] -= k * a[y][i];
        }
    }

    void rmult(unsigned x, Field k) {
        for (size_t i = 0; i < M; ++i) {
            a[x][i] *= k;
        }
    }

    bool rIsZero(unsigned x) {
        for (size_t i = 0; i < M; ++i) {
            if (a[x][i] != 0) {
                return false;
            }
        }
        return true;
    }

    ThisMatrix& operator=(const ThisMatrix& x) = default;

    bool operator==(const ThisMatrix& x) const {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                if (a[i][j] != x.a[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }

    bool operator!=(const ThisMatrix& x) const {
        return !operator==(x);
    }

    ThisMatrix& operator+=(const ThisMatrix& x) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                a[i][j] += x.a[i][j];
            }
        }
        return *this;
    }

    ThisMatrix& operator-=(const ThisMatrix& x) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                a[i][j] -= x.a[i][j];
            }
        }
        return *this;
    }

    ThisMatrix& operator*=(const Field& x) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                a[i][j] *= x;
            }
        }
        return *this;
    }

    ThisMatrix& operator*=(const ThisMatrix& x) {
        static_assert(N == M, "Matrix: not square");
        operator=(*this * x);
        return *this;
    }

    Field det() const {
        static_assert(N == M, "Matrix: not square");
        return GaussAlgo().second;
    }

    Matrix<M, N, Field> transposed() const {
        Matrix<M, N, Field> result;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                result[j][i] = a[i][j];
            }
        }
        return result;
    }

    template<unsigned SN, unsigned SM>
    Matrix<SN, SM, Field> simpleSubmatrix(size_t k, size_t p) const {
        Matrix<SN, SM, Field> result;
        for (size_t i = 0; i < SN; ++i) {
            for(size_t j = 0; j < SM; ++j) {
                result[i][j] = a[k + i][p + j];
            }
        }
        return result;
    }

    template<unsigned SN, unsigned SM>
    void setSubmatrix(const Matrix<SN, SM, Field>& x, size_t k, size_t p) {
        for (size_t i = 0; i < SN; ++i) {
            for (size_t j = 0; j < SM; ++j) {
                a[k + i][p + j] = x[i][j];
            }
        }
    }

    template<unsigned SN, unsigned SM>
    Matrix<SN, SM, Field> resized() const {
        Matrix<SN, SM, Field> result;
        for (size_t i = 0; i < SN; ++i) {
            for (size_t j = 0; j < SM; ++j) {
                if (i >= N || j >= M) {
                    result[i][j] = 0;
                } else {
                    result[i][j] = a[i][j]; 
                }
            }
        }
        return result;
    }

    unsigned rank() const {
        ThisMatrix x = GaussAlgo().first.first;
        if (N == 0) {
            return 0;
        }
        unsigned ans = 0;
        for (size_t i = N - 1;; --i) {
            if (x.rIsZero(i)) {
                ++ans;
            } else {
                break;
            }
            if (i == 0) {
                break;
            }
        }
        ans = N - ans;
        return ans;
    }

    ThisMatrix inverted() const {
        return GaussAlgo().first.second;
    }

    ThisMatrix& invert() {
        operator=(inverted());
        return *this;
    }

    Field trace() const {
        static_assert(N == M, "Matrix: not square");
        Field ans = 0;
        for (size_t i = 0; i < N; ++i) {
            ans += a[i][i];
        }
        return ans;
    }

    Vec getRow(size_t k) {
        return a[k];
    }

    Vec getColumn(size_t k) {
        Vec b(M, 0);
        for (size_t i = 0; i < M; ++i) {
            b[i] = a[k][i];
        }
        return b;
    }

    Vec& operator[](size_t k) {
        return a[k];
    }

    const Vec& operator[](size_t k) const {
        return a[k];
    }

};

template<unsigned N, typename Field = Rational>
using SquareMatrix = Matrix<N, N, Field>;

template<unsigned N, unsigned M, typename Field = Rational>
Matrix<N, M, Field> operator+(const Matrix<N, M, Field>& x, const Matrix<N, M, Field>& y) {
    Matrix<N, M, Field> cp = x;
    cp += y;
    return cp;
}

template<unsigned N, unsigned M, typename Field = Rational>
Matrix<N, M, Field> operator-(const Matrix<N, M, Field>& x, const Matrix<N, M, Field>& y) {
    Matrix<N, M, Field> cp = x;
    cp -= y;
    return cp;
}

template<unsigned N, unsigned M, typename Field = Rational>
Matrix<N, M, Field> operator*(const Matrix<N, M, Field>& x, const Field& y) {
    Matrix<N, M, Field> cp = x;
    cp *= y;
    return cp;
}

template<unsigned N, unsigned M, typename Field = Rational>
Matrix<N, M, Field> operator*(const Field& y, const Matrix<N, M, Field>& x) {
    Matrix<N, M, Field> cp = x;
    cp *= y;
    return cp;
}

template<unsigned N, unsigned M, unsigned K, typename Field = Rational>
Matrix<N, K, Field> simpleMult(const Matrix<N, M, Field>& x, const Matrix<M, K, Field>&  y) {
    Matrix<N, K, Field> result;
    for (size_t i = 0; i < N; ++i) {
         for (size_t j = 0; j < K; ++j) {
             for (size_t k = 0; k < M; ++k) {
                 result[i][j] += x[i][k] * y[k][j];
             }
         }
    }
    return result;    
}

template<unsigned N, unsigned M, unsigned K, typename Field = Rational>
Matrix<N, K, Field> operator*(const Matrix<N, M, Field>& x, const Matrix<M, K, Field>& y) {
    static const unsigned FAST_MULT_CONST = 60;
    if (std::max(N, std::max(M, K)) < FAST_MULT_CONST) {
        return simpleMult(x, y);
    }
    static const unsigned P = temphelp::max_v<temphelp::pow_log2_up_v<N>, 
        temphelp::pow_log2_up_v<M>, temphelp::pow_log2_up_v<K> >;
    static const unsigned T = P / 2;
    Matrix<P, P, Field> a = x.template resized<P, P>();
    Matrix<P, P, Field> b = y.template resized<P, P>();
    std::vector<std::vector<Matrix<T, T, Field> > > suba(2, 
        std::vector<Matrix<T, T, Field> >(2));
    std::vector<std::vector<Matrix<T, T, Field> > > subb(2, 
        std::vector<Matrix<T, T, Field> >(2));
    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            suba[i][j] = a.template simpleSubmatrix<T, T>(i * T, j * T);
            subb[i][j] = b.template simpleSubmatrix<T, T>(i * T, j * T);
        }
    }
    std::vector<Matrix<T, T, Field> > subr(7);
    subr[0] = (suba[0][0] + suba[1][1]) * (subb[0][0] + subb[1][1]);
    subr[1] = (suba[1][0] + suba[1][1]) * subb[0][0];
    subr[2] = suba[0][0] * (subb[0][1] - subb[1][1]);
    subr[3] = suba[1][1] * (subb[1][0] - subb[0][0]);
    subr[4] = (suba[0][0] + suba[0][1]) * subb[1][1];
    subr[5] = (suba[1][0] - suba[0][0]) * (subb[0][0] + subb[0][1]);
    subr[6] = (suba[0][1] - suba[1][1]) * (subb[1][0] + subb[1][1]);
    Matrix<P, P, Field> result;
    result.template setSubmatrix<T, T>(subr[0] + subr[3] - subr[4] + subr[6], 0, 0);
    result.template setSubmatrix<T, T>(subr[2] + subr[4], 0, T);
    result.template setSubmatrix<T, T>(subr[1] + subr[3], T, 0);
    result.template setSubmatrix<T, T>(subr[0] - subr[1] + subr[2] + subr[5], T, T);
    return result.template resized<N, K>();
}

template<typename Field>
Matrix<1, 1, Field> operator*(const Matrix<1, 1, Field>& x, const Matrix<1, 1, Field>& y) {
    return Matrix<1, 1, Field>({{x[0][0]* y[0][0]}});
}