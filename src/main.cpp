#include "raytracer.h"

// hitable *random_scene() {
//     int n = 500;
//     hitable **list = new hitable *[n + 1];
//     list[0] =
//         new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
//     int i = 1;
//     for (int a = -11; a < 11; a++) {
//         for (int b = -11; b < 11; b++) {
//             float choose_mat = r();
//             vec3 center(a + 0.9 * r(), 0.2, b + 0.9 * r());
//             if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
//                 if (choose_mat < 0.8) {  // diffuse
//                     // list[i++] = new sphere(center, 0.2, new lambertian(vec3(r() * r(),
//                     // r() * r(), r() * r())));
//                     list[i++] = new moving_sphere(
//                         center, center + vec3(0, 0.5 * r(), 0), 0.0, 1.0, 0.2,
//                         new lambertian(vec3(r() * r(), r() * r(), r() * r())));
//                 } else if (choose_mat < 0.95) {  // metal
//                     list[i++] = new sphere(
//                         center, 0.2,
//                         new metal(vec3(0.5 * (1 + r()), 0.5 * (1 + r()), 0.5 * (1 + r())),
//                                   0.5 * r()));
//                 } else {  // glass
//                     list[i++] = new sphere(center, 0.2, new dielectric(1.5));
//                 }
//             }
//         }
//     }
//     list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
//     list[i++] =
//         new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
//     list[i++] =
//         new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));
//     return new hitable_list(list, i);
// }

int main() {
    raytracer renderer(200, 100, 10);
    renderer.render();
}