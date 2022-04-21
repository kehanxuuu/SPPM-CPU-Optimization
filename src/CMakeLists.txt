add_subdirectory(ext)

include_directories(${OPENEXR_INCLUDE_DIRS})
aux_source_directory(lib LIB_SOURCES)
aux_source_directory(algorithm ALG_SOURCES)
add_library(rt_lib ${LIB_SOURCES})
add_library(rt_alg ${ALG_SOURCES})
target_link_libraries(rt_lib IlmImf)
target_link_libraries(rt_alg rt_lib)

include_directories(lib)
include_directories(algorithm)
aux_source_directory(. SOURCES)
add_executable(main ${SOURCES})
target_link_libraries(main rt_lib)
target_link_libraries(main rt_alg)

target_compile_options(main PUBLIC $<$<CONFIG:RELEASE>:-Wall>)

target_compile_options(main PUBLIC $<$<CONFIG:DEBUG>:-Wall -fsanitize=address -fsanitize=undefined>)

target_link_options(main PUBLIC $<$<CONFIG:RELEASE>:-Wall>)

target_link_options(main PUBLIC $<$<CONFIG:DEBUG>:-Wall -fsanitize=address -fsanitize=undefined>)