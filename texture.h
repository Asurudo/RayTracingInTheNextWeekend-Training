#ifndef TEXTUREH
#define TEXTUREH

// 前向声明
extern Rand jyorandengine;

#include "perlin.h"

class texture {
 public:
  virtual vec3 value(double u, double v, const vec3& p) const = 0;
};

// 无论uv，无论交点的颜色恒定的颜色材质
class constant_texture : public texture {
 private:
  vec3 color;

 public:
  constant_texture() {}
  constant_texture(vec3 c) : color(c) {}
  // 返回输入的RGB反射信息，也就是这个球的颜色恒定
  virtual vec3 value(double u, double v, const vec3& p) const { return color; }
};

// 格子状的检测材质
class checker_texture : public texture {
 private:
  texture *odd, *even;

 public:
  checker_texture() {}
  checker_texture(texture* t0, texture* t1) : even(t0), odd(t1) {}
  virtual vec3 value(double u, double v, const vec3& p) const {
    // 对于传入的点坐标，运用该函数将每个点做哈希，判断该点应该使用两种材质中的哪一个
    double sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
    if (sines < 0)
      return odd->value(u, v, p);
    else
      return even->value(u, v, p);
  }
};

// perlin噪声材质
class noise_texture : public texture {
 private:
  perlin noise;
  // 噪音频率
  double scale;

  // 多重采样
  double turb(const vec3& p, int depth = 7) const {
    double accum = 0;
    vec3 tmp_p = p;
    double weight = 1.0;
    for (int i = 0; i < depth; i++) {
      accum += weight * noise.noise(tmp_p);
      weight /= 2;
      tmp_p *= 2;
    }
    return abs(accum);
  }

 public:
  noise_texture() {}
  noise_texture(double scale) : scale(scale) {}
  virtual vec3 value(double u, double v, const vec3& p) const {
    // 淡雅近似大理石
    // return vec3(1, 1, 1) * 0.5 * (1 + turb(scale * p));
    // 深纹近似大理石
    // return vec3(1, 1, 1) * turb(scale * p);
    // 大理石
    return vec3(1, 1, 1) * 0.5 * (1 + sin(scale * p.z() + 10 * turb(p)));
  }
};

#endif