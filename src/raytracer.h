#ifndef RAYTRACERH
#define RAYTRACERH

#include <malloc.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <random>
#include "camera.h"
#include "float.h"
#include "hitable_list.h"
#include "material.h"
#include "moving_sphere.h"
#include "sphere.h"

#define STB_IMAGE_ IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// ===============================================================
// trying to separate this from main.cpp in order to multithread..?
// ===============================================================

class raytracer {
   public:
    raytracer() {}
    raytracer(const int &width, const int &height, const int &samples) { nx = width; ny = height; ns = samples; }
    vec3 color(const ray &r, hitable *world, int depth);
    hitable *random_scene();
    void render();

    int nx;
    int ny;
    int ns;

    // class vars needed for multithreaded render function?
    hitable *world;
    unsigned char *buffer;
    camera cam;
};

vec3 color(const ray &r, hitable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.0001, 100000.0, rec)) {
        ray scattered;
        vec3 attenuation;
        if (depth < 24 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * color(scattered, world, depth + 1);
        } else {
            return vec3(0, 0, 0);
        }
    } else {
        vec3 unit_directon = unit_vector(r.direction());
        float t = 0.5 * (unit_directon.y() + 1.0);
        return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
    }
}

void render() {
    int nx = 200;
    int ny = 100;
    int ns = 10;
    // stb stuff
    unsigned char *buffer = new unsigned char[nx * ny * 3];

    hitable *list[5];
    list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.8, 0.3, 0.3)));
    list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
    list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));
    list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
    list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
    hitable *world = new hitable_list(list, 5);

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.2;
    camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);

    //   // try multithreading?
    //   int mMaxThreads = 16;
    //   std::thread mThreads[mMaxThreads];
    //   for (int threadIndex = 0; threadIndex < mMaxThreads; ++threadIndex) {
    //     mThreads[threadIndex] =
    //         std::thread(&ray_tracer::put_pixel_with_thread, this, threadIndex);
    //   }
    //   for (int threadIndex = 0; threadIndex < mMaxThreads; ++threadIndex) {
    //     if (mThreads[threadIndex].joinable()) {
    //       mThreads[threadIndex].join();
    //     }
    //   }

    // for (int col = int((threadIndex / mMaxThreads)*mImageWidth);
    //     col < int(((threadIndex + 1) / mMaxThreads)*mImageWidth);
    //     ++col {

    for (int j = ny - 1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0.0, 0.0, 0.0);
            for (int s = 0; s < ns; s++) {
                float u = float(i + r()) / float(nx);
                float v = float(j + r()) / float(ny);
                ray r = cam.get_ray(u, v);
                vec3 p = r.point_at_parameter(2.0);
                col += color(r, world, 0);
            }
            col /= float(ns);
            float degamma = 1.0 / 2.2;
            col = vec3(pow(col[0], degamma), pow(col[1], degamma), pow(col[2], degamma));
            int ir = int(255.99 * col[0]);
            int ig = int(255.99 * col[1]);
            int ib = int(255.99 * col[2]);

            int idx = (((ny - j) * nx) + i) * 3;
            buffer[idx] = ir;
            buffer[idx + 1] = ig;
            buffer[idx + 2] = ib;
        }
        std::cout << j << "\n";
    }

    if (!stbi_write_bmp("render.bmp", nx, ny, 3, buffer)) {
        std::cout << "ERROR: could not write image";
    }
}

#endif