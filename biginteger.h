#include <iostream>
#include <cassert>
#include <vector>
#include <string>

class BigInteger {
private:
    static const int base = 10;

    bool isNegative = false;
    std::vector<int> digits;

    BigInteger(bool isNegative, std::vector<int> digits) : 
        isNegative(isNegative), digits(digits) {}

    void toNormal() {
        for (size_t i = 1; i < digits.size(); i++) {
            digits[i] += digits[i - 1] / base;
            digits[i - 1] %= base;
        }
        if (digits.empty()) {
            digits.push_back(0);
            isNegative = false;
            return;
        }
        for (int i = 0; i < static_cast<int>(digits.size()) - 1; i++) {
            while (digits[i] < 0) {
                digits[i] += base;
                digits[i + 1]--;
            }
        }
        while (digits.back() >= base) {
            digits.push_back(digits.back() / base);
            digits[digits.size() - 2] %= base;
        }
        while (digits.size() > 1 && digits.back() == 0) {
            digits.pop_back();
        }
        if (digits.size() == 1 && digits.back() == 0) {
            isNegative = false;
        }
    }

    BigInteger shiftRight(int n, int k) const {
        BigInteger res = *this;
        res.digits.insert(res.digits.begin(), n, 0);
        for (int &i : res.digits) {
            i *= k;
        }
        res.toNormal();
        return res;
    }

    std::vector<int> sum(std::vector<int> left, std::vector<int> right, int sign = 1) {
        if (right.size() > left.size()) {
            left.resize(right.size());
        } else {
            right.resize(left.size());
        }
        for (size_t i = 0; i < left.size(); i++) {
            left[i] += sign * right[i];
        }
        return left;
    }

public:
    BigInteger() : isNegative(false), digits({0}) {}

    BigInteger(long long x) {
        if (x == 0) {
            digits.push_back(0);
            return;
        }
        if (x < 0) {
            isNegative = true;
            x = -x;
        }
        while (x > 0) {
            digits.push_back(x % base);
            x /= base;
        }
    }

    BigInteger(std::string s) {
        isNegative = false;
        if (s[0] == '-') {
            isNegative = true;
            s = s.substr(1);
        }
        for (int i = static_cast<int>(s.size()) - 1; i >= 0; i--) {
            digits.push_back(s[i] - '0');
        }
        toNormal();
    }

    BigInteger operator - () const;
    explicit operator bool() const;

    BigInteger abs() const {
        return !isNegative ? *this : -(*this);
    }

    int toInt() const {
        int res = 0;
        int pow = 1;
        for (size_t i = 0; i < digits.size(); ++i) {
            res += digits[i] * pow;
            pow *= base;
        }
        if (isNegative) {
            res = -res;
        }
        return res;
    }

    double toDouble() const {
        double res = 0;
        double pow = 1;
        for (size_t i = 0; i < digits.size(); ++i) {
            res += digits[i] * pow;
            pow *= base;   
        }
        if (isNegative) {
            res = -res;
        }
        return res;
    }

    BigInteger& operator += (const BigInteger& right);

    BigInteger& operator -= (const BigInteger& right) {
        return *this += (-right);
    }

    BigInteger& operator *= (const BigInteger& right);
    BigInteger& operator /= (const BigInteger& right);

    BigInteger& operator %= (const BigInteger&);

    BigInteger& operator ++ () {
        *this += 1;
        return *this;
    }

    BigInteger& operator -- () {
        *this -= 1;
        return *this;
    }

    BigInteger operator ++ (int) {
        BigInteger temp = *this; 
        ++(*this);
        return temp;
    }

    BigInteger operator -- (int) {
        BigInteger temp = *this;
        --(*this);
        return temp;
    }

    std::string toString() const {
        std::string res;
        if (isNegative) {
            res.push_back('-');
        }
        for (int i = static_cast<int>(digits.size()) - 1; i >= 0; i--) {
            res.push_back(digits[i] + '0');
        }
        return res;
    }
    
    friend bool operator <  (const BigInteger& left, const BigInteger& right);

    friend std::istream& operator >> (std::istream& in, BigInteger& bi);

    void roundLastDig() {
        *this += (!isNegative ? 5 : -5);
        *this /= 10;
    }
};

bool operator < (const BigInteger& left, const BigInteger& right) {
    if (left.isNegative && !right.isNegative) {
        return true;
    }
    if (!left.isNegative && right.isNegative) {
        return false;
    }
    if (left.digits.size() < right.digits.size()) {
        return (true ^ left.isNegative);
    }
    if (left.digits.size() > right.digits.size()) {
        return (false ^ left.isNegative);
    }
    for (int i = static_cast<int>(left.digits.size()) - 1; i >= 0; i--) {
        if (left.digits[i] != right.digits[i]) {
            return (left.isNegative ^ (left.digits[i] < right.digits[i]));
        }
    }
    return false;
}

bool operator > (const BigInteger& left, const BigInteger& right) {
    return right < left;
}

bool operator == (const BigInteger& left, const BigInteger& right) {
    return !(left < right) && !(right < left);
}

bool operator != (const BigInteger& left, const BigInteger& right) {
    return !(left == right);
}

bool operator <= (const BigInteger& left, const BigInteger& right) {
    return !(left > right); 
}

bool operator >= (const BigInteger& left, const BigInteger& right) {
    return !(left < right); 
}

BigInteger BigInteger::operator - () const {
    if (*this == BigInteger(0)) {
        return *this;
    }
    return BigInteger(!isNegative, digits);
}

BigInteger& BigInteger::operator += (const BigInteger& right) {
    if (isNegative == right.isNegative) {
        digits = sum(digits, right.digits);
    } else if (abs() > right.abs()) {
        digits = sum(digits, right.digits, -1);
    } else {
        digits = sum(right.digits, digits, -1);
        isNegative = !isNegative;
    }
    toNormal();
    return *this;
}

BigInteger operator + (const BigInteger& left, const BigInteger& right) {
    BigInteger res = left;
    res += right;
    return res;
}

BigInteger operator - (const BigInteger& left, const BigInteger& right) {
    BigInteger res = left;
    res -= right;
    return res;
}

BigInteger operator * (const BigInteger& left, const BigInteger& right) {
    BigInteger res = left;
    res *= right;
    return res;
}

BigInteger operator / (const BigInteger& left, const BigInteger& right) {
    BigInteger res = left;
    res /= right;
    return res;
}

BigInteger operator % (const BigInteger& left, const BigInteger& right) {
    BigInteger res = left;
    res %= right;
    return res;
}

std::istream& operator >> (std::istream& in, BigInteger& bi) {
    bi.digits.clear();
    std::string s;
    in >> s;
    bi = BigInteger(s);
    return in;
}

std::ostream& operator << (std::ostream& out, const BigInteger& bi) {
    out << bi.toString();
    return out;
}

BigInteger& BigInteger::operator *= (const BigInteger& right) {
/*
    std::vector<int> res(digits.size() + right.digits.size());
    for (size_t i = 0; i < digits.size(); i++) {
        for (size_t j = 0; j < right.digits.size(); j++) {
            res[i + j] += digits[i] * right.digits[j];
        }
    }
    isNegative ^= right.isNegative;
    digits = res;
    toNormal();
    return *this;
*/
    if (digits.size() < 2 && right.digits.size() < 2) {
        *this = BigInteger(toInt() * right.toInt());
        return *this;
    }
    int n = (std::max(digits.size(), right.digits.size()) + 1) / 2;
    BigInteger A0(false, std::vector<int>(n));
    BigInteger A1(false, std::vector<int>(n));
    BigInteger B0(false, std::vector<int>(n));
    BigInteger B1(false, std::vector<int>(n));
    for (int i = 0; i < n; i++) {
        A1.digits[i] = (n + i < static_cast<int>(digits.size())) ? digits[n + i] : 0;
        A0.digits[i] = (i < static_cast<int>(digits.size())) ? digits[i] : 0;
        B1.digits[i] = (n + i < static_cast<int>(right.digits.size())) ? right.digits[n + i] : 0;
        B0.digits[i] = (i < static_cast<int>(right.digits.size())) ? right.digits[i] : 0;
    }
    BigInteger C0 = A0 * B0;
    BigInteger C1 = A1 * B1;
    BigInteger C2 = (A0 + A1) * (B0 + B1) - C0 - C1;
    BigInteger res = C1.shiftRight(2 * n, 1) + C2.shiftRight(n, 1) + C0;
    if (isNegative ^ right.isNegative) {
        res = -res;
    }
    *this = res;
    toNormal();
    return *this;
}

BigInteger& BigInteger::operator /= (const BigInteger& right) {
    if (abs() < right.abs()) {
        *this = 0;
        return *this;
    }
    BigInteger a = abs();
    BigInteger b = right.abs();
    isNegative ^= right.isNegative;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; i--) {
        int j = 0;
        while (b.shiftRight(i, j) <= a && j <= 9) {
            j++;
        }
        j--;
        digits[i] = j;
        a -= b.shiftRight(i, j);
        
    }
    toNormal();
    return *this;
}

BigInteger& BigInteger::operator %= (const BigInteger& right) {
    return *this -= right * (*this / right);
}

BigInteger::operator bool() const {
    return (*this != BigInteger(0));
}

class Rational {
private:
    BigInteger nom;
    BigInteger denom;

    static BigInteger gcd(BigInteger a, BigInteger b) {
        if (a < 0) {
            a = -a;
        }
        if (b < 0) {
            b = -b;
        }
        while (a > 0) {
            b %= a;
            std::swap(a, b);
        }
        return b;
    }

    void reduce() {
        if (nom == 0) {
            denom = 1;
            return;
        }
        if (denom < 0) {
            nom = -nom;
            denom = -denom;
        }
        BigInteger div = gcd(nom, denom);
        nom /= div;
        denom /= div;
    }
   
public:
    Rational() : nom(0), denom(1) {}

    Rational(int n) : nom(n), denom(1) {}

    Rational(BigInteger n) : nom(n), denom(1) {}

    Rational(BigInteger n, BigInteger d) {
        nom = n;
        denom = d;
        reduce();
    }
 
    Rational& operator += (const Rational& right) {
        nom = nom * right.denom + denom * right.nom;
        denom *= right.denom;
        reduce();
        return *this;
    }
    
    Rational& operator -= (const Rational& right) {
        *this += (-right);
        return *this;
    }

    Rational& operator *= (const Rational& right) {
        nom *= right.nom;
        denom *= right.denom;
        reduce();
        return *this;
    }

    Rational& operator /= (const Rational& right) {
        nom *= right.denom;
        denom *= right.nom;
        reduce();
        return *this;
    }

    Rational operator - () const {
        Rational ans(-nom, denom);
        return ans;
    }

    friend bool operator <  (const Rational& left, const Rational& right);

    std::string toString() const {
        std::string res;
        res += nom.toString();
        if (denom != 1) {
            res += '/' + denom.toString();
        }
        return res;
    }

    std::string asDecimal(size_t precision = 0) const {
        std::string sign;
        std::string res;
        BigInteger ans(1);
        for (size_t i = 0; i < precision + 1; i++) {
            ans *= 10;
        }
        ans *= nom;
        if (ans < 0) {
            ans = -ans;
            sign = "-";
        }
        ans /= denom;     
        ans.roundLastDig();
        res = ans.toString(); 
        if (res.size() <= precision) {
            res.insert(res.begin(), precision - res.size() + 1, '0');
        }
        if (precision != 0) {
            res.insert(res.end() - precision, 1, '.');
        }
        return sign + res;  
    }

    explicit operator double() {
        return (nom.toDouble() / denom.toDouble());
    }
};

std::ostream& operator << (std::ostream& out, const Rational& r) {
    out << r.toString();
    return out;
}

Rational operator + (const Rational& left, const Rational& right) {
    Rational res = left;
    res += right;
    return res;
}

Rational operator - (const Rational& left, const Rational& right) {
    Rational res = left;
    res -= right;
    return res;
}

Rational operator * (const Rational& left, const Rational& right) {
    Rational res = left;
    res *= right;
    return res;
}

Rational operator / (const Rational& left, const Rational& right) {
    Rational res = left;
    res /= right;
    return res;
}

bool operator < (const Rational& left, const Rational& right) {
    return ((left.nom * right.denom) < (left.denom * right.nom));
}

bool operator > (const Rational& left, const Rational& right) {
    return (right < left);
}

bool operator <= (const Rational& left, const Rational& right) {
    return !(left > right);
}

bool operator >= (const Rational& left, const Rational& right) {
    return (right <= left);
}

bool operator == (const Rational& left, const Rational& right) {
    return !(left < right) && !(right < left);
}

bool operator != (const Rational& left, const Rational& right) {
    return !(left == right); 
}

