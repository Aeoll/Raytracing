#ifndef RAYTRACERH
#define RAYTRACERH

#include <malloc.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
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

        buffer = new unsigned char[width * height * 3];
        world = random_scene();
    }

    vec3 color(const ray &r, hitable *world, int depth);
    hitable *random_scene();
    void render();
    void renderchunk(int threadIndex, int mMaxThreads);
    void test(int threadIndex);

    int nx;
    int ny;
    int ns;
    unsigned char *buffer;
    hitable *world;
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

//
void raytracer::renderchunk(int threadIndex, int mMaxThreads) {
    int start = int((float(threadIndex) / float(mMaxThreads)) * ny);
    int end = int((float(threadIndex + 1) / float(mMaxThreads)) * ny);

    // multithreading row chunks
    for (int j = end - 1; j >= start; j--) {
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
            // modulo to prevent segfault... should fix properly..?
            buffer[(idx)%(nx*ny*3)] = ir;
            buffer[(idx + 1)%(nx*ny*3)] = ig;
            buffer[(idx + 2)%(nx*ny*3)] = ib;
        }
    }
}

void raytracer::test(int threadIndex) {
    std::cout << threadIndex << std::endl;
}

void raytracer::render() {
    // multithreaded
    const int mMaxThreads = std::min(32*4, ny);

    std::vector<std::thread> mThreads(mMaxThreads);
    for (int threadIndex = 0; threadIndex < mMaxThreads; ++threadIndex) {
        mThreads[threadIndex] = std::thread(&raytracer::renderchunk, this, threadIndex, mMaxThreads);
    }
    for (int threadIndex = 0; threadIndex < mMaxThreads; ++threadIndex) {
        if (mThreads[threadIndex].joinable()) {
            mThreads[threadIndex].join();
        }
    }

    if (!stbi_write_bmp("render.bmp", nx, ny, 3, buffer)) {
        std::cout << "ERROR: could not write image";
    }
}

#endif