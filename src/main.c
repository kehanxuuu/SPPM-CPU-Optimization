#include "scene_configs.h"
#include "bitmap.h"
#include "sppm.h"
#include "sppm_s.h"
#include "pt.h"
#include "normal.h"
#include <time.h>

typedef struct {
    int width;
    int height;
    int num_iterations;
    int ray_max_depth;
    int photon_num_per_iter;
    float initial_radius;
    char *algorithm;
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
#define parse_str(field, field_name, short_param_name, full_param_name) \
if (strcmp(argv[i], short_param_name) == 0 || strcmp(argv[i], full_param_name) == 0) { \
    if (i + 1 >= argc) { \
        fprintf(stderr, "Argument `%s` is missing a value!\n", field_name); \
        exit(1); \
    } \
    params->field = argv[i + 1]; \
}
    for (int i = 1; i < argc; i += 2) {
        char *endptr;
        if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: ./main [options] output_path\n");
            printf("Options:\n");
            printf("    -w --width             image width\n");
            printf("    -h --height            image height\n");
            printf("    -i --iterations        number of iterations (sample per pixel for path tracing)\n");
            printf("    -d --depth             ray max depth\n");
            printf("    -p --photons_per_iter  number of photons per iteration\n");
            printf("    -r --init_radius       initial search radius\n");
            printf("    -a --algorithm         integrator: \"pt\" for path tracing, \"sppm\" for photon mapping (sequential version), \"sppm-simd\" for photon mapping (SIMD version, default), or \"normal\" for quick visualization\n");
            printf("    --help                 print this help text\n");
            exit(0);
        }
        else parse_long(width, "width", "-w", "--width")
        else parse_long(height, "height", "-h", "--height")
        else parse_long(num_iterations, "num_iterations", "-i", "--iterations")
        else parse_long(ray_max_depth, "ray_max_depth", "-d", "--depth")
        else parse_long(photon_num_per_iter, "photon_num_per_iter", "-p", "--photons_per_iter")
        else parse_float(initial_radius, "initial_radius", "-r", "--init_radius")
        else parse_str(algorithm, "algorithm", "-a", "--algorithm")
        else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown argument `%s`\n", argv[i]);
            exit(1);
        }
        else {
            params->output_path = argv[i];
            --i;
        }
    }
    printf("Parameters: width = %d, height = %d, num_iterations = %d, ray_max_depth = %d, photon_num_per_iter = %d, initial_radius = %f, algorithm = %s, output_path = \"%s\"\n",
           params->width, params->height, params->num_iterations, params->ray_max_depth, params->photon_num_per_iter, params->initial_radius,
           params->algorithm, params->output_path);
#undef parse_long
#undef parse_float
}

int main(int argc, char *argv[]) {
    clock_t tic = clock();
    simd_seed(1);
    Params params = {512, 384, 6, 5, 200000, 2.0f, "sppm-simd", "out.exr"};
    parse_args(argc, argv, &params);

    Scene scene;
    Camera camera;
    // init_cornell_box(&scene, &camera);
    // init_large_box(&scene, &camera);
    init_reflect_box(&scene, &camera);
    // init_sky(&scene, &camera);
    cam_set_resolution(&camera, params.width, params.height);

    Bitmap film;
    bitmap_init(&film, params.width, params.height);

    Vector background = ZERO_VEC;

    if (strcmp(params.algorithm, "pt") == 0) {
        PathTracing pt;
        pt_init(&pt, params.num_iterations, params.ray_max_depth, &scene, &camera, background);
        pt_render(&pt, &film);
    }
    else if (strcmp(params.algorithm, "sppm") == 0) {  // sequential
        SPPM_S sppm;
        sppm_init_s(&sppm, params.num_iterations, params.ray_max_depth, params.photon_num_per_iter, params.initial_radius,
                    &scene, &camera, background);
        sppm_render_s(&sppm, &film);
    }
    else if (strcmp(params.algorithm, "sppm-simd") == 0) {  // SIMD
        SPPM sppm;
        sppm_init(&sppm, params.num_iterations, params.ray_max_depth, params.photon_num_per_iter, params.initial_radius,
                  &scene, &camera, background);
        sppm_render(&sppm, &film);
        sppm_free(&sppm);
    }
    else if (strcmp(params.algorithm, "normal") == 0) {
        NormalVisualizer nv;
        nv_init(&nv, &scene, &camera, background);
        nv_render(&nv, &film);
    }
    else {
        fprintf(stderr, "Invalid algorithm `%s`\n", params.algorithm);
        exit(1);
    }

    bitmap_save_exr(&film, params.output_path);
    fprintf(stderr, "Checksum: %.10f\n", bitmap_checksum(&film));
    bitmap_free(&film);

    scene_free(&scene);

    float elapse = (float) (clock() - tic) / CLOCKS_PER_SEC;
    printf("Safe exit in %f seconds\n", elapse);
    return 0;
}
