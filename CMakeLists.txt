cmake_minimum_required(VERSION 3.5.1)
project(dpdkLearning)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
#set(CMAKE_CXX_FLAGS_DEBUG “${CMAKE_CXX_FLAGS_DEBUG} -O0”)
#set(CMAKE_CXX_STANDARD 14)

#if(CMAKE_COMPILER_IS_GNUCXX)
#    #    add_compile_options(-std=c++11)
#    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
#    message(STATUS "optional:-std=c++11")
#endif(CMAKE_COMPILER_IS_GNUCXX)

set(RTE_SDK /home/king/dpdk-stable-19.08.2/)
set(RTE_TARGET x86_64-native-linux-gcc)

set(SOURCE_FILE
        main.c)

set(WERROR_FLAGS
        -W
        -Wall
        #        -Wstrict-prototypes
        #        -Wmissing-prototypes
        -Wmissing-declarations
        #        -Wold-style-definition
        -Wpointer-arith
        -Wcast-align
        #        -Wnested-externs
        -Wcast-qual
        -Wformat-nonliteral
        -Wformat-security
        -Wundef
        -Wwrite-strings
        -Wdeprecated)

set(CFLAGS
        -m64
        -pthread
        -march=native
        #-include ${RTE_SDK}/${RTE_TARGET}/include/rte_config.h
        -O3
        ${WERROR_FLAGS})

set(DPDKLIBS
        -Wl,--whole-archive
        -ldpdk
        -Wl,--no-whole-archive)

add_definitions(
        -DRTE_MACHINE_CPUFLAG_SSE
        -DRTE_MACHINE_CPUFLAG_SSE2
        -DRTE_MACHINE_CPUFLAG_SSE3
        -DRTE_MACHINE_CPUFLAG_SSSE3
        -DRTE_MACHINE_CPUFLAG_SSE4_1
        -DRTE_MACHINE_CPUFLAG_SSE4_2
        -DRTE_MACHINE_CPUFLAG_AES
        -DRTE_MACHINE_CPUFLAG_PCLMULQDQ
        -DRTE_MACHINE_CPUFLAG_AVX
        -DRTE_MACHINE_CPUFLAG_RDRAND
        -DRTE_MACHINE_CPUFLAG_FSGSBASE
        -DRTE_MACHINE_CPUFLAG_F16C
        -DRTE_MACHINE_CPUFLAG_AVX2
        -D_GNU_SOURCE)

#add_compile_options(-fno-elide-constructors) #关闭编译器优化
add_compile_options(${CFLAGS})

include_directories(SYSTEM "/foo/bar")

include_directories(${PROJECT_SOURCE_DIR}/Assemble/)
include_directories(${PROJECT_SOURCE_DIR}/Disassemble/)
include_directories(${PROJECT_SOURCE_DIR}/include/)

include_directories(${RTE_SDK}/${RTE_TARGET}/include)
link_directories(${RTE_SDK}/${RTE_TARGET}/lib)
link_libraries(
        ${DPDKLIBS}
        pthread
        rt
        dl
        m
        numa
        pcap)

add_executable(dpdkLearning ${SOURCE_FILE})

#target_link_libraries(
#        dpdkLearning)

#ADD_SUBDIRECTORY(Assemble )
#add_subdirectory(Disassemble)
