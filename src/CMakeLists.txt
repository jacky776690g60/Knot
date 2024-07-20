# Add executables
add_executable(encrypter encrypter.cpp)
add_executable(decrypter decrypter.cpp)
add_executable(cleaner cleaner.cpp)

# Set output directory for executables
set_target_properties(encrypter decrypter cleaner
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)