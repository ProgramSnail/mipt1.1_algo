#include <complex>
#include <vector>

namespace fft
{
    using comp = std::complex<double>;
    using Polynom = std::vector<int>;
    using CompPolynom = std::vector<comp>;

    const long double PI = acosl(-1);
    const comp cI = comp(0, 1); 

    comp& opposite(comp &c) {
        c.imag(-c.imag());
        return c;
    }

    comp getOpposite(comp c) {
        opposite(c);
        return c;
    }

    CompPolynom toCompPolynom(const Polynom &p) {
        CompPolynom x(p.size());
        for (size_t i = 0; i < p.size(); ++i) {
            x[i] = p[i];
        }
        return x;
    }

    CompPolynom toCompPolynom(const Polynom &p, const Polynom &q) {
        CompPolynom x(p.size());
        for (size_t i = 0; i < p.size(); ++i) {
            x[i] = static_cast<comp>(p[i]) + cI * static_cast<double>(q[i]);
        } 
        return x;
    }

    std::pair<CompPolynom, CompPolynom> toCompPolynoms(const CompPolynom &p) {
        size_t n = p.size();
        CompPolynom x(n, 0);
        CompPolynom y(n, 0);
        for (size_t i = 0; i < n; ++i) {
            x[i] = (p[i] + getOpposite(p[(n - i) % n])) / 2.0;
            y[i] = (p[i] - getOpposite(p[(n - i) % n])) / (2.0 * cI);
        }
        return {x, y};
    }

    Polynom toPolynom(const CompPolynom &p) {
        Polynom x(p.size());
        for (size_t i = 0; i < p.size(); ++i) {
            x[i] = static_cast<int>(p[i].real() + 0.5);
        }
        return x;
    }

    comp getKCompRoot(size_t n, int k) {
        double angle = (2 * PI * k) / n;
        return comp(std::cos(angle), std::sin(angle));
    }

    size_t backBForm(size_t n, size_t num) {
        size_t ans = 0;
        for (size_t i = 0; i < num; ++i) {
            if (n & (1 << i)) {
                ans += (1 << (num - i - 1));
            }
        }
        return ans;
    }

    CompPolynom fft(CompPolynom p, comp w) {
        if (p.size() == 1) {
            return p;
        }
        size_t num = 0;
        for (size_t i = p.size(); i > 1; i /= 2, ++num) {}
        std::vector<comp> a(num, 0);
        a[0] = w;
        for (size_t i = 1; i < a.size(); ++i) {
            a[i] = a[i - 1] * a[i - 1];
        }
        for (size_t i = 0; i < p.size(); ++i) {
            size_t j = backBForm(i, num);
            if (i < j) {
                swap(p[i], p[j]);
            }
        }
        size_t t = 1;
        for (size_t part = 2; part <= p.size(); part *= 2, ++t) {
            size_t partHalf = part / 2;
            for (size_t j = 0; j < p.size(); j += part) {
                comp c = 1;
                size_t i = j;
                size_t k = j + partHalf;
                for (; i < j + partHalf; ++i, ++k, c *= a[num - t]) {
                    comp ac = p[i];
                    comp bc = c * p[k];
                    p[i] = ac + bc;
                    p[k] = ac - bc;
                }
            }
        }
        return p;
    }

    CompPolynom fft(const CompPolynom &p) {
        return fft(p, getKCompRoot(p.size(), 1));
    }

    CompPolynom fftBack(const CompPolynom &p) {
        CompPolynom x = fft(p, getKCompRoot(p.size(), -1));
        for (size_t i = 0; i < x.size(); ++i) {
            x[i] /= p.size();
        }
        return x;
    } 

    Polynom mult(Polynom a, Polynom b) {
        size_t n;
        for (n = 1; n < a.size() || n < b.size(); n *= 2) {}
        n *= 2;
        a.resize(n, 0);
        b.resize(n, 0);
        CompPolynom c = fft(toCompPolynom(a, b));
        std::pair<CompPolynom, CompPolynom> d = toCompPolynoms(c);
        CompPolynom ans(n);
        for (size_t i = 0; i < n; ++i) {
            ans[i] = d.first[i] * d.second[i];
        }
        return toPolynom(fftBack(ans));
    } 
}