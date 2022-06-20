#ifndef TEAM32_SCENE_L_H
#define TEAM32_SCENE_L_H

#include "common.h"
#include "dt_l.h"
#include "scene.h"

typedef struct {
//  center, r2
    Float4 collision_sphere;
//  material, albedo, emission, ir
    Float8 material_data;
} MeshL;

typedef struct {
    //  sphere c, sphere r, emission, padding
    Float8 emission_data;
} EmitterL;

typedef struct {
    //  sphere c, sphere r, emission, padding
    Float4 albedo_data;
} AlbedoL;

typedef struct {
    MeshL meshes; // store pointers to meshes
    EmitterL emitters;
    AlbedoL albedos;
    float *accum_probabilities;  // accumulated probabilities for emitter sample
    int n_meshes;
    int n_emitters;
} SceneL;

void mesh_l_init(MeshL* mesh_l, size_t size);

void mesh_l_free(MeshL* mesh_l);

void emitter_l_init(EmitterL* emitter_l, size_t size);

void emitter_l_free(EmitterL* emitter_l);

void albedo_l_init(AlbedoL* emitter_l, size_t size);

void albedo_l_free(AlbedoL* emitter_l);

void scene_l_init_from(SceneL* scene_l, Scene *scene);

void scene_l_free(SceneL* scene_l);

#endif //TEAM32_SCENE_L_H
