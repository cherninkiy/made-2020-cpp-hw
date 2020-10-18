#pragma once

#include <cmath>
#include <vector>


namespace proj4 {
    using namespace std;

struct Point {
    double x;
    double y;

    constexpr bool operator ==(const Point& other) {
        return this->x == other.x && this->y == other.y;
    }

    constexpr bool operator !=(const Point& other) {
        return !(*this == other);
    }
};

class Line {
    Point left;
    Point right;

public:

    Line(const Point& x1, const Point& x2) {
        if (x1.x <= x2.x && x1.y <= x2.y) {
            left = x1;
            right = x2;
        } else {
            left = x2;
            right = x1;
        }
        auto it = (x1.x <= x2.x && x1.y <= x2.y) 
            ? points_.end() : points_.begin();
        points_.insert(it, x2);
    }

    Line(const Point& x1, double angle) {
        points_.push_back(x1);
        points_.push_back(Point { x1.x + 1,  x1.y + atan(angle) });
    }

    Line(double shift, double angle) {
        points_.push_back(Point { shift, 0.0 });
        points_.push_back(Point { shift + 1, atan(angle) });
    }

    bool operator ==(const Line& other)
};

class Shape {
public:
    virtual double perimeter() = 0;

    virtual double area() = 0;

    virtual bool operator ==(const Shape& another) = 0;
};

class Polygon : Shape {
    vector<Point> vertices_;

public:
    Shape() {
        vertices_ = vector();
    }

    virtual double perimeter() {
        return 0;
    }

    virtual double area() {
        return 0;
    }

    virtual bool operator ==(const Shape& another) {

    }

    size_t verticesCount() {
        return vertices_.size();
    }

    std::vector<Point> getVertices() const {
        return vertices_;
    }
};

}