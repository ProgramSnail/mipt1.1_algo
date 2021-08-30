#include <initializer_list>
#include <math.h>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>

const long double PI = acosl(-1);
const long double PRECISION = 1e-8;

bool same(long double a, long double b) {
    return abs(a - b) < PRECISION;
}

bool different(long double a, long double b) {
    return !same(a, b);
}

class Line;

struct Vector2D {
    long double x, y;

    explicit Vector2D(long double x = 0, long double y = 0) : x(x), y(y) {}

    Vector2D(const Vector2D &v) : x(v.x), y(v.y) {}
    
    void swap(Vector2D &v) {
        std::swap(x, v.x);
        std::swap(y, v.y);
    }

    bool operator==(Vector2D v) const {
        return same(x, v.x) && same(y, v.y);
    }

    bool operator!=(Vector2D v) const {
        return !operator==(v);
    }
    
    long double length() const {
        return sqrt(x * x + y * y);
    }

    // scalar multiplication
    long double operator^(Vector2D v) const { 
        return x * v.x + y * v.y;
    }

    // vector multimplication
    long double operator*(Vector2D v) const { 
        return x * v.y - y * v.x;
    }

    Vector2D getOrtogonal(Vector2D v) const;

    Vector2D& normalize() {
        long double len = length();
        operator/=(len); 
        return *this;
    }

    Vector2D getNormalized() const {
        Vector2D cp = *this;
        cp.normalize();
        return cp;
    }

    Vector2D& operator=(Vector2D v) {
        Vector2D cp = v;
        swap(cp);
        return *this;
    }

    Vector2D operator-() const {
        Vector2D cp = *this;
        cp.x = -x;
        cp.y = -y;
        return cp;
    }

    Vector2D& operator*=(long double d) {
        x *= d;
        y *= d;
        return *this;
    }

    Vector2D& operator/=(long double d) {
        x /= d;
        y /= d;
        return *this;
    }

    Vector2D& operator+=(Vector2D v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    Vector2D& operator-=(Vector2D v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vector2D& reflex(Vector2D v);

    Vector2D& reflex(Line line);

    Vector2D& rotate(long double angle) {
        angle *= PI / 180.0;
        *this = Vector2D(x * cos(angle) - y * sin(angle), x * sin(angle) + y * cos(angle));
        return *this;
    }
    
    Vector2D& rotate(Vector2D v, long double angle) {
        Vector2D x = *this;
        x -= v;
        x.rotate(angle);
        *this = x;
        *this += v;
        return *this;
    }

    Vector2D& scale(Vector2D v, long double angle);

    long double getDistance(Line line) const;
};

Vector2D operator*(Vector2D v, long double d) {
    Vector2D cp = v;
    cp *= d;
    return cp;
}

Vector2D operator*(long double d, Vector2D v) {
    return v * d;
}

Vector2D operator/(Vector2D v, long double d) {
    Vector2D cp = v;
    cp /= d;
    return cp;
}

Vector2D operator+(const Vector2D &a, const Vector2D &b) {
    Vector2D cp = a;
    cp += b;
    return cp;
}

Vector2D operator-(const Vector2D &a, const Vector2D &b) {
    Vector2D cp = a;
    cp -= b;
    return cp;
}

Vector2D& Vector2D::reflex(Vector2D v) {
    operator-=(2 * (*this - v));
    return *this;
}

Vector2D& Vector2D::scale(Vector2D v, long double param) {
    operator=(v + (param * (*this - v)));
    return *this;
}

Vector2D Vector2D::getOrtogonal(Vector2D v) const {
    Vector2D u = v.getNormalized();
    return *this - u * operator^(u);
}

using Point = Vector2D;

class Line { // y = param * x + ymove
protected:
    bool infparam = false; 
    long double param = 0;
    long double move = 0; // y move, if infparam - x move
public:
    friend Point intersect(Line a, Line b);

    Line() = default;
    
    Line(const Point &a, const Point &b) : infparam(a.x - b.x == 0), 
        param(infparam ? 0 : ((a.y - b.y) / (a.x - b.x))), 
        move(infparam ? a.x : (a.y - param * a.x)) {}

    Line(const Point &a, long double param) : param(param), move(a.y - param * a.x) {}

    Line(long double move, long double param) : param(param), move(move) {}

    bool operator==(Line line) const {
        return infparam == line.infparam && 
            same(move, line.move) && same(param, line.param);
    }

    bool operator!=(Line line) const {
        return !operator==(line);
    }

    Point getPoint() const {
        return infparam ? Point(move, 0) : Point(0, move);
    }

    // get normalized vector
    Point getVector() const { 
        return infparam ? Point(0, 1) : Point(1, param).getNormalized(); 
    }

    Point getNormal() const {
        return getVector().rotate(90);
    }
};

Point intersect(Line a, Line b) {
    if (b.infparam) {
        std::swap(a, b);
    }
    if (a.infparam) {
        if (b.infparam) {
            return a.getPoint();
        }
        return Point(a.move, b.param * a.move + b.move);
    }
    if (a.param == b.param) {
        return a.getPoint();
    }
    long double x = (b.move - a.move) / (a.param - b.param);
    long double y = a.param * x + a.move;
    return Point(x, y);
}

Vector2D& Vector2D::reflex(Line line) {
    Vector2D v = line.getVector();
    Vector2D toLine = *this - v * operator^(v);
    operator-=(2 * toLine);
    return *this;
}

long double Vector2D::getDistance(Line line) const {
    Line x(*this, *this + line.getNormal());
    return (intersect(line, x) - *this).length();
}

class Shape {
public:
    virtual long double perimeter() const = 0;

    virtual long double area() const = 0;

    virtual bool operator==(const Shape &s) const = 0;

    bool operator!=(const Shape &s)const {
        return !operator==(s);
    }

    virtual bool isCongruentTo(const Shape &s) const = 0;

    virtual bool isSimilarTo(const Shape &s) const = 0;

    virtual bool containsPoint(Point p) const = 0;

    virtual void rotate(Point p, long double angle) = 0;

    virtual void reflex(Point center) = 0;

    virtual void reflex(Line axis) = 0;

    virtual void scale(Point center, long double param) = 0;

    virtual ~Shape() = default;
};

class Polygon : public Shape {
protected:
    std::vector<Point> v;
public:
    Polygon() = default;

    explicit Polygon(size_t size) : v(size) {}

    Polygon(const std::vector<Point> &a) {
        v = a;
    }

    Polygon(const std::initializer_list<Point> &a) {
        v.reserve(a.size());
        for (Point p : a) {
            v.push_back(p);
        }
    }

    bool operator==(const Shape &s) const override {
        Polygon polygon;
        try {
            polygon = dynamic_cast<const Polygon&>(s);
        } catch(...) {
            return false;
        }
        if (polygon.v.size() != v.size()) {
            return false;
        }
        size_t k;
        for (k = 0; k < v.size(); ++k) {
            if (v[k] == polygon.v[0]) {
                break;
            }
        }
        if (k >= v.size()) {
            return false;
        }
        size_t falseCount = 0;
        for (size_t i = 0; i < v.size(); ++i) {
            if (v[(k + i) % v.size()] != polygon.v[i]) {
                falseCount++;
                break;
            }
        }
        for (size_t i = 0; i < v.size(); ++i) {
            if (v[(k + v.size() - i) % v.size()] != polygon.v[i]) {
                falseCount++;
                break;
            }
        }
        return (falseCount < 2);
    }

    bool isCongruentTo(const Shape &s, bool withConst) const {
        Polygon polygon;
        try {
            polygon = dynamic_cast<const Polygon&>(s);
        } catch(...) {
            return false;
        }
        if (v.size() <= 1) {
            return true;
        }
        if (v.size() == 2){
            return withConst || same((v[0] - v[1]).length(),
                (polygon.v[0] - polygon.v[1]).length());
        }
        long double polyConst = (polygon.v[1] - polygon.v[0]).length();
        for (size_t i = 0; i < v.size(); ++i) {
            bool b = true;
            bool c = true;
            long double bConst = (v[(i + 1) % v.size()] - v[i]).length();
            long double cConst = (v[(i + v.size() - 1) % v.size()] - v[i]).length();
            for (size_t j = 1; j < v.size(); ++j) {
                if (different((v[(i + j) % v.size()] - v[i]).length() * 
                    (withConst ? polyConst : 1), (polygon.v[j] - polygon.v[0]).length() * 
                    (withConst ? bConst : 1)) || different((v[(i + j) % v.size()] - 
                    v[(i + j - 1) % v.size()]).length() * (withConst ? polyConst : 1), 
                    (polygon.v[j] - polygon.v[j - 1]).length() * (withConst ? bConst : 1))) {
                    b = false;
                }
                if (different((v[(i + v.size() - j) % v.size()] - v[i]).length() * 
                    (withConst ? polyConst : 1), (polygon.v[j] - polygon.v[0]).length() * 
                    (withConst ? cConst : 1)) || different((v[(i + v.size() - j) % v.size()] 
                    - v[(i + v.size() + 1 - j) % v.size()]).length() * (withConst ? 
                    polyConst : 1), (polygon.v[j] - polygon.v[j - 1]).length() * 
                    (withConst ? cConst : 1))) {
                    c = false;
                }
                if (!b && !c) {
                    break;
                }
            }
            if (b || c) {
                return true;
            }
        }
        return false;
    }

    bool isCongruentTo(const Shape &s) const override {
        return isCongruentTo(s, false);
    }

    bool isSimilarTo(const Shape &s) const override {
        return isCongruentTo(s, true);
    }

    bool containsPoint(Point p) const override {
        long double sum = 0;
        for (size_t i = 0; i < v.size(); ++i) {
            size_t j = (i + 1) % v.size();
            Point x = v[i] - p;
            Point y = v[j] - p;
            sum += acosl((x ^ y) / (x.length() * y.length()));
        }
        return same(abs(sum), 2 * PI);
    }

    size_t verticesCount() const {
        return v.size();
    }

    std::vector<Point> getVertices() const {
        return v;
    }

    bool isConvex() const {
        bool result;
        if (v.size() <= 3) {
            return true;
        }
        for (size_t i = 0; i < v.size(); ++i) {
            size_t j = (i + 1) % v.size();
            size_t k = (i + 2) % v.size();
            bool x = (v[j] - v[i]) * (v[k] - v[j]) > 0;
            if (i == 0) {
                result = x;
            }
            if (result != x) {
                return false;
            }
        }
        return true;
    }

    long double perimeter() const override {
        long double p = 0;
        if (v.size() <= 1) {
            return 0; 
        }
        for (size_t i = 0; i < v.size(); ++i) {
            size_t j = (i + 1) % v.size();
            p += (v[j] - v[i]).length();
        }
        return p;
    }

    long double area() const override {
        long double s = 0;
        if (v.size() <= 2) {
            return 0;
        }
        for (size_t i = 0; i < v.size(); ++i) {
            size_t j = (i + 1) % v.size();
            s += v[i] * v[j];
        }
        s /= 2.0;
        s = abs(s);
        return s;
    }

    void reflex(Point center) override {
        for (size_t i = 0; i < v.size(); ++i) {
            v[i].reflex(center);
        }
    }

    void reflex(Line axis) override {
        for (size_t i = 0; i < v.size(); ++i) {
            v[i].reflex(axis);
        }
    }

    void rotate(Point center, long double angle) override {
        for (size_t i = 0; i < v.size(); ++i) {
            v[i].rotate(center, angle);
        }
    }

    void scale(Point center, long double param) override {
        for (size_t i = 0; i < v.size(); ++i) {
            v[i].scale(center, param);
        }
    }

};

class Ellipse : public Shape {
protected:
    std::pair<Point, Point> f;
    long double dist = 0;
public: 
    Ellipse() = default;

    Ellipse(Point f1, Point f2, long double dist) : f(f1, f2), dist(dist) {}
    
    std::pair<Point, Point> focuses() const {
        return f;
    }

    std::pair<Line, Line> derectrices() const {
        Point c = center();
        Point v = (f.first - f.second).getNormalized();
        long double a = aParam();
        Point dist = v * ((a * a) / cParam());
        std::pair<Point, Point> d = {c + dist, c - dist};
        v.rotate(90);
        return {Line(d.first, d.first + v), Line(d.second, d.second + v)};
    }

    bool operator==(const Shape &s) const override {
        Ellipse e;
        try {
            e = dynamic_cast<const Ellipse&>(s);
        } catch(...) {
            return false;
        }
        return ((f.first == e.f.second && f.second == e.f.first) || 
            (f.first == e.f.first && f.second == e.f.second)) && dist == e.dist;
    }

    bool isCongruentTo(const Shape &s) const override {
        Ellipse e;
        try {
            e = dynamic_cast<const Ellipse&>(s);
        } catch(...) {
            return false;
        }
        return same(cParam(), e.cParam()) && same(dist, e.dist);
        return false;
    }

    bool isSimilarTo(const Shape &s) const override {
        Ellipse e;
        try {
            e = dynamic_cast<const Ellipse&>(s);
        } catch(...) {
            return false;
        }
        return same(cParam() * e.dist, e.cParam() * dist);
        return false;
    }

    bool containsPoint(Point p) const override {
        return (f.first - p).length() + (f.second - p).length() < dist;
    }
    
    long double aParam() const {
        return dist / 2;
    }

    long double cParam() const {
        return (f.second - f.first).length() / 2;
    }

    long double bParam() const {
        return sqrt(aParam() * aParam() - cParam() * cParam());
    }

    long double eccentricity() const {
        if (dist == 0)
        {
            return 0;
        }
        return cParam() / aParam();
    }

    Point center() const {
        return (f.first + f.second) / 2;
    }

    long double perimeter() const override {
        long double a = aParam();
        long double b = bParam();
        long double h = ((a - b) * (a - b)) / ((a + b) * (a + b));
        return PI * (a + b) * (1 + (3 * h) / (10 + sqrt(4 - 3 * h)));
    }

    long double area() const override {
        return PI * aParam() * bParam();
    }

    void reflex(Point center) override {
        f.first.reflex(center);
        f.second.reflex(center);
    }

    void reflex(Line axis) override {
        f.first.reflex(axis);
        f.second.reflex(axis);
    }
    
    void rotate(Point center, long double angle) override {
        f.first.rotate(center, angle);
        f.second.rotate(center, angle);
    }

    void scale(Point center, long double param) override {
        f.first.scale(center, param);
        f.second.scale(center, param);
        dist *= abs(param);
    }
};

class Circle : public Ellipse {
public:
    Circle() = default;

    Circle(Point center, long double r) : Ellipse(center, center, 2 * r) {}

    long double radius() const {
        return dist / 2;
    }
    
    long double perimeter() const override {
        return 2 * PI * radius();
    }

    long double area() const override {
        long double r = radius();
        return PI * r * r;
    }
};

class Rectangle : public Polygon {
public:
    Rectangle() : Polygon(4) {}

    Rectangle(Point x, Point y, long double ratio = 1) : 
        Polygon({x, Point(), y, Point()}) {
        float angle = atan(ratio);
        Point p = (y - x);
        p.rotate(angle);
        p /= sqrt(1 + ratio * ratio);
        v[1] = x + p;
        v[3] = y - p;
    }
    
    Point center() const {
        return (v[0] + v[2]) / 2;
    }

    std::pair<Line, Line> diagonals() const {
        return {Line(v[0], v[2]), Line(v[1], v[3])};
    }

    std::pair<long double, long double> sides() const {
        return {(v[1] - v[0]).length(), (v[2] - v[1]).length()};
    }

    long double perimeter() const override {
        auto x = sides();
        return 2 * (x.first + x.second);
    }

    long double area() const override {
        auto x = sides();
        return x.first * x.second;
    }
};

class Square : public Rectangle {
public:
    Square() = default;

    Square(Point x, Point y) : Rectangle(x, y) {}

    Circle circumscribedCircle() const {
        Point c = center();
        return Circle(c, (v[0] - c).length());
    }

    Circle inscribedCircle() const {
        Point c = center();
        return Circle(center(), (v[0] - c).length() / sqrt(2));
    }
};

class Triangle : public Polygon {
public:
    Triangle() : Polygon(3) {}

    Triangle(Point a, Point b, Point c) : Polygon({a, b, c}) {}

    Point orthocenter() const {
        Point x = Line(v[2], v[1]).getNormal();
        Point y = Line(v[2], v[0]).getNormal();
        return intersect(Line(v[0], v[0] + x), Line(v[1], v[1] + y));
    }

    Circle inscribedCircle() const {
        Line lx(v[0], v[0] + (v[1] - v[0]).normalize() + (v[2] - v[0]).normalize());
        Line ly(v[1], v[1] + (v[0] - v[1]).normalize() + (v[2] - v[1]).normalize());
        Point c = intersect(lx, ly);
        Line line(v[1], v[2]);
        return Circle(c, c.getDistance(line));
    }

    Circle circumscribedCircle() const {
        Point x = (v[0] + v[1]) / 2;
        Point y = (v[0] + v[2]) / 2;
        Point z = (v[1] + v[2]) / 2;
        Point c = Triangle(x, y, z).orthocenter();
        return Circle(c, (c - v[0]).length());
    }

    Circle ninePointsCircle() const {
        Circle circle = circumscribedCircle();
        Point c = (circle.center() + orthocenter()) / 2;
        return Circle(c, circle.radius() / 2);
    }

    Point centroid() const {
        return (v[0] + v[1] + v[2]) / 3;
    }

    Line EulerLine() const {
        return Line(centroid(), orthocenter());
    }
}; 