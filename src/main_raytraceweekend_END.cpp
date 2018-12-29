#include <math.h>
#include <fstream>
#include <iostream>
#include <random>
#include "camera.h"
#include "float.h"
#include "hitable_list.h"
#include "material.h"
#include "sphere.h"

// trying with stb_image lib
#define STB_IMAGE_ IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

vec3 color(const ray &r, hitable *world, int depth) {
    hit_record rec;
    if (world->hit(r, 0.0001, 100000.0, rec)) {
        ray scattered;
        vec3 attenuation;
        if (depth < 10 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
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

hitable *random_scene() {
    int n = 500;
    hitable **list = new hitable *[n + 1];
    list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = r();
            vec3 center(a + 0.9 * r(), 0.2, b + 0.9 * r());
            if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
                if (choose_mat < 0.8) {  // diffuse
                    list[i++] = new sphere(center, 0.2, new lambertian(vec3(r() * r(), r() * r(), r() * r())));
                } else if (choose_mat < 0.95) {  // metal
                    list[i++] = new sphere(center, 0.2,
                                           new metal(vec3(0.5 * (1 + r()), 0.5 * (1 + r()), 0.5 * (1 + r())), 0.5 * r()));
                } else {  // glass
                    list[i++] = new sphere(center, 0.2, new dielectric(1.5));
                }
            }
        }
    }
    list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
    list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
    list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));
    return new hitable_list(list, i);
}

int main() {
    int nx = 640;
    int ny = 480;
    int ns = 10;

    // stb stuff
    // int x, y, n;
    // unsigned char *data = stbi_load("foo.png", &x, &y, &n, 0);
    std::ofstream outfile;
    outfile.open("myfile.ppm", std::ios::out);
    outfile << "P3\n"
            << nx << " " << ny << "\n255\n";

    vec3 lower_left_corner(-2.0, -1.0, -1.0);
    vec3 horizontal(4.0, 0.0, 0.0);
    vec3 vertical(0.0, 2.0, 0.0);
    vec3 origin(0.0, 0.0, 0.0);

    hitable *list[5];
    list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.8, 0.3, 0.3)));
    list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
    list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));
    list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
    list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
    hitable *world = new hitable_list(list, 5);
    world = random_scene();

    vec3 lookfrom(13, 2, 3);
    vec3 lookat(0, 0, 0);
    float dist_to_focus = 10.0;
    float aperture = 0.2;
    camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, dist_to_focus);
    for (int j = ny - 1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            std::cout << i << ":" << j << "\n";
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
            // std::cout << ir << " " << ig << " " << ib << "\n";
            outfile << ir << " " << ig << " " << ib << "\n";
        }
    }
    // stbi_write_png("foo_out.png", nx, ny, 3, data, 0);
}