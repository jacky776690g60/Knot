#### Find external libraries
### ==== fills gaps in the C++ Standard Library
find_package(Boost 1.70.0 REQUIRED COMPONENTS filesystem system)
if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_VERSION}")
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost not found!")
endif()
###
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    message(STATUS "OpenSSL found: ${OPENSSL_VERSION}")
    include_directories(${OPENSSL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "OpenSSL not found!")
endif()




### Add executables
add_executable(encrypter encrypter.cpp)
add_executable(decrypter decrypter.cpp)
add_executable(cleaner cleaner.cpp)


# set(CMAKE_PDB_OUTPUT_DIRECTORY "") # remove .pdb files. not recommended


### Link external libraries to executables
target_link_libraries(encrypter PRIVATE Boost::filesystem Boost::system OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(decrypter PRIVATE Boost::filesystem Boost::system OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(cleaner PRIVATE Boost::filesystem Boost::system OpenSSL::SSL OpenSSL::Crypto)



### Set output directory for executables
set_target_properties(encrypter decrypter cleaner
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY         "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}"
)



### Function to copy runtime dependencies (e.g., DLLs on Windows)
### ====== manual here (or use CMake's GetRuntimeDependencies)
function(copy_runtime_dependencies target)
    if (WIN32)
        set(OPENSSL_DLL_DIR "${OpenSSL_DIR}/bin") # Example path
        # List of OpenSSL DLLs to copy
        set(OPENSSL_DLLS
            "libssl-1_1-x64.dll"
            "libcrypto-1_1-x64.dll"
        )

        foreach(dll ${OPENSSL_DLLS})
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${OPENSSL_DLL_DIR}/${dll}"
                    "${OUTPUT_DIR}"
            )
        endforeach()


        if(Boost_USE_STATIC_LIBS)
            # If using static libraries, no need to copy DLLs
        else()
            # List Boost DLLs to copy
            set(BOOST_DLLS
                "libboost_filesystem-vc142-mt-x64-1_70.dll"
                "libboost_system-vc142-mt-x64-1_70.dll"
            )

            foreach(dll ${BOOST_DLLS})
                add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${Boost_LIBRARY_DIRS}/${dll}"
                        "${OUTPUT_DIR}"
                )
            endforeach()
        endif()
    elseif(UNIX)
        execute_process(COMMAND ${CMAKE_COMMAND} -E echo "Copying OpenSSL shared libraries (UNIX).")
    endif()
endfunction()

### Apply the helper function to each executable
copy_runtime_dependencies(encrypter)
copy_runtime_dependencies(decrypter)
copy_runtime_dependencies(cleaner)


# Optionally, set RPATH on Unix to find shared libraries at runtime
if(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()