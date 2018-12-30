#ifndef RAYTRACERH
#define RAYTRACERH

#include <malloc.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>
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
    raytracer(const int width, const int height, const int samples) : nx(width), ny(height), ns(samples) {
        vec3 lookfrom(13, 2, 3);
        vec3 lookat(0, 0, 0);
        float dist_to_focus = 10.0;
        float aperture = 0.2;
        cam = camera(lookfrom, lookat, vec3(0, 1, 0), 20, float(width) / float(height), aperture, dist_to_focus, 0.0, 1.0);

        // WHY WONT THIS WORK????
        buffer = new unsigned char[width * height * 3];
    }

    vec3 color(const ray &r, hitable *world, int depth);
    hitable *random_scene();
    void render();
    void renderchunk();

    int nx;
    int ny;
    int ns;
    unsigned char *buffer;
    camera cam;
};

hitable *raytracer::random_scene() {
    int n = 500;
    hitable **list = new hitable *[n + 1];
    list[0] =
        new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = r();
            vec3 center(a + 0.9 * r(), 0.2, b + 0.9 * r());
            if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
                if (choose_mat < 0.8) {  // diffuse
                    // list[i++] = new sphere(center, 0.2, new lambertian(vec3(r() * r(),
                    // r() * r(), r() * r())));
                    list[i++] = new moving_sphere(
                        center, center + vec3(0, 0.5 * r(), 0), 0.0, 1.0, 0.2,
                        new lambertian(vec3(r() * r(), r() * r(), r() * r())));
                } else if (choose_mat < 0.95) {  // metal
                    list[i++] = new sphere(
                        center, 0.2,
                        new metal(vec3(0.5 * (1 + r()), 0.5 * (1 + r()), 0.5 * (1 + r())),
                                  0.5 * r()));
                } else {  // glass
                    list[i++] = new sphere(center, 0.2, new dielectric(1.5));
                }
            }
        }
    }
    list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
    list[i++] =
        new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
    list[i++] =
        new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));
    return new hitable_list(list, i);
}

vec3 raytracer::color(const ray &r, hitable *world, int depth) {
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

// need to pass in args.. instance variables arent working???
// void raytracer::renderchunk() {
//     for (int j = ny - 1; j >= 0; j--) {
//         for (int col = int((threadIndex / mMaxThreads)*mImageWidth); col < int(((threadIndex + 1) / mMaxThreads)*mImageWidth); ++col {
//             vec3 col(0.0, 0.0, 0.0);
//             for (int s = 0; s < ns; s++) {
//                 float u = float(i + r()) / float(nx);
//                 float v = float(j + r()) / float(ny);
//                 ray r = cam.get_ray(u, v);
//                 vec3 p = r.point_at_parameter(2.0);
//                 col += color(r, world2, 0);
//             }
//             col /= float(ns);
//             float degamma = 1.0 / 2.2;
//             col = vec3(pow(col[0], degamma), pow(col[1], degamma), pow(col[2], degamma));
//             int ir = int(255.99 * col[0]);
//             int ig = int(255.99 * col[1]);
//             int ib = int(255.99 * col[2]);

//             int idx = (((ny - j) * nx) + i) * 3;
//             buffer2[idx] = ir;
//             buffer2[idx + 1] = ig;
//             buffer2[idx + 2] = ib;
//         }
//     }
// }

void raytracer::render() {
    hitable *list2[1];
    list2[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.8, 0.3, 0.3)));
    hitable *world2 = random_scene();

    unsigned char *buffer2 = new unsigned char[nx * ny * 3];
    
    // try multithreading?
    // int mMaxThreads = 16;
    // std::thread mThreads[mMaxThreads];
    // for (int threadIndex = 0; threadIndex < mMaxThreads; ++threadIndex) {
    //     mThreads[threadIndex] = std::thread(&renderchunk, this, threadIndex);
    // }
    // for (int threadIndex = 0; threadIndex < mMaxThreads; ++threadIndex) {
    //     if (mThreads[threadIndex].joinable()) {
    //         mThreads[threadIndex].join();
    //     }
    // }

    for (int j = ny - 1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0.0, 0.0, 0.0);
            for (int s = 0; s < ns; s++) {
                float u = float(i + r()) / float(nx);
                float v = float(j + r()) / float(ny);
                ray r = cam.get_ray(u, v);
                vec3 p = r.point_at_parameter(2.0);
                col += color(r, world2, 0);
            }
            col /= float(ns);
            float degamma = 1.0 / 2.2;
            col = vec3(pow(col[0], degamma), pow(col[1], degamma), pow(col[2], degamma));
            int ir = int(255.99 * col[0]);
            int ig = int(255.99 * col[1]);
            int ib = int(255.99 * col[2]);

            int idx = (((ny - j) * nx) + i) * 3;
            buffer2[idx] = ir;
            buffer2[idx + 1] = ig;
            buffer2[idx + 2] = ib;
        }
        std::cout << j << "\n";
    }

    if (!stbi_write_bmp("render.bmp", nx, ny, 3, buffer2)) {
        std::cout << "ERROR: could not write image";
    }
}

#endif