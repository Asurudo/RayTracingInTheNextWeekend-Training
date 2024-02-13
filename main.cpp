#include <algorithm>
#include <cfloat>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "bvh.h"
#include "camera.h"
#include "hitablelist.h"
#include "jyorand.h"
#include "kuinkerm.h"
#include "material.h"
#include "sphere.h"
#include "texture.h"

using namespace std;
Rand jyorandengine;
hitable_list world;

vec3 randomInUnitSphere() {
  vec3 p;
  do {
    p = vec3(jyorandengine.jyoRandGetReal<double>(-1, 1),
             jyorandengine.jyoRandGetReal<double>(-1, 1),
             jyorandengine.jyoRandGetReal<double>(-1, 1));
  } while (p.squared_length() >= 1.0);
  return p;
}

vec3 randomInUnitDisk() {
  vec3 p;
  do {
    p = vec3(jyorandengine.jyoRandGetReal<double>(-1, 1),
             jyorandengine.jyoRandGetReal<double>(-1, 1), 0);
  } while (p.squared_length() >= 1.0);
  return p;
}

// 颜色插值
vec3 color(const ray& in, int depth) {
  hit_record rec;
  // 减少误差，-0.00001也可以是交点
  if (world.hitanythingbvh(in, 0.001, DBL_MAX, rec)) {
    // 反射出来的光线
    ray scattered;
    // 材料的吸收度
    vec3 attenuation;
    if (depth < 50 && rec.mat_ptr->scatter(in, rec, attenuation, scattered))
      return attenuation * color(scattered, depth + 1);
    else
      return vec3(0, 0, 0);
  } else  // 天空
  {
    // 将射线in单位化，让其长度为1
    vec3 unit_direction = unit_vector(in.direction());
    // 单位化后的射线的y的取值范围为[-1,1]，将其+1再除以2转化为[0,1]
    double t = 0.5 * (unit_direction.y() + 1.0);
    // 进行颜色插值
    return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
  }
  exit(0);
}
std::vector<shared_ptr<hitable>> worldlist;
void buildWorld() {
  texture* checkertextptr =
      new checker_texture(new constant_texture(
          vec3(0.2, 0.3, 0.1)), new constant_texture(vec3(0.9, 0.9, 0.9)));
  worldlist.emplace_back(
      new sphere(vec3(0, -1000, 0), 1000,
                 new lambertian(checkertextptr)));
  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      double choose_mat = jyorandengine.jyoRandGetReal<double>(0, 1);
      vec3 center(a + 0.9 * jyorandengine.jyoRandGetReal<double>(0, 1), 0.2,
                  b + 0.9 * jyorandengine.jyoRandGetReal<double>(0, 1));
      if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
        if (choose_mat < 0.8) {
          worldlist.emplace_back(new moving_sphere(
              center, center + vec3(0, 0.5, 0), 0.0, 1.0, 0.2,
              new lambertian(new constant_texture(
                  vec3(jyorandengine.jyoRandGetReal<double>(0, 1) *
                           jyorandengine.jyoRandGetReal<double>(0, 1),
                       jyorandengine.jyoRandGetReal<double>(0, 1) *
                           jyorandengine.jyoRandGetReal<double>(0, 1),
                       jyorandengine.jyoRandGetReal<double>(0, 1) *
                           jyorandengine.jyoRandGetReal<double>(0, 1))))));
        } else if (choose_mat < 0.95)
          worldlist.emplace_back(new sphere(
              center, 0.2,
              new metal(
                  vec3(0.5 *
                           (1 + jyorandengine.jyoRandGetReal<double>(0, 1) *
                                    jyorandengine.jyoRandGetReal<double>(0, 1)),
                       0.5 *
                           (1 + jyorandengine.jyoRandGetReal<double>(0, 1) *
                                    jyorandengine.jyoRandGetReal<double>(0, 1)),
                       0.5 * (1 +
                              jyorandengine.jyoRandGetReal<double>(0, 1) *
                                  jyorandengine.jyoRandGetReal<double>(0, 1))),
                  0.5 * jyorandengine.jyoRandGetReal<double>(0, 1) *
                      jyorandengine.jyoRandGetReal<double>(0, 1))));
        else
          worldlist.emplace_back(new sphere(center, 0.2, new dielectric(1.5)));
      }
    }
  }

  worldlist.emplace_back(new sphere(vec3(0, 1, 0), 1, new dielectric(1.5)));
  worldlist.emplace_back(
      new sphere(vec3(-4, 1, 0), 1,
                 new lambertian(new constant_texture(vec3(0.4, 0.2, 0.1)))));
  worldlist.emplace_back(
      new sphere(vec3(4, 1, 0), 1, new metal(vec3(0.7, 0.6, 0.5), 0)));

  // 从世界列表中创建bvh树
  shared_ptr<hitable> rootptr;
  bvh_node(worldlist, rootptr);
  world = hitable_list(rootptr);
  // world = hitable_list(worldlist);
}

int main() {
  ofstream mout;
  mout.open("output.PPM");

  // 画布的长
  int nx = 1000;
  // 画布的宽
  int ny = 500;
  // 画布某一点的采样数量
  int ns = 50;
  mout << "P3\n" << nx << " " << ny << "\n255\n";

  buildWorld();
  vec3 lookfrom(13, 2, 3), lookat(0, 0, 0);
  camera cam(lookfrom, lookat, 20, double(nx) / double(ny), 0.0, 1.0, 0.0,
             0.7 * (lookfrom - lookat).length());

  for (int j = ny - 1; j >= 0; j--) {
    cout << "loading..." << 100 * (1.0 - double(j) / double(ny)) << "%";
    for (int i = 0; i < nx; i++) {
      // 最终的颜色
      vec3 col(0, 0, 0);
      for (int k = 0; k < ns; k++) {
        // 点(u,v)是点(i,j)的反离散化
        double u = double(i + jyorandengine.jyoRandGetReal<double>(-1, 1)) /
                   double(nx);
        double v = double(j + jyorandengine.jyoRandGetReal<double>(-1, 1)) /
                   double(ny);

        // 一条射向画布上点(u,v)的光线，注意(u,v)不是真实坐标而是在画布上的比例位置
        ray r = cam.get_ray(u, v);
        col += color(r, 0);
      }
      // 取颜色的平均值
      col /= double(ns);
      // gamma2修正，提升画面的质量
      col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
      int ir = int(255.99 * col[0]);
      int ig = int(255.99 * col[1]);
      int ib = int(255.99 * col[2]);
      mout << ir << " " << ig << " " << ib << "\n";
    }
    system("cls");
  }

  mout.close();
  system("pause");
  return 0;
}