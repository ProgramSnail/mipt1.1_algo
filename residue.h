#include <cmath>
#include <iostream>
#include <vector>

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
            if (k & (1ull << i)) {
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