#pragma once

#include <cmath>
#include <vector>
#include <numeric>
#include <algorithm>

namespace proj4 {

// Forward declarations
struct Point;
struct Segment;

class Line;
class Shape;
class Ellipse;
class Circle;
class Polygon;
class Triangle;
class Rectangle;
class Square;

namespace utils {

constexpr double PI = 3.14159265;

constexpr double degrees(double radians) { return radians * 180 / utils::PI; }

constexpr double radians(double degrees) { return degrees * utils::PI / 180.0; }

// Equality approximation epsilon
constexpr double EPS = 1e-6;

// Equality approximation
constexpr bool equals(double a, double b, double eps = EPS) {
  return (a - b <= eps) && (b - a <= eps);
}

// Pairwise equality approximation
constexpr bool equals(double a, double b, double c, double d,
                      double eps = EPS) {
  return equals(a, b, EPS) && equals(c, d, EPS);
}
}

struct Point {
  double x;
  double y;

  constexpr Point(double x, double y) : x(x), y(y) {}

  constexpr Point(int x, int y) : x(0.0 + x), y(0.0 + y) {}

  constexpr Point operator-() const { return Point{-x, -y}; }

  constexpr bool operator==(const Point& other) const {
    return utils::equals(x, other.x, y, other.y);
  }

  constexpr bool operator!=(const Point& other) const {
    return !(*this == other);
  }
};

struct Segment {
  Point first;
  Point second;

  constexpr Segment(Point first, Point second) : first(first), second(second) {}

  constexpr Segment(Point pt, double angle, double length)
      : first(pt), second(pt) {
    double pi = utils::PI;

    if (utils::equals(angle, pi / 2)) {
      second.y += length;
    } else if (utils::equals(angle, -pi / 2.0)) {
      second.y -= length;
    } else {
      double kx = ((-pi / 2 < angle) && (angle < pi / 2.0)) ? 1.0 : -1.0;
      double ky = ((0 < angle) && (angle < pi)) ? 1.0 : -1.0;
      double gradient = std::tan(angle);
      second.x += kx * length / std::sqrt(1 + gradient * gradient);
      second.y += ky * length * gradient / std::sqrt(1 + gradient * gradient);
    }
  }

  constexpr double length() const {
    if (first == second) {
      return 0.0;
    }
    return sqrt((first.x - second.x) * (first.x - second.x) +
                (first.y - second.y) * (first.y - second.y));
  }

  constexpr double angle() const {
    if (utils::equals(first.x, second.x)) {
      return (first.y > second.y) ? -utils::PI / 2.0 : utils::PI / 2.0;
    }
    double angle = std::atan((second.y - first.y) / (second.x - first.x));
    return (first.x > second.x) ? utils::PI + angle : angle;
  }

  constexpr Point center() const {
    return Point{(first.x + second.x) / 2.0, (first.y + second.y) / 2.0};
  }
};

class Line {
  Point first_;
  Point second_;

 public:
  constexpr Line(Point first, Point second) : first_(first), second_(second) {}

  constexpr Line(Point pt, double gradient)
      : Line(pt, Point{pt.x + 1.0, pt.y + gradient}) {}

  constexpr Line(double shift, double gradient)
      : Line(Point{shift, 0.0}, Point{shift + 1.0, gradient}) {}

  constexpr Line NormalLine(Point pt) const {
    double dx = second_.x - first_.x;
    double dy = second_.y - first_.y;
    return Line(Point{-dy - pt.x, dx - pt.y}, Point{dy - pt.x, -dx - pt.y});
  }

  constexpr bool contains(Point pt) const {
    double t = (first_.y - second_.y) * pt.x + (second_.x - first_.x) * pt.y +
               (first_.x * second_.y - second_.x * first_.y);
    return utils::equals(t, 0);
  }

  constexpr Point reflex(Point pt) const {
    double dx = (second_.x - first_.x);
    double dy = (second_.y - first_.y);

    if (dx == 0.0) {
      return Point{-pt.x, pt.y};
    }
    if (dy == 0.0) {
      return Point{pt.x, -pt.y};
    }

    double x0 =
        (first_.x * dy * dy + pt.x * dx * dx + dx * dy * (pt.y - first_.y)) /
        (dx * dx + dy * dy);
    double y0 = dx * (pt.x - x0) / dy + pt.y;

    return Point{2.0 * x0 - pt.x, 2.0 * y0 - pt.y};
  }

  constexpr bool operator==(const Line& other) const {
    return contains(other.first_) && contains(other.second_);
  }

  constexpr bool operator!=(const Line& other) const {
    return !(*this == other);
  }
};

// Vertices transformations
namespace transformations {
void translate(std::vector<Point>& vertices, double dx, double dy) {
  std::for_each(vertices.begin(), vertices.end(), [dx, dy](Point& pt) {
    pt = Point{pt.x + dx, pt.y + dy};
  });
}

void rotate(std::vector<Point>& vertices, Point center, double degrees) {
  double angle_sin = sin(utils::radians(degrees));
  double angle_cos = cos(utils::radians(degrees));

  translate(vertices, -center.x, -center.y);

  std::for_each(vertices.begin(), vertices.end(),
      [angle_sin, angle_cos](Point& pt) {
        double x = pt.x * angle_cos - pt.y * angle_sin;
        double y = pt.x * angle_sin + pt.y * angle_cos;
        pt = Point{x, y};
      });

  translate(vertices, center.x, center.y);
}

void scale(std::vector<Point>& vertices, Point center, double coefficient) {
  translate(vertices, -center.x, -center.y);

  std::for_each(vertices.begin(), vertices.end(), 
      [coefficient](Point& pt) {
        double x = pt.x * coefficient;
        double y = pt.y * coefficient;
        pt = Point{x, y};
      });

  translate(vertices, center.x, center.y);
}

void reflex(std::vector<Point>& vertices, const Line& axis) {
  std::for_each(vertices.begin(), vertices.end(),
      [&axis](Point& pt) { pt = axis.reflex(pt); });
}
}

class Shape {
 public:
  virtual double perimeter() const = 0;

  virtual double area() const = 0;

  virtual void translate(double dx, double dy) = 0;

  virtual void translate(Point offset) = 0;

  virtual void rotate(Point center, double angle) = 0;

  virtual void scale(Point center, double coefficient) = 0;

  virtual void reflex(Point center) = 0;

  virtual void reflex(Line axis) = 0;

  virtual bool operator==(const Shape& another) const = 0;

  virtual bool operator!=(const Shape& another) const = 0;
};

class Ellipse : public Shape {
 protected:
  Segment focusSegment_;
  double majorAxis_;
  double minorAxis_;
  double eccentricity_;

 public:
  Ellipse(Point focus1, Point focus2, double majorAxis)
      : focusSegment_(focus1, focus2), majorAxis_(majorAxis / 2.0) {
    eccentricity_ = focusSegment_.length() / (2.0 * majorAxis_);
    minorAxis_ = majorAxis_ * std::sqrt(1 - eccentricity_ * eccentricity_);
  }

  constexpr std::pair<Point, Point> focuses() const {
    return std::make_pair(focusSegment_.first, focusSegment_.second);
  }

  constexpr std::pair<double, double> axis() const {
    return std::make_pair(majorAxis_, minorAxis_);
  }

  constexpr Point center() const { return focusSegment_.center(); }

  constexpr double eccentricity() const { return eccentricity_; }

  constexpr double perimeter() const override {
    // double p = 4 * majorAxis_ * std::comp_ellint_2(eccentricity_);

    double a = majorAxis_;
    double b = minorAxis_;
    double ab2 = (a - b) * (a - b) / ((a + b) * (a + b));
    double p = utils::PI * (a + b) *
               (1.0 + 3.0 * ab2 / (10.0 + std::sqrt(4.0 - 3.0 * ab2)));

    return p;
  }

  constexpr double area() const override {
    return utils::PI * majorAxis_ * minorAxis_;
  }

  void translate(double dx, double dy) override {
    std::vector<Point> vertices({focusSegment_.first, focusSegment_.second});
    transformations::translate(vertices, dx, dy);
    focusSegment_ = Segment{vertices.front(), vertices.back()};
  }

  void translate(Point offset) override {
    Ellipse::translate(offset.x, offset.y);
  }

  void rotate(Point center, double angle) override {
    std::vector<Point> vertices({focusSegment_.first, focusSegment_.second});
    transformations::rotate(vertices, center, angle);
    focusSegment_ = Segment{vertices.front(), vertices.back()};
  }

  void scale(Point center, double coefficient) override {
    std::vector<Point> vertices({focusSegment_.first, focusSegment_.second});
    transformations::scale(vertices, center, coefficient);
    focusSegment_ = Segment{vertices.front(), vertices.back()};

    majorAxis_ *= coefficient;
    eccentricity_ = focusSegment_.length() / (2.0 * majorAxis_);
    minorAxis_ = majorAxis_ * std::sqrt(1 - eccentricity_ * eccentricity_);
  }

  void reflex(Point center) override { Ellipse::scale(center, -1.0); }

  void reflex(Line axis) override {
    focusSegment_.first = axis.reflex(focusSegment_.first);
    focusSegment_.second = axis.reflex(focusSegment_.second);
  }

  bool operator==(const Shape& other) const override {
    if (this == &other) {
      return true;
    }
    const Ellipse* ptr = dynamic_cast<const Ellipse*>(&other);
    if (ptr == nullptr) {
      return false;
    }
    // compare ellipses by eccentricity and major axis
    return utils::equals(this->axis().first, ptr->axis().first,
                         this->eccentricity(), this->eccentricity());
  }

  bool operator!=(const Shape& other) const override {
    return !(*this == other);
  }
};

class Circle : public Ellipse {
 public:
  Circle(const Point& center, double radius)
      : Ellipse(center, center, 2.0 * radius) {}

  constexpr double radius() const { return Ellipse::axis().first; }

  constexpr double perimeter() const override {
    return 2.0 * utils::PI * radius();
  }

  constexpr double area() const override {
    return utils::PI * radius() * radius();
  }
};

class Polygon : public Shape {
 protected:
  std::vector<Point> vertices_;

 public:
  Polygon() : vertices_() {}

  Polygon(const std::vector<Point>& vertices) : vertices_(vertices) {}

  size_t verticesCount() const { return vertices_.size(); }

  std::vector<Point> getVertices() const { return vertices_; }

  double perimeter() const override {
    auto semgentAccumulator =
        std::vector<Segment>({Segment{vertices_.back(), vertices_.back()}});

    auto sides = std::accumulate(
        vertices_.cbegin(), vertices_.cend(), semgentAccumulator,
        [](auto& segments, const Point& pt) -> auto& {
          segments.push_back(Segment{segments.back().second, pt});
          return segments;
        });

    return std::accumulate(sides.cbegin(), sides.cend(), 0.0,
        [](double sum, auto& segment) -> double {
          return sum + segment.length();
        });
  }

  double area() const override {
    auto semgentAccumulator =
        std::vector<Segment>({Segment{vertices_.front(), vertices_.front()}});

    // vertices_[0] and vertices_[1] prodice degenerate triangles here
    auto triangleSides = std::accumulate(
        vertices_.cbegin(), vertices_.cend(), semgentAccumulator,
        [](auto& segments, const Point& pt) -> auto& {
          segments.push_back(Segment{segments.back().second, pt});
          return segments;
        });

    Point firstPt = vertices_.front();
    double area = std::accumulate(
        triangleSides.cbegin(), triangleSides.cend(), 0.0,
        [firstPt](double sum, auto& segment) -> double {
          double area =
              ((segment.first.x - firstPt.x) *  (segment.second.y - firstPt.y) -
               (segment.second.x - firstPt.x) * (segment.first.y - firstPt.y)) /
              2.0;
          return sum + 
              ((segment.first.x - firstPt.x) *  (segment.second.y - firstPt.y) -
               (segment.second.x - firstPt.x) * (segment.first.y - firstPt.y)) / 2.0;
        });
    return std::abs(area);
  }

  void translate(double dx, double dy) {
    transformations::translate(vertices_, dx, dy);
  }

  void translate(Point offset) override {
    Polygon::translate(offset.x, offset.y);
  }

  void rotate(Point center, double angle) override {
    transformations::rotate(vertices_, center, angle);
  }

  void scale(Point center, double coefficient) override {
    transformations::scale(vertices_, center, coefficient);
  }

  void reflex(Point center) override { Polygon::scale(center, -1.0); }

  void reflex(Line axis) override { transformations::reflex(vertices_, axis); }

  bool operator==(const Shape& other) const override {
    if (this == &other) {
      return true;
    }
    const Polygon* ptr = dynamic_cast<const Polygon*>(&other);
    if (ptr == nullptr) {
      return false;
    }

    // compare polygons by the set of vertices
    if (this->verticesCount() != ptr->verticesCount()) {
      return false;
    }

    auto v = this->getVertices();
    auto thisVertices = this->getVertices();
    v.insert(v.end(), thisVertices.begin(), thisVertices.end());

    auto s = ptr->getVertices();

    auto it = std::search(v.begin(), v.end(), s.begin(), s.end(), 
        [](Point v1, Point v2) -> bool {
          return utils::equals(v1.x, v2.x, v1.y, v1.y);
        });
    if (it != v.end()) {
      return true;
    }

    auto rev = std::search(v.rbegin(), v.rend(), s.begin(), s.end(), 
        [](Point v1, Point v2) -> bool {
          return utils::equals(v1.x, v2.x, v1.y, v1.y);
        });

    if (rev != v.rend()) {
      return true;
    }

    return false;
  }

  bool operator!=(const Shape& other) const override {
    return !(*this == other);
  }
};

class Triangle : public Polygon {
 private:
  constexpr Point centroid(Point A, Point B, Point C) const {
    double x_numer =
        (A.y - B.y) * ((C.x - B.x) * (C.x + B.x) + (C.y - B.y) * (C.y + B.y)) -
        (C.y - B.y) * ((A.x - B.x) * (A.x + B.x) + (A.y - B.y) * (A.y + B.y));
    double x_denom =
        2.0 * (C.y - B.y) * (B.x - A.x) - 2.0 * (B.x - C.x) * (A.y - B.y);

    double y_numer =
        (B.x - C.x) * ((A.x - B.x) * (A.x + B.x) + (A.y - B.y) * (A.y + B.y)) -
        (B.x - A.x) * ((C.x - B.x) * (C.x + B.x) + (C.y - B.y) * (C.y + B.y));
    double y_denom =
        2.0 * (B.y - C.y) * (B.x - A.x) - 2.0 * (B.x - C.x) * (B.y - A.y);

    return {x_numer / x_denom, y_numer / y_denom};
  }

 public:
  Triangle(Point first, Point second, Point third) {
    vertices_.push_back(first);
    vertices_.push_back(second);
    vertices_.push_back(third);
  }

  Circle circumscribedCircle() const {
    Point A = vertices_[0];
    Point B = vertices_[1];
    Point C = vertices_[2];

    Point center = centroid(A, B, C);
    double radius = (Segment{center, A}).length();
    return Circle(center, radius);
  }

  Circle inscribedCircle() const {
    Point A = vertices_[0];
    Point B = vertices_[1];
    Point C = vertices_[2];

    double a = (Segment{A, B}).length();
    double b = (Segment{B, C}).length();
    double c = (Segment{C, A}).length();

    double k = c / a;
    Point U{(C.x + k * B.x) / (1.0 + k), (C.y + k * B.y) / (1.0 + k)};
    k = b / a;
    Point V{(C.x + k * A.x) / (1.0 + k), (C.y + k * A.y) / (1.0 + k)};

    Point center{0.0, 0.0};
    if (A.x == U.x) {
      center.x = A.x;
      center.y = A.x * (V.y - B.y) / (V.x - B.x) +
                 (V.x * B.y - B.x * V.y) / (V.x - B.x);
    } else if (B.x == V.x) {
      center.x = B.x;
      center.y = B.x * (U.y - A.y) / (U.x - A.x) +
                 (U.x * A.y - A.x * U.y) / (U.x - A.x);
    } else {
      double au_num = (U.x * A.y - A.x * U.y) / (U.x - A.x);
      double bv_num = (V.x * B.y - B.x * V.y) / (V.x - B.x);
      double bv_den = (V.y - B.y) / (V.x - B.x);
      double au_den = (U.y - A.y) / (U.x - A.x);
      center.x = (au_num - bv_num) / (bv_den - au_den);
      center.y = (bv_den * au_num - au_den * bv_num) / (bv_den - au_den);
    }

    double radius =
        std::sqrt((-a + b + c) * (a - b + c) * (a + b - c) / (a + b + c)) / 2.0;
    return Circle(center, radius);
  }

  Point centroid() const {
    return centroid(vertices_[0], vertices_[1], vertices_[2]);
  }

  Point orthocenter() const {
    Point A = vertices_[0];
    Point B = vertices_[1];
    Point C = vertices_[2];

    Point D = (Segment{A, B}).center();
    Point E = (Segment{B, C}).center();
    Point F = (Segment{C, A}).center();

    Point O = centroid(A, B, C);
    Point N = centroid(D, E, F);
    return Point{2.0 * N.x - O.x, 2.0 * N.y - O.y};
  }

  Line EulerLine() const {
    Point A = vertices_[0];
    Point B = vertices_[1];
    Point C = vertices_[2];

    Point D = (Segment{A, B}).center();
    Point E = (Segment{B, C}).center();
    Point F = (Segment{C, A}).center();

    Point O = centroid(A, B, C);
    Point N = centroid(D, E, F);
    return Line(O, N);
  }

  Circle ninePointsCircle() const {
    Point A = vertices_[0];
    Point B = vertices_[1];
    Point C = vertices_[2];

    Point D = (Segment{A, B}).center();
    Point E = (Segment{B, C}).center();
    Point F = (Segment{C, A}).center();

    Point center = centroid(D, E, F);
    double radius = (Segment{center, D}).length();
    return Circle(center, radius);
  }
};

class Rectangle : public Polygon {
 public:
  Rectangle(const std::vector<Point>& vertices) : Polygon(vertices) {}

  Rectangle(Point diagFirst, Point diagSecond, double sidesRatio) {
    auto diagonal = Segment(diagFirst, diagSecond);

    double minorSide =
        diagonal.length() / std::sqrt(1.0 + sidesRatio * sidesRatio);
    double majorSide = minorSide * sidesRatio;
    if (sidesRatio < 1.0) {
      std::swap(minorSide, majorSide);
    }

    auto minorAngle =
        diagonal.angle() + std::acos(minorSide / diagonal.length());
    auto majorAngle = minorAngle - utils::PI / 2.0;
    auto minorSegment = Segment(diagFirst, minorAngle, minorSide);
    auto majorSegment = Segment(diagFirst, majorAngle, majorSide);

    vertices_.push_back(diagFirst);
    vertices_.push_back(minorSegment.second);
    vertices_.push_back(diagSecond);
    vertices_.push_back(majorSegment.second);
  }

  std::pair<Line, Line> diagonals() {
    return std::make_pair(Line(vertices_[0], vertices_[2]),
                          Line(vertices_[1], vertices_[3]));
  }

  Point center() const {
    return (Segment{vertices_[0], vertices_[2]}).center();
  }

  Circle circumscribedCircle() {
    return Circle(center(), Segment(vertices_[0], vertices_[2]).length() / 2.0);
  }
};

class Square : public Rectangle {
 public:
  Square(const std::vector<Point>& vertices) : Rectangle(vertices) {}

  Square(Point diagFirst, Point diagSecond)
      : Rectangle(diagFirst, diagSecond, 1.0) {}

  Circle inscribedCircle() {
    return Circle(Rectangle::center(),
                  Segment(vertices_[0], vertices_[1]).length() / 2);
  }

  Circle circumscribedCircle() { return Rectangle::circumscribedCircle(); }
};
}