cmake_minimum_required(VERSION 3.10.2)

project(vulkan_lib 
	LANGUAGES C CXX
)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
if(COMPILER_SUPPORTS_CXX17)
	message(STATUS "use c++17")
	set(CMAKE_CXX_STANDARD 17)
elseif(COMPILER_SUPPORTS_CXX14)
	message(STATUS "use c++14")
	set(CMAKE_CXX_STANDARD 14)
else()
	message(STATUS "use c++11")
	set(CMAKE_CXX_STANDARD 11)
endif()

include(CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG("-std=c17" COMPILER_SUPPORTS_C17)
CHECK_C_COMPILER_FLAG("-std=c14" COMPILER_SUPPORTS_C14)
if(COMPILER_SUPPORTS_C17)
	message(STATUS "use c17")
	set(CMAKE_C_STANDARD 17)
elseif(COMPILER_SUPPORTS_C14)
	message(STATUS "use c14")
	set(CMAKE_C_STANDARD 14)
else()
	message(STATUS "use c11")
	set(CMAKE_C_STANDARD 11)
endif()

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -g -ggdb")
set(DCMAKE_SYSTEM_PROCESSOR "arm64")
set(DCMAKE_OSX_ARCHITECTURES "arm64")

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install CACHE PATH "Default prefix" FORCE)

set(VULKAN_ROOT /Users/wangxin/vulkan)

set(MOLTENVK_INCLUDE ${VULKAN_ROOT}/sdk/MoltenVK/include)
set(MOLTENVK_LIB_MACOS ${VULKAN_ROOT}/sdk/MoltenVK/dylib/macOS)
set(VULKAN_LIB_MACOS ${VULKAN_ROOT}/sdk/macOS/lib)
set(MOLTENVK_FRAMEWORK_MACOS ${VULKAN_ROOT}/sdk/MoltenVK/MoltenVK.xcframework/macos-arm64_x86_64)

add_subdirectory(Sources)
add_subdirectory(ThirdParty)
add_subdirectory(example)

# function
function(add_module name)
	
endfunction(add_module name)
