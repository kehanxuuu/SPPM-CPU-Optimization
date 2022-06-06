#include "scene_l.h"

void mesh_l_init(MeshL* mesh_l, size_t size){
    float4_init(&mesh_l->collision_sphere, size);
    float8_init(&mesh_l->material_data, size);
}

void mesh_l_free(MeshL* mesh_l){
    float4_free(&mesh_l->collision_sphere);
    float8_free(&mesh_l->material_data);
}

void emitter_l_init(EmitterL* emitter_l, size_t size){
    float8_init(&emitter_l->emission_data, size);
}

void emitter_l_free(EmitterL* emitter_l){
    float8_free(&emitter_l->emission_data);
}

void albedo_l_init(AlbedoL* albedo_l, size_t size){
    float4_init(&albedo_l->albedo_data, size);
}

void albedo_l_free(AlbedoL* albedo_l){
    float4_free(&albedo_l->albedo_data);
}

void scene_l_init_from(SceneL* scene_l, Scene *scene){
    scene_l->n_meshes = (int)scene->n_meshes;
    scene_l->n_emitters = (int)scene->n_emitters;
    mesh_l_init(&scene_l->meshes, scene_l->n_meshes);
    emitter_l_init(&scene_l->emitters, scene_l->n_emitters);
    albedo_l_init(&scene_l->albedos, scene_l->n_meshes);
    scene_l->accum_probabilities = (float *) malloc_align((scene_l->n_emitters + 1) * sizeof(float));
    memcpy(scene_l->accum_probabilities, scene->accum_probabilities, (scene_l->n_emitters + 1) * sizeof(float));
    for(int i = 0; i < scene_l->n_meshes; i++){
        Mesh* mesh = scene_get(scene, i);
        Sphere* sphere = mesh->geometry->data;
        scene_l->meshes.collision_sphere.data[4 * i + 0] = sphere->c.x;
        scene_l->meshes.collision_sphere.data[4 * i + 1] = sphere->c.y;
        scene_l->meshes.collision_sphere.data[4 * i + 2] = sphere->c.z;
        scene_l->meshes.collision_sphere.data[4 * i + 3] = sphere->r2;

        scene_l->meshes.material_data.data[8 * i + 0] = mesh->material;
        scene_l->meshes.material_data.data[8 * i + 1] = mesh->albedo.x;
        scene_l->meshes.material_data.data[8 * i + 2] = mesh->albedo.y;
        scene_l->meshes.material_data.data[8 * i + 3] = mesh->albedo.z;
        scene_l->meshes.material_data.data[8 * i + 4] = mesh->emission.x;
        scene_l->meshes.material_data.data[8 * i + 5] = mesh->emission.y;
        scene_l->meshes.material_data.data[8 * i + 6] = mesh->emission.z;
        scene_l->meshes.material_data.data[8 * i + 7] = mesh->ir;

        scene_l->albedos.albedo_data.data[4 * i + 0] = mesh->albedo.x;
        scene_l->albedos.albedo_data.data[4 * i + 1] = mesh->albedo.y;
        scene_l->albedos.albedo_data.data[4 * i + 2] = mesh->albedo.z;
    }

    for(int i = 0; i < scene_l->n_emitters; i++){
        Mesh* mesh = scene->emitters[i];
        Sphere* sphere = mesh->geometry->data;
        scene_l->emitters.emission_data.data[8 * i + 0] = sphere->c.x;
        scene_l->emitters.emission_data.data[8 * i + 1] = sphere->c.y;
        scene_l->emitters.emission_data.data[8 * i + 2] = sphere->c.z;
        scene_l->emitters.emission_data.data[8 * i + 3] = sphere->r;
        scene_l->emitters.emission_data.data[8 * i + 4] = mesh->emission.x;
        scene_l->emitters.emission_data.data[8 * i + 5] = mesh->emission.y;
        scene_l->emitters.emission_data.data[8 * i + 6] = mesh->emission.z;
    }
}

void scene_l_free(SceneL* scene_l){
    mesh_l_free(&scene_l->meshes);
    emitter_l_free(&scene_l->emitters);
    albedo_l_free(&scene_l->albedos);
    free(scene_l->accum_probabilities);
}