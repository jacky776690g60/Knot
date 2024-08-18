# Add executables
add_executable(encrypter encrypter.cpp)
add_executable(decrypter decrypter.cpp)
add_executable(cleaner cleaner.cpp)

# set(CMAKE_PDB_OUTPUT_DIRECTORY "") # remove .pdb files. not recommended

# Set output directory for executables
set_target_properties(encrypter decrypter cleaner
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}"
)