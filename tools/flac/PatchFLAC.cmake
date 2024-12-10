# prevent FLAC from adding stuff that we don't use
file(READ "${FLAC_DIR}/src/CMakeLists.txt" FLAC_SRC_CMAKELISTS_CONTENTS)
string(REGEX REPLACE "\nadd_subdirectory\\(\"share/" "\n\#add_subdirectory(\"share/" FLAC_SRC_CMAKELISTS_CONTENTS "${FLAC_SRC_CMAKELISTS_CONTENTS}")
file(WRITE "${FLAC_DIR}/src/CMakeLists.txt" "${FLAC_SRC_CMAKELISTS_CONTENTS}")

file(READ "${FLAC_DIR}/CMakeLists.txt" FLAC_CMAKELISTS_CONTENTS)
string(REPLACE "set_target_properties(FLAC grabbag getopt replaygain_analysis replaygain_synthesis utf8 PROPERTIES FOLDER Libraries)" "" FLAC_CMAKELISTS_CONTENTS "${FLAC_CMAKELISTS_CONTENTS}")
string(REPLACE "install(FILES \${FLAC++_HEADERS} DESTINATION \"\${CMAKE_INSTALL_INCLUDEDIR}/FLAC++\")" "" FLAC_CMAKELISTS_CONTENTS "${FLAC_CMAKELISTS_CONTENTS}")
string(REPLACE "\n\nadd_subdirectory(\"src\")" "\nset(CMAKE_DEBUG_POSTFIX d)\nadd_subdirectory(\"src\")" FLAC_CMAKELISTS_CONTENTS "${FLAC_CMAKELISTS_CONTENTS}")
file(WRITE "${FLAC_DIR}/CMakeLists.txt" "${FLAC_CMAKELISTS_CONTENTS}")
