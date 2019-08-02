# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 11)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library(
        # Sets the name of the library.
        MyNativeGLInterface

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/jni/MyNativeGLInterface.cpp
)

add_library(
        mmdviewer

        SHARED

        src/main/jni/gl/objects/cube.cpp
        src/main/jni/gl/objects/globject.cpp
        src/main/jni/gl/abscamera.cpp
        src/main/jni/gl/application.cpp
        src/main/jni/gl/renderer.cpp
        src/main/jni/gl/scenecamera.cpp
        src/main/jni/gl/shaderloader.cpp
        src/main/jni/gl/environment.cpp
        src/main/jni/gl/glworld.cpp
        src/main/jni/gl/texture/texture.cpp
        src/main/jni/gl/texture/texture_jpeg.cpp
        src/main/jni/gl/texture/texture_png.cpp
        src/main/jni/gl/texture/texture_bmp.cpp
        src/main/jni/gl/texture/texture_tga.cpp
        src/main/jni/gl/texture/texture_load_exception.cpp
        src/main/jni/matrix/matrix.c
        src/main/jni/mmd/pmx/pmxvertex.cpp
        src/main/jni/mmd/pmx/pmxtexture.cpp
        src/main/jni/mmd/pmx/pmxmaterial.cpp
        src/main/jni/mmd/pmx/pmxbone.cpp
        src/main/jni/mmd/pmx/pmxmorph.cpp
        src/main/jni/mmd/pmx/pmxreader.cpp
        src/main/jni/mmd/pmx/pmxobject.cpp
        src/main/jni/mmd/pmx/pmx_exception.cpp
        src/main/jni/mmd/pmx/pmxobject_transform_ik.cpp
        src/main/jni/mmd/pmx/pmxobject_transform.cpp
        src/main/jni/mmd/pmx/pmxobject_shader.cpp
        src/main/jni/mmd/pmx/pmxobject_morph.cpp
        src/main/jni/mmd/pmx/pmxobject_shader_holder.cpp
        src/main/jni/mmd/vmd/vmdreader.cpp
        src/main/jni/mmd/vmd/vmdkeyframe.cpp
        src/main/jni/mmd/vmd/bonekeyframe.cpp
        src/main/jni/mmd/vmd/morphkeyframe.cpp
        src/main/jni/mmd/vmd/camerakeyframe.cpp
        src/main/jni/mmd/vmd/lightkeyframe.cpp
        src/main/jni/mmd/vmd/ikkeyframe.cpp
        src/main/jni/utils/mstring.cpp
        src/main/jni/utils/mathutils.cpp
        src/main/jni/vector/vector.c
        src/main/jni/quaternion/quaternion.c
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

#find_library( # Sets the name of the path variable.
#              log-lib
#
#               Specifies the name of the NDK library that
#               you want CMake to locate.
#              log )

add_library(
        jpeg

        SHARED

        IMPORTED
)

set_target_properties(
        jpeg

        PROPERTIES

        IMPORTED_LOCATION

        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libjpeg.so
)

add_library(
        turbojpeg

        SHARED

        IMPORTED
)

set_target_properties(
        turbojpeg

        PROPERTIES

        IMPORTED_LOCATION

        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libturbojpeg.so
)

add_library(
        png

        SHARED

        IMPORTED
)

set_target_properties(
        png

        PROPERTIES

        IMPORTED_LOCATION

        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libpng.so
)

add_library(
        png16

        SHARED

        IMPORTED
)

set_target_properties(
        png16

        PROPERTIES

        IMPORTED_LOCATION

        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libpng16.so
)

add_library(
        z

        SHARED

        IMPORTED
)

set_target_properties(
        z

        PROPERTIES

        IMPORTED_LOCATION

        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libz.so
)

add_library(
        charset

        SHARED

        IMPORTED
)

set_target_properties(
        charset

        PROPERTIES

        IMPORTED_LOCATION

        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libcharset.so
)

add_library(
        iconv

        SHARED

        IMPORTED
)

set_target_properties(
        iconv

        PROPERTIES

        IMPORTED_LOCATION

        ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libiconv.so
)

find_library(
        GLESv3

        GLESv3
)

find_library(
        log

        log
)

find_library(
        android

        android
)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries(
        mmdviewer

        jpeg
        turbojpeg
        png
        png16
        z
        charset
        iconv
        GLESv3
        log
        android
)

target_link_libraries( # Specifies the target library.
        MyNativeGLInterface

        # Links the target library to the log library
        # included in the NDK. == ldLibs
        #${log-lib}
        mmdviewer
)