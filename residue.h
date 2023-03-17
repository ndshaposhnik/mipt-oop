#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

template<unsigned N, unsigned ANS, unsigned D>
struct helperMinimalDivisor {
    static const unsigned value = helperMinimalDivisor<N, N % D == 0 ? D : ANS, D - 1>::value; 
};

template<unsigned N, unsigned ANS>
struct helperMinimalDivisor<N, ANS, 1> {
    static const unsigned value = ANS;  
};

template<unsigned N>
struct squareRoot {
    static const unsigned value = static_cast<unsigned>(sqrt(static_cast<double>(N)) + 0.5);
};

template<unsigned N>
struct minimalDivisor {
    static const unsigned value = helperMinimalDivisor<N, N, squareRoot<N>::value>::value;
};

template<unsigned N>
struct is_prime {
    static const bool value = (minimalDivisor<N>::value == N) ? 1 : 0;
};

template<>
struct is_prime<1> {
    static const bool value = false;
};

template<>
struct is_prime<0> {
    static const bool value = false;
};

template<unsigned P, unsigned N> 
struct isPower {
    static const bool value = N % P ? false : isPower<P, N / P>::value;
};

template<unsigned P>
struct isPower<P, 1> {
    static const bool value = true;
};

template<unsigned P> 
struct isPower<P, 0> {
    static const bool value = false;
};

template<unsigned N>
struct isPowerOfPrime {
    static const bool value = minimalDivisor<N>::value % 2 && 
                              isPower<minimalDivisor<N>::value, N>::value;
};

template<unsigned N>
struct has_primitive_root {
    static const bool value = isPowerOfPrime<N>::value || 
                              (N % 2 == 0 && isPowerOfPrime<N / 2>::value);
};

template<>
struct has_primitive_root<2> {
    static const bool value = true;
};

template<>
struct has_primitive_root<4> {
    static const bool value = true;
};

template<>
struct has_primitive_root<0> {
    static const bool value = false;
};

template<>
struct has_primitive_root<1> {
    static const bool value = false;
};

template<unsigned N>
const bool is_prime_v = is_prime<N>::value;

template<unsigned N>
const bool has_primitive_root_v = has_primitive_root<N>::value;

template <unsigned N>
class Residue {
private:
    long long value = 0;

    static long long gcd(long long a, long long b) {
        if (b == 0) {
            return a;
        }
        return gcd(b, a % b);
    }

    static std::vector<unsigned> primeDivisors(unsigned n) {
        std::vector<unsigned> res;
        for (unsigned i = 2; i * i <= n; i++) {
            if (n % i == 0) {
                while (n % i == 0) {
                    n /= i;
                }
                res.push_back(i);
            }
        }
        if (n > 1) {
            res.push_back(n);
        }
        return res;
    }

    static unsigned functionEuler(unsigned n) {
        unsigned ans = n;
        for (auto u : primeDivisors(n)) {
            ans -= ans / u;
        }
        return ans;
    }
public:
    explicit Residue<N>(long long val) : value((val % N + N) % N) {}

    explicit operator int() const {
        return static_cast<int>(value);
    }

    Residue<N>& operator += (const Residue<N>& left) {
        value += left.value;
        value %= N;
        return *this;
    }

    Residue<N>& operator -= (const Residue<N>& left) {
        *this += Residue(-left.value);
        return *this;
    }

    Residue<N>& operator *= (const Residue<N>& left) {
        value *= left.value;
        value %= N;
        return *this;
    }

    Residue<N>& operator /= (const Residue<N>& left) {
        *this *= left.getInverse();
        return *this;
    }

    Residue<N> pow(unsigned p) const {
        long long ans = 1;
        long long now = value;
        while (p) {
            if (p % 2) {
                ans *= now;
                ans %= N;
            }
            p /= 2;
            now *= now;
            now %= N;
        }
        return Residue<N>(ans);
    }

    Residue getInverse() const {
        assert(is_prime_v<N>);
        return pow(N - 2);
    }

    unsigned order() const {
        if (gcd(N, value) != 1) {
            return 0;
        }
        unsigned funcEuler = functionEuler(N);
        unsigned ans = funcEuler;
        for (unsigned i = 1; i * i <= funcEuler; i++) {
            if (funcEuler % i == 0) {
                if (pow(i).value == 1) {
                    ans = std::min(ans, i);
                }
                if (pow(funcEuler / i).value == 1) {
                    ans = std::min(ans, funcEuler / i);
                }
            }
        }
        return ans;
    }

    static Residue<N> getPrimitiveRoot() {
        assert(has_primitive_root_v<N>);
        if (N == 2) {
            return Residue<N>(1);
        }
        unsigned funcEuler = functionEuler(N);
        std::vector <unsigned> pDivisors = primeDivisors(funcEuler);
        unsigned ans = 0;
        for (unsigned tmp = 2; tmp <= N; tmp++) {
            bool flag = Residue<N>(tmp).pow(funcEuler).value == 1;
            for (size_t i = 0; i < pDivisors.size(); ++i) {
                if (Residue<N>(tmp).pow(funcEuler / pDivisors[i]).value == 1) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                ans = tmp;
                break;
            }
        }
        return Residue<N>(ans);
    }
};

template<unsigned N>
Residue<N> operator + (const Residue<N>& left, const Residue<N>& right) {
    Residue<N> ans = left;
    ans += right;
    return ans;
}

template<unsigned N>
Residue<N> operator - (const Residue<N>& left, const Residue<N>& right) {
    Residue<N> ans = left;
    ans -= right;
    return ans;
} 

template<unsigned N>
Residue<N> operator * (const Residue<N>& left, const Residue<N>& right) {
    Residue<N> ans = left;
    ans *= right;
    return ans;
} 

template<unsigned N>
Residue<N> operator / (const Residue<N>& left, const Residue<N>& right) {
    Residue<N> ans = left;
    ans /= right;
    return ans;
} 

