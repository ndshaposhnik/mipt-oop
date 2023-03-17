#include <iostream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <cmath>

const double EPS = 1e-4;
const double PI = acos(-1);

bool areDoublesEqual(double a, double b) {
    return fabs(a - b) < EPS;
}

struct Point {
    double x, y;

    Point() : x(0), y(0) {}
    Point(double x, double y) : x(x), y(y) {}
};

struct Vector2D {
    double x, y;

    Vector2D() : x(0), y(0) {}
    Vector2D(double x, double y) : x(x), y(y) {}

    Vector2D(Point p) : x(p.x), y(p.y) {}

    operator Point() {
        return Point(x, y);
    }

    double len() {
        return sqrt(x * x + y * y);
    }

    Vector2D& rotate(double angle) {
        double nx = x;
        double ny = y;
        nx = x * cos(angle) - y * sin(angle);
        ny = x * sin(angle) + y * cos(angle);
        x = nx;
        y = ny;
        return *this;
    }
};

std::istream& operator >> (std::istream& in, Vector2D& x) {
    return in >> x.x >> x.y;
}

std::ostream& operator << (std::ostream& out, const Vector2D& x) {
    return out << '{' << x.x << ", " << x.y << '}';
}

Vector2D operator + (const Vector2D& left, const Vector2D& right) {
    return {left.x + right.x, left.y + right.y};
}

Vector2D operator - (const Vector2D& left, const Vector2D& right) {
    return {left.x - right.x, left.y - right.y};
}

Vector2D operator * (const Vector2D& left, double k) {
    return {left.x * k, left.y * k};
}

double operator * (const Vector2D& left, const Vector2D& right) {
    return left.x * right.x + left.y * right.y;
}

double crossProduct(const Vector2D& left, const Vector2D& right) {
    return left.x * right.y - left.y * right.x;
}

bool operator == (const Vector2D& left, const Vector2D& right) {
    return areDoublesEqual(left.x, right.x) && areDoublesEqual(left.y, right.y);
}

bool operator != (const Vector2D& left, const Vector2D& right) {
    return !(left == right);
};

class Line {
private:
    double A = 0;
    double B = 0;
    double C = 0;
public:
    Line(const Point& left, const Point& right) {
        A = right.y - left.y;
        B = left.x - right.x;
        C = -A * left.x - B * left.y;    
    }

    Line(double k, double b) : A(k), B(-1), C(b) {}
    Line(const Point& p, double k) : A(k), B(-1), C(-A * p.x - B * p.y) {}
    
    Vector2D getNormal() const {
        Vector2D n = {A, B};
        return n * (1 / n.len()); 
    }

    double getDist(Point p) const {
        return (A * p.x + B * p.y + C) / sqrt(A * A + B * B);
    }

    friend bool operator == (const Line& left, const Line& right);
    friend Point intersection(const Line& a, const Line& b);
};

Point intersection(const Line& a, const Line& b) {
    double x = (a.B * b.C - b.B * a.C) / (a.A * b.B - b.A * a.B);
    double y = (b.A * a.C - a.A * b.C) / (a.A * b.B - b.A * a.B);
    return {x, y};
}

bool operator == (const Line& left, const Line& right) {
    return areDoublesEqual(left.A * right.B, left.B * right.A) && 
           areDoublesEqual(left.A * right.C, left.C * right.A);
}

bool operator != (const Line& left, const Line& right) {
    return !(left == right);
}

class Shape {
public:
    virtual double perimeter() const = 0;
    virtual double area() const = 0;
    virtual bool containsPoint(Point point) const = 0;
    virtual Shape& rotate(Point center, double angle) = 0;
    virtual Shape& reflex(Point center) = 0;
    virtual Shape& reflex(const Line& axis) = 0;
    virtual Shape& scale(Point center, double coefficient) = 0;
    virtual bool isCongruentTo(const Shape& another) const = 0;
    virtual bool isSimilarTo(const Shape& another) const = 0;

    virtual bool operator == (const Shape& another) const = 0;

    bool operator != (const Shape& another) const {
        return !(*this == another);
    }
    
    virtual ~Shape() {}
};

class Polygon : public Shape {
private:
    bool areEqual(const Polygon& another) const {
        if (n != static_cast<int>(another.points.size())) {
            return false;
        }
        for (int d : {-1, 1}) {
            for (int shift = 0; shift < n; shift++) {
                bool fl = true;
                for (int i = 0; i < n; i++) {
                    if (points[i] != another.points[(i * d + shift + n) % n]) {
                        fl = false;
                        break;
                    }
                }
                if (fl) {
                    return true;
                }
            }
        }
        return false;
    }

    bool notMirrorSymmetry(const Polygon& another) const {
        if (n != another.n) {
            return false;
        }
        for (int d : {-1, 1}) {
            for (int shift = 0; shift < n; shift++) {
                bool fl = true;
                for (int i = 0; i < n; i++) {
                    Vector2D edge1 = points[(i + 1) % n] - points[i];
                    Vector2D edge2 = points[(i + 2) % n] - points[(i + 1) % n];
                    int an1 = (i * d + shift + n) % n;
                    int an2 = ((i + 1) * d + shift + n) % n;
                    int an3 = ((i + 2) * d + shift + n) % n;
                    Vector2D anotherEdge1 = another.points[an2] - another.points[an1];
                    Vector2D anotherEdge2 = another.points[an3] - another.points[an2];
                    if (!areDoublesEqual(edge1.len(), anotherEdge1.len()) || 
                        !areDoublesEqual(edge2.len(), anotherEdge2.len()) ||
                        !areDoublesEqual(crossProduct(edge1, edge2), 
                                         crossProduct(anotherEdge1, anotherEdge2))) {
                        fl = false;
                        break;
                    }
                }
                if (fl) {
                    return true;
                }
            }
        }
        return false;
    } 
protected:
    int n;
    std::vector<Point> points;
public:
    Polygon() : n(0), points(std::vector<Point>()) {}
    Polygon(std::vector<Point> _points) : n(_points.size()), points(_points) {}
    Polygon(std::initializer_list<Point> _points) : n(_points.size()), points(_points) {}

    int verticesCount() const {
        return n;
    }

    std::vector<Point> getVertices() const {
        return points;
    }

    bool isConvex() const {
        bool lessZero = false;
        bool grZero = false;
        for (int i = 0; i < n; i++) {
            int ii = (i + 1) % n;
            int iii = (i + 2) % n;
            if (crossProduct((points[ii] - points[i]), (points[iii] - points[ii])) < 0) {
                lessZero = true;
            } else {
                grZero = true;
            }
        }
        return lessZero ^ grZero;
    }

    double perimeter() const {
        double res = 0;
        for (int i = 0; i < n - 1; ++i) {
            res += (points[i] - points[i + 1]).len();
        }
        res += (points[n - 1] - points[0]).len();
        return res;
    }
    
    double area() const {
        double res = 0;
        for (int i = 1; i < n - 1; i++) {
            res += crossProduct((points[i] - points[0]), (points[i + 1] - points[0]));
        }
        if (res < 0) {
            res = -res;
        }
        return res / 2;
    }

    bool operator == (const Shape& other) const {
        const Polygon* otherPtr = dynamic_cast<const Polygon*>(&other);
        if (otherPtr == nullptr) {
            return false;
        }
        return areEqual(*otherPtr);
    }

    bool isCongruentTo(const Shape& other) const {
        const Polygon* otherPtr = dynamic_cast<const Polygon*>(&other);
        if (otherPtr == nullptr) {
            return false;
        }
        const Polygon& another = *otherPtr;
        Polygon reflexed = another;
        reflexed.reflex(Line(Point(0, 0), Point(1, 0)));
        return notMirrorSymmetry(another) || notMirrorSymmetry(reflexed);
    }

    bool isSimilarTo(const Shape& other) const {
        const Polygon* otherPtr = dynamic_cast<const Polygon*>(&other);
        if (otherPtr == nullptr) {
            return false;
        }
        Polygon another = *otherPtr;
        return isCongruentTo(another.scale({0, 0}, perimeter() / another.perimeter())); 
    }

    bool containsPoint(Point point) const {
        double angle = 0;
        for (int i = 0; i < n; i++) {
            int ii = (i + 1) % n;
            Vector2D v1 = point - points[i];
            Vector2D v2 = point - points[ii];
            angle += acos(v1 * v2 / v1.len() / v2.len()) * (crossProduct(v1, v2) >= 0 ? 1 : -1);
        }
        return !areDoublesEqual(angle, 0);
    }
    
    Polygon& rotate(Point center, double angle) {
        for (Point& v : points) {
            v = (v - center).rotate(angle) + center;
        }
        return *this;
    }

    Polygon& reflex(Point center) {
        for (Point& v : points) {
            v = center * 2 - v;
        }
        return *this;
    }

    Polygon& reflex(const Line& axis) {
        Point normal = axis.getNormal();
        for (Point& v : points) {
            v = v - normal * 2 * axis.getDist(v);
        }
        return *this;
    }

    Polygon& scale(Point center, double coefficient) {
        for (Point& v : points) {
            v = ((v - center) * coefficient) + center;
        }
        return *this;
    }
    friend std::ostream& operator << (std::ostream& out, const Polygon& pol);
};

std::ostream& operator << (std::ostream& out, const Polygon& pol) {
    out << pol.verticesCount() << " {";
    for (auto p : pol.points) {
        out << p << ", ";
    }
    out << '}';
    return out << '\n';
}

class Ellipse : public Shape {
protected:
    double a = 0, b = 0, c = 0;
    Point f1;
    Point f2;
public:
    Ellipse(Point _f1, Point _f2, double a2) : f1(_f1), f2(_f2) {
        a = a2 / 2;
        c = (f1 - f2).len() / 2;
        b = sqrt(a * a - c * c);
    }

    std::pair<Point, Point> focuses() const {
        return {f1, f2};
    }

    std::pair<Line, Line> directrices() const {
        Point d1 = center() + (f1 - center()) * (a * a / c / c); 
        Point d2 = center() - (f1 - center()) * (a * a / c / c);
        Line axis(f1, f2);
        Line l1(d1, d1 + axis.getNormal());
        Line l2(d2, d2 + axis.getNormal());
        return {l1, l2};
    }

    double eccentricity() const {
        std::cerr << f1 << ' ' << f2 << a << ' ' << b << ' ' << c <<
                    ' ' << sqrt(1 - b * b / a / a) << '\n';
        return sqrt(1 - b * b / a / a);
    }

    Point center() const {
        return (f1 + f2) * 0.5;
    }

    double perimeter() const {
        return PI * (3 * (a + b) - sqrt((3 * a + b) * (a + 3 * b)));
    }
     
    double area() const {
        return PI * a * b;
    }

    bool operator == (const Shape& other) const {
        const Ellipse* otherPtr = dynamic_cast<const Ellipse*>(&other);
        if (otherPtr == nullptr) {
            return false;
        }
        const Ellipse& another = *otherPtr;
        return Point(a, b) == Point(another.a, another.b) && 
            ((f1 == another.f1 && f2 == another.f2) || (f1 == another.f2 && f2 == another.f1));
    }

    bool isCongruentTo(const Shape& other) const {
        const Ellipse* otherPtr = dynamic_cast<const Ellipse*>(&other);
        if (otherPtr == nullptr) {
            return false;
        }
        return Point(a, b) == Point(otherPtr->a, otherPtr->b); 
    }

    bool isSimilarTo(const Shape& other) const {
        const Ellipse* otherPtr = dynamic_cast<const Ellipse*>(&other);
        if (otherPtr == nullptr) {
            return false;
        }
        return areDoublesEqual(eccentricity(), otherPtr->eccentricity());
    }

    bool containsPoint(Point point) const {
        return (point - f1).len() + (point - f2).len() <= 2 * a;
    }

    Ellipse& rotate(Point center, double angle) {
        f1 = (f1 - center).rotate(angle) + center;
        f2 = (f2 - center).rotate(angle) + center;
        return *this;
    }

    Ellipse& reflex(Point center) {
        f1 = center * 2 - f1;
        f2 = center * 2 - f2;
        return *this;
    }

    Ellipse& reflex(const Line& axis) {
        Point normal = axis.getNormal();
        f1 = f1 - normal * 2 * axis.getDist(f1);
        f2 = f2 - normal * 2 * axis.getDist(f2);
        return *this;
    }

    Ellipse& scale(Point center, double coefficient) {
        f1 = ((f1 - center) * coefficient) + center;
        f2 = ((f2 - center) * coefficient) + center;
        a *= coefficient;
        b *= coefficient;
        c *= coefficient;
        return *this;
    }

};

class Circle : public Ellipse {
public:
    Circle(Point center, double radius) : Ellipse(center, center, 2 * radius) {}

    double radius() const {
        return a;
    }
};

std::ostream& operator << (std::ostream& out, const Circle& c) {
    return out << c.center() << ' ' << c.radius();
}

class Rectangle : public Polygon {
public:
    using Polygon::Polygon;

    Rectangle(Point a, Point b, double tg) {
        if (tg < 1) {
            tg = 1 / tg;
        }
        n = 4;
        points.clear();
        points.resize(4);
        points[0] = a;
        points[2] = b;
        points[1] = points[0] + (b - a).rotate(atan(tg)) * cos(atan(tg));
        points[3] = points[0] + points[2] - points[1];
    }

    Point center() const {
        return (points[0] + points[2]) * 0.5;
    }

    std::pair<Line, Line> diagonals() const {
        return {Line(points[0], points[2]), Line(points[1], points[3])};
    }
};

class Square : public Rectangle {
public:
    Square(Point a, Point b) : Rectangle(a, b, 1) {}

    Circle circumscribedCircle() const {
        return Circle((points[0] + points[2]) * 0.5, (points[0] - points[2]).len() / 2);
    }

    Circle inscribedCircle() const {
        return Circle((points[0] + points[2]) * 0.5, (points[0] - points[1]).len() / 2);
    }
};

class Triangle : public Polygon {
public :
    Triangle(Point a, Point b, Point c) {
        n = 3;
        points.clear();
        points.resize(3);
        points[0] = a;
        points[1] = b;
        points[2] = c;
    }

    Circle circumscribedCircle() const {
        Point a = points[0], b = points[1], c = points[2];
        Line ab(a, b);
        Line ac(a, c);
        Line spab = Line((a + b) * 0.5, (a + b) * 0.5 + ab.getNormal());
        Line spac = Line((a + c) * 0.5, (a + c) * 0.5 + ac.getNormal());
        Point center = intersection(spab, spac);
        return Circle(center, (center - a).len());
    }

    Circle inscribedCircle() const {
        Vector2D ab = points[1] - points[0];
        Vector2D ba = points[0] - points[1];
        Vector2D bc = points[2] - points[1];
        Vector2D ac = points[2] - points[0];
        Vector2D bissBAC = ab * ac.len() + ac * ab.len();
        Line bissA(points[0], points[0] + bissBAC);
        Vector2D bissABC = ba * bc.len() + bc * ba.len();
        Line bissB(points[1], points[1] + bissABC);
        Point center = intersection(bissA, bissB); 
        double radius = Line(points[0], points[1]).getDist(center);
        if (radius < 0) {
            radius = -radius;
        }
        return Circle(center, radius);
    }

    Point centroid() const {
        return (points[0] + points[1] + points[2]) * (1.0 / 3.0);
    }

    Point orthocenter() const {
        Point n1 = Line(points[0], points[2]).getNormal();
        Point n2 = Line(points[0], points[1]).getNormal();
        Line h1(points[1], points[1] + n1);
        Line h2(points[2], points[2] + n2);
        return intersection(h1, h2);
    }

    Line EulerLine() const {
        return Line(orthocenter(), centroid());
    }

    Circle ninePointsCircle() const {
        Triangle t((points[0] + points[1]) * 0.5, (points[1] + points[2]) * 0.5, 
                   (points[0] + points[2]) * 0.5);
        return t.circumscribedCircle();
    }
};

