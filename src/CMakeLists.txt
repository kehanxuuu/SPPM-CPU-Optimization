aux_source_directory(. SOURCES)
aux_source_directory(lib LIB_SOURCES)

add_executable(photon_mapper ${SOURCES} ${LIB_SOURCES})