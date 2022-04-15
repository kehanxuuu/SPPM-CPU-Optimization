#include "scene.h"
#include "camera.h"
#include "bitmap.h"

#define DEBUG

#ifdef DEBUG
void print_mesh_memory_address(const struct Scene *scene, int idx) {
    struct Mesh *m = scene_get(scene, 0);
    // *(struct Mesh **)arr_get(&(scene.meshes), 0); // scene.meshes.data
    struct Geometry g = (struct Geometry)(m->geometry);
    struct Sphere *s = (struct Sphere *)(m->geometry.data);
    printf("Mesh address %d: %p\n", idx, m);
    printf("Geometry address %d: %p\n", idx, &g);
    printf("Sphere address %d: %p\n", idx, s);
    printf("Sphere radius %d: %f\n", idx, s->r);
    fflush(stdout);
}
#endif

int main() {
    srand(0);
    size_t W = 1920, H = 1080;
    struct Bitmap film;
    bitmap_init(&film, W, H);

    struct Scene scene;
    {
        // TODO: scene initialization codes
        // Simple scene & initialization
        struct Sphere sphere = {
            {0, 0, 0},
            5.0, 25.0
        };
        struct Geometry geometry = {
            Sphere,
            &sphere
        };
        struct Mesh mesh = {
            geometry,
            Diffuse,
            {0.5, 0.5, 0.5},
            {0, 0, 0}
        };
        scene_initialize_with_mesh(&scene, &mesh);
    }

#ifdef DEBUG
    // can read sphere data correctly from memory
    print_mesh_memory_address(&scene, 1);
#endif

    struct Camera camera;
    {
        cam_set_resolution(&camera, W, H);
        camera.fov = M_PI_2;
        Vector eye = {0, 0, 10}, target = {0, 0, 0}, up = {0, 1, 0};
        cam_look_at(&camera, &eye, &target, &up);

#ifdef DEBUG
        // print camera memeory address
        printf("camera address c: %p\n", &(camera.c));
        printf("camera address ez: %p\n", &(camera.ez));
        printf("camera address ex: %p\n", &(camera.ex));
        printf("camera address ey: %p\n", &(camera.ey));
#endif
    }

#ifdef DEBUG
    // cannot read sphere data correctly from memory!!
    // WRONG sphere memory address!!
    // would be fine if the above camera code gets romoved
    // seems like camera struct address interfere with scene struct -> why??
    print_mesh_memory_address(&scene, 2);
#endif

    for (size_t py = 0; py < H; ++py) {
        for (size_t px = 0; px < W; ++px) {
            Vector2f sample = {randf(), randf()};
            Ray ray = generate_ray(&camera, px, py, sample);
            struct Intersection isect;
            Vector color;
            if (scene_do_intersect(&scene, &ray)) {
                // scene_intersect(&scene, &ray, &isect)
                color.x = color.y = color.z = 1;
            }
            else 
            {
                color.x = (float) px / (float) W;
                color.y = (float) py / (float) H;
                color.z = 0;
                // color = Vector{(float) px / (float) W, (float) py / (float) H, 0};
            }
            bitmap_set(&film, px, py, &color);
        }
    }

    bitmap_save_exr(&film, "../out/sample.exr");
    bitmap_free(&film);
    return 0;
}
