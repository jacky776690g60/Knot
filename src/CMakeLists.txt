# ================================================================
# CMakeLists.txt  --  Knot/src/CMakeLists.txt
# ================================================================
#### Set dynamic linking for OpenSSL
set(OPENSSL_USE_STATIC_LIBS FALSE)

find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
if(OpenSSL_FOUND)
    message(STATUS "OpenSSL found: ${OpenSSL_VERSION}")
    message(STATUS "OPENSSL_ROOT_DIR: ${OPENSSL_ROOT_DIR}")
    message(STATUS "OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
    include_directories(${OPENSSL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "OpenSSL not found!")
endif()

add_executable(encrypter encrypter.cpp)
add_executable(decrypter decrypter.cpp)
add_executable(cleaner   cleaner.cpp)

target_link_libraries(encrypter PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(decrypter PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(cleaner   PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# Set output directories
set_target_properties(encrypter decrypter cleaner
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY         "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}"
)

# -------------------------------------------------
# Auto-Copying Dynamic Libraries
# -------------------------------------------------
function(copy_library target lib_path)
    if(EXISTS "${lib_path}")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${lib_path}"
                "${OUTPUT_DIR}"
            COMMENT "Copying ${lib_path} to ${OUTPUT_DIR}"
        )
    else()
        message(WARNING "Library ${lib_path} does not exist and will not be copied.")
    endif()
endfunction()


set(EXECUTABLES encrypter decrypter cleaner)
# Platform-specific library copying
if(MSVC)
    # foreach(exec IN LISTS EXECUTABLES)
    #     ...
    #     # Copy OpenSSL DLLs
    #     copy_library(${exec} "${OPENSSL_SSL_DLL}")
    #     copy_library(${exec} "${OPENSSL_CRYPTO_DLL}")
    # endforeach()
elseif(UNIX AND NOT APPLE)
    # ...
elseif(APPLE)
    foreach(exec IN LISTS EXECUTABLES)
        # Use OpenSSL_LIBRARIES to get the paths
        list(GET OPENSSL_LIBRARIES 0 OPENSSL_SSL_DYLIB)
        list(GET OPENSSL_LIBRARIES 1 OPENSSL_CRYPTO_DYLIB)
        message(STATUS "OPENSSL_SSL_DYLIB for ${exec}: ${OPENSSL_SSL_DYLIB}")
        message(STATUS "OPENSSL_CRYPTO_DYLIB for ${exec}: ${OPENSSL_CRYPTO_DYLIB}")
        # Copy OpenSSL .dylib files
        copy_library(${exec} "${OPENSSL_SSL_DYLIB}")
        copy_library(${exec} "${OPENSSL_CRYPTO_DYLIB}")
    endforeach()

    # # Adjust RPATH using install_name_tool
    # foreach(exec IN LISTS EXECUTABLES)
    #     # Replace the hard-coded paths with @executable_path
    #     add_custom_command(TARGET ${exec} POST_BUILD
    #         COMMAND install_name_tool -change "${OPENSSL_SSL_DYLIB}" "@executable_path/libssl.dylib" "${OUTPUT_DIR}/${exec}"
    #         COMMENT "Adjusting RPATH for libssl.dylib in ${exec}"
    #     )
    #     add_custom_command(TARGET ${exec} POST_BUILD
    #         COMMAND install_name_tool -change "${OPENSSL_CRYPTO_DYLIB}" "@executable_path/libcrypto.dylib" "${OUTPUT_DIR}/${exec}"
    #         COMMENT "Adjusting RPATH for libcrypto.dylib in ${exec}"
    #     )
    # endforeach()
endif()
