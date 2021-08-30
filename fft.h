#include <complex>
#include <vector>

namespace fft
{
    const long double PI = acosl(-1);
    using comp = std::complex<double>;
    using Polynom = std::vector<int64_t>;
    using CompPolynom = std::vector<comp>;

    CompPolynom toCompPolynom(const Polynom &p) {
        CompPolynom x(p.size());
        for (size_t i = 0; i < p.size(); ++i) {
            x[i] = p[i];
        }
        return x;
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

    CompPolynom fft(const CompPolynom &p, comp w) {
        if (p.size() == 1) {
            return p;
        }
        size_t s = p.size() / 2;
        CompPolynom a(s);
        CompPolynom b(s);
        CompPolynom ans(p.size());
        for (size_t i = 0; i < p.size(); ++i) {
            if (i % 2 == 0) {
                a[i / 2] = p[i];
            } else {
                b[i / 2] = p[i];
            }
        }; 
        a = fft(a, w * w);
        b = fft(b, w * w);
        comp c = 1;
        for (size_t i = 0; i < s; ++i, c *= w) {
            ans[i] = a[i] + c * b[i];
            ans[i + s] = a[i] - c * b[i];
        }
        return ans;
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
        CompPolynom ca = fft(toCompPolynom(a));
        CompPolynom cb = fft(toCompPolynom(b));
        CompPolynom ans(n);
        for (size_t i = 0; i < n; ++i) {
            ans[i] = ca[i] * cb[i];
        }
        return toPolynom(fftBack(ans));
    } 
}