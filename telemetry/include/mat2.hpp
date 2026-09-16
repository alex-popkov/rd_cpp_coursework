#pragma once

namespace telemetry {

struct Vec2 {
  float a = 0.0f;
  float b = 0.0f;
};

struct Mat2 {
  float m00 = 0.0f, m01 = 0.0f;
  float m10 = 0.0f, m11 = 0.0f;

  static Mat2 identity() { return {1.0f, 0.0f, 0.0f, 1.0f}; }
};

inline Mat2 operator*(const Mat2& A, const Mat2& B)
{
  return {A.m00 * B.m00 + A.m01 * B.m10, A.m00 * B.m01 + A.m01 * B.m11, A.m10 * B.m00 + A.m11 * B.m10, A.m10 * B.m01 + A.m11 * B.m11};
}
inline Vec2 operator*(const Mat2& A, const Vec2& v)
{
  return {A.m00 * v.a + A.m01 * v.b, A.m10 * v.a + A.m11 * v.b};
}
inline Mat2 operator*(float s, const Mat2& A)
{
  return {s * A.m00, s * A.m01, s * A.m10, s * A.m11};
}
inline Mat2 operator+(const Mat2& A, const Mat2& B)
{
  return {A.m00 + B.m00, A.m01 + B.m01, A.m10 + B.m10, A.m11 + B.m11};
}
inline Mat2 operator-(const Mat2& A, const Mat2& B)
{
  return {A.m00 - B.m00, A.m01 - B.m01, A.m10 - B.m10, A.m11 - B.m11};
}
inline Vec2 operator+(const Vec2& u, const Vec2& v)
{
  return {u.a + v.a, u.b + v.b};
}
inline Vec2 operator-(const Vec2& u, const Vec2& v)
{
  return {u.a - v.a, u.b - v.b};
}

inline Mat2 transpose(const Mat2& A)
{
  return {A.m00, A.m10, A.m01, A.m11};
}

// інверс 2×2: 1/det · [[d,-b],[-c,a]]
inline Mat2 inverse(const Mat2& A)
{
  float det = A.m00 * A.m11 - A.m01 * A.m10;
  float inv = 1.0f / det;
  return {inv * A.m11, -inv * A.m01, -inv * A.m10, inv * A.m00};
}

}  // namespace telemetry