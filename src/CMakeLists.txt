add_subdirectory(ext)
include_directories(${OPENEXR_INCLUDE_DIRS})

# TODO: separate CMakeLists into own directory

aux_source_directory(lib LIB_SOURCES)
add_library(rt_lib ${LIB_SOURCES})
target_link_libraries(rt_lib IlmImf)

aux_source_directory(algorithm ALG_SOURCES)
add_library(rt_alg ${ALG_SOURCES})
target_link_libraries(rt_alg rt_lib)

include_directories(lib)
include_directories(algorithm)
aux_source_directory(. SOURCES)
add_executable(main ${SOURCES})
target_link_libraries(main rt_lib)
target_link_libraries(main rt_alg)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64") # M1 mac doesn't support -march=native and -lubsan
    target_compile_options(rt_lib PUBLIC $<$<CONFIG:DEBUG>:-Wall -fsanitize=address -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=implicit-conversion>)
    target_link_options(rt_lib PUBLIC $<$<CONFIG:DEBUG>:-Wall -fsanitize=address -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=implicit-conversion>)

    target_compile_options(rt_lib PUBLIC $<$<CONFIG:RELEASE>:-Wall -ffast-math>)
    target_link_options(rt_lib PUBLIC $<$<CONFIG:RELEASE>:-Wall -ffast-math>)

    target_compile_options(rt_lib PUBLIC $<$<CONFIG:PROFILE>:-Wall -g -O3 -ffast-math>)
    target_link_options(rt_lib PUBLIC $<$<CONFIG:PROFILE>:-Wall -g -O3 -ffast-math>)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin") # Intel mac also doesn't support -lubsan
    target_compile_options(rt_lib PUBLIC $<$<CONFIG:DEBUG>:-Wall -fsanitize=address -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=implicit-conversion -march=native>)
    target_link_options(rt_lib PUBLIC $<$<CONFIG:DEBUG>:-Wall -fsanitize=address -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=implicit-conversion -march=native -lm>)

    target_compile_options(rt_lib PUBLIC $<$<CONFIG:RELEASE>:-Wall -ffast-math -march=native>)
    target_link_options(rt_lib PUBLIC $<$<CONFIG:RELEASE>:-Wall -ffast-math -march=native -lm>)

    target_compile_options(rt_lib PUBLIC $<$<CONFIG:PROFILE>:-Wall -g -O3 -ffast-math -march=native>)
    target_link_options(rt_lib PUBLIC $<$<CONFIG:PROFILE>:-Wall -g -O3 -ffast-math -march=native -lm>)
else()
    target_compile_options(rt_lib PUBLIC $<$<CONFIG:DEBUG>:-Wall -fsanitize=address -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=implicit-conversion -march=native>)
    target_link_options(rt_lib PUBLIC $<$<CONFIG:DEBUG>:-Wall -fsanitize=address -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=implicit-conversion -lubsan -march=native -lm>)

    target_compile_options(rt_lib PUBLIC $<$<CONFIG:RELEASE>:-Wall -ffast-math -march=native>)
    target_link_options(rt_lib PUBLIC $<$<CONFIG:RELEASE>:-Wall -ffast-math -march=native -lm>)

    target_compile_options(rt_lib PUBLIC $<$<CONFIG:PROFILE>:-Wall -g -O3 -ffast-math -march=native>)
    target_link_options(rt_lib PUBLIC $<$<CONFIG:PROFILE>:-Wall -g -O3 -ffast-math -march=native -lm>)
endif()

if(DEFINED _SPPM_RADIUS_MULT)
    target_compile_definitions(rt_alg PRIVATE _SPPM_RADIUS_MULT=${_SPPM_RADIUS_MULT})
endif()
if(DEFINED _SPPM_RADIUS_TYPE)
    target_compile_definitions(rt_alg PRIVATE _SPPM_RADIUS_TYPE=${_SPPM_RADIUS_TYPE})
endif()

