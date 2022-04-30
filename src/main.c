#include "scene.h"
#include "camera.h"
#include "bitmap.h"
#include "sppm.h"
#include "pt.h"

void init_cornell_box(Scene *scene, Camera *camera) {
    const float inf = 1e4f;
    scene_init(scene);
    Mesh *left = mesh_make_sphere((Vector) {inf + 1, 40.8f, 81.6f}, inf, DIFFUSE, (Vector) {.75f, .25f, .25f}, ZERO_VEC, 1.0);
    Mesh *right = mesh_make_sphere((Vector) {-inf + 99, 40.8f, 81.6f}, inf, DIFFUSE, (Vector) {.25f, .25f, .75f}, ZERO_VEC, 1.0);
    Mesh *back = mesh_make_sphere((Vector) {50, 40.8f, inf}, inf, DIFFUSE, (Vector) {.75f, .75f, .75f}, ZERO_VEC, 1.0);
    Mesh *front = mesh_make_sphere((Vector) {50, 40.8f, -inf + 170}, inf, DIFFUSE, ZERO_VEC, ZERO_VEC, 1.0);
    Mesh *bottom = mesh_make_sphere((Vector) {50, inf, 81.6f}, inf, DIFFUSE, (Vector) {.75f, .75f, .75f}, ZERO_VEC, 1.0);
    Mesh *top = mesh_make_sphere((Vector) {50, -inf + 81.6f, 81.6f}, inf, DIFFUSE, (Vector) {.75f, .75f, .75f}, ZERO_VEC, 1.0);
    Mesh *mirror = mesh_make_sphere((Vector) {27, 16.5f, 47}, 16.5f, SPECULAR, (Vector) {.999f, .999f, .999f}, ZERO_VEC, 1.0);
    Mesh *glass = mesh_make_sphere((Vector) {73, 16.5f, 78}, 16.5f, DIELECTRIC, (Vector) {.999f, .999f, .999f}, ZERO_VEC, 1.5);
    Mesh *light1 = mesh_make_sphere((Vector) {20, 81.6f - 16.5f, 81.6f}, 7.5f, DIFFUSE, ZERO_VEC, (Vector) {20, 10, 10}, 1.0);
    Mesh *light2 = mesh_make_sphere((Vector) {80, 81.6f - 16.5f, 81.6f}, 5, DIFFUSE, ZERO_VEC, (Vector) {10, 10, 20}, 1.0);
    scene_add(scene, left);
    scene_add(scene, right);
    scene_add(scene, back);
//    scene_add(scene, front);
    scene_add(scene, bottom);
    scene_add(scene, top);
    scene_add(scene, mirror);
    scene_add(scene, glass);
    scene_add(scene, light1);
    scene_add(scene, light2);
    scene_finish(scene);
    camera->fov = 30 * M_PI / 180.f;
    Vector eye = {50, 52, 295.6f}, target = vv_add(&eye, &(Vector) {0, -0.042612f, -1}), up = {0, 1, 0};
    cam_look_at(camera, eye, target, up);
}

typedef struct {
    int width;
    int height;
    int num_iterations;
    int ray_max_depth;
    int photon_num_per_iter;
    float initial_radius;
    char *output_path;
} Params;

void parse_args(int argc, char *argv[], Params *params) {
#define parse_long(field, field_name, short_param_name, full_param_name) \
if (strcmp(argv[i], short_param_name) == 0 || strcmp(argv[i], full_param_name) == 0) { \
    if (i + 1 >= argc) { \
        fprintf(stderr, "Argument `%s` is missing a value!\n", field_name); \
        exit(1); \
    } \
    params->field = strtol(argv[i + 1], &endptr, 10); \
    if (*endptr != '\0') { \
        fprintf(stderr, "Argument `%s` is invalid!\n", field_name); \
        exit(1); \
    } \
}
#define parse_float(field, field_name, short_param_name, full_param_name) \
if (strcmp(argv[i], short_param_name) == 0 || strcmp(argv[i], full_param_name) == 0) { \
    if (i + 1 >= argc) { \
        fprintf(stderr, "Argument `%s` is missing a value!\n", field_name); \
        exit(1); \
    } \
    params->field = strtof(argv[i + 1], &endptr); \
    if (*endptr != '\0') { \
        fprintf(stderr, "Argument `%s` is invalid!\n", field_name); \
        exit(1); \
    } \
}
    for (int i = 1; i < argc; i += 2) {
        char *endptr;
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: ./main [options] output_path\n");
            printf("Options:\n");
            printf("    -w --width             image width\n");
            printf("    -h --height            image height\n");
            printf("    -i --iterations        number of iterations\n");
            printf("    -d --depth             ray max depth\n");
            printf("    -p --photons_per_iter  number of photons per iteration\n");
            printf("    -r --init_radius       initial search radius\n");
            printf("    -h --help              print this help text\n");
            exit(0);
        }
        else parse_long(width, "width", "-w", "--width")
        else parse_long(height, "height", "-h", "--height")
        else parse_long(num_iterations, "num_iterations", "-i", "--iterations")
        else parse_long(ray_max_depth, "ray_max_depth", "-d", "--depth")
        else parse_long(photon_num_per_iter, "photon_num_per_iter", "-p", "--photons_per_iter")
        else parse_float(initial_radius, "initial_radius", "-r", "--init_radius")
        else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown argument `%s`\n", argv[i]);
            exit(1);
        }
        else {
            params->output_path = argv[i];
            --i;
        }
    }
    printf("Parameters: width = %d, height = %d, num_iterations = %d, ray_max_depth = %d, photon_num_per_iter = %d, initial_radius = %f, output_path = \"%s\"\n",
           params->width, params->height, params->num_iterations, params->ray_max_depth, params->photon_num_per_iter, params->initial_radius,
           params->output_path);
}

int main(int argc, char *argv[]) {
    srand(0);
    Params params = {1024, 768, 8, 20, 200000, 2.0f, "out.exr"};
    parse_args(argc, argv, &params);

    Scene scene;
    Camera camera;
    init_cornell_box(&scene, &camera);
    cam_set_resolution(&camera, params.width, params.height);

    Bitmap film;
    bitmap_init(&film, params.width, params.height);

    Vector background = ZERO_VEC; // mimic sky color for now, should be zero for physical correctness

//    PathTracing pt;
//    pt_init(&pt, 16, 10, &scene, &camera, background);
//    pt_render(&pt, &film);
    SPPM sppm;
    sppm_init(&sppm, params.num_iterations, params.ray_max_depth, params.photon_num_per_iter, params.initial_radius,
              &scene, &camera, background);
    sppm_render(&sppm, &film);

    bitmap_save_exr(&film, params.output_path);
    bitmap_free(&film);

    scene_free(&scene);

    printf("Safe exit\n");
    return 0;
}
