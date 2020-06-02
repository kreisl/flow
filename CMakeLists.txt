cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0048 NEW)

if ("${CMAKE_VERSION}" VERSION_GREATER "3.12.0")
    cmake_policy(SET CMP0074 NEW)
endif ()
project(flow VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_compile_options("-Wall" "-Wextra"
        "$<$<CONFIG:RELEASE>:-O3;-march=native;-ftree-vectorize;-ffast-math>"
        "$<$<CONFIG:DEBUG>:-O0;-O0>")
if ("${CMAKE_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options("-stdlib=libc++")
endif ()

set(CORRECTION_SOURCES
        Correction/CorrectionHistogramBase.cpp
        Correction/CorrectionHistogramChannelizedSparse.cpp
        Correction/CorrectionHistogramSparse.cpp
        Correction/GainEqualization.cpp
        Correction/CorrectionProfile3DCorrelations.cpp
        Correction/CorrectionProfileChannelized.cpp
        Correction/CorrectionProfileChannelizedIngress.cpp
        Correction/CorrectionProfileComponents.cpp
        Correction/CorrectionProfileCorrelationComponents.cpp
        Correction/SubEvent.cpp
        Correction/SubEventChannels.cpp
        Correction/Alignment.cpp
        Correction/SubEventTracks.cpp
        Correction/Recentering.cpp
        Correction/TwistAndRescale.cpp
        Correction/CorrectionManager.cpp
        Correction/QAHistogram.cpp
        Correction/Detector.cpp)

set(CORRECTION_HEADERS
        CorrectionManager.h
        CorrectionOnInputData.h
        CorrectionOnQnVector.h
        CorrectionAxis.h
        CorrectionAxisSet.h
        CorrectionBase.h
        CorrectionHistogramBase.h
        CorrectionsSet.h
        CorrectionHistogramChannelizedSparse.h
        CorrectionHistogramSparse.h
        CorrectionProfile3DCorrelations.h
        CorrectionProfileChannelized.h
        CorrectionProfileChannelizedIngress.h
        CorrectionDataVector.h
        CorrectionProfileComponents.h
        CorrectionProfileCorrelationComponents.h
        SubEvent.h
        SubEventChannels.h
        SubEventTracks.h
        GainEqualization.h
        Alignment.h
        Recentering.h
        TwistAndRescale.h
        RunList.h
        DetectorList.h
        InputVariable.h
        QAHistogram.h
        CorrectionFillHelper.h
        )

set(BASE_SOURCES
        Base/QVector.cpp
        Base/DataContainerHelper.cpp
        Base/ReSamples.cpp
        Base/EventShape.cpp
        Base/Stats.cpp
        Base/Statistic.cpp
        )

set(BASE_HEADERS DataContainer.h
        DataContainerHelper.h
        Axis.h
        QVector.h
        ReSamples.h
        CorrelationResult.h
        Stats.h
        Cuts.h
        Statistic.h
        EqualEntriesBinner.h
        )

set(CORRELATION_SOURCES
        )

set(CORRELATION_HEADERS
        AxesConfiguration.h
        CorrelationHelper.h
        Correlation.h
        ReSampler.h
        TemplateHelpers.h
        )

set (TOYMC_SOURCES
        ToyMC/ParticleGenerator.cpp
        ToyMC/ChannelDetector.cpp)
set (TOYMC_HEADERS
        ToyMC/ParticleGenerator.h
        ToyMC/include/ChannelDetector.h
        )

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# ROOT
find_package(ROOT REQUIRED COMPONENTS Core MathCore MathMore RIO Hist Tree Net TreePlayer)
include(${ROOT_USE_FILE})
message(STATUS "Using ROOT: ${ROOT_VERSION} <${ROOT_CONFIG}>")

set(ROOTCLING ${ROOT_DIR}/../bin/rootcling)

#Base
add_custom_command(OUTPUT G__BASE.cxx
        COMMAND ${ROOTCLING} -v -f G__BASE.cxx
        -rml libBase${CMAKE_SHARED_LIBRARY_SUFFIX}
        -rmf libBase.rootmap
        -s libBase
        -I${CMAKE_SOURCE_DIR}/Base -I${CMAKE_SOURCE_DIR}/Base/include
        ${BASE_HEADERS}
        ${CMAKE_SOURCE_DIR}/Base/BaseLinkDef.h
        )
add_library(Base SHARED ${BASE_SOURCES} G__BASE.cxx)
add_library(Qn::Base ALIAS Base)
target_compile_definitions(Base PUBLIC "-DUSE_ROOT")
target_include_directories(Base
        PRIVATE
        ${ROOT_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/Base
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Base/include>
        )
target_link_libraries(Base PRIVATE ${ROOT_LIBRARIES})

#CorrectionStep
add_custom_command(OUTPUT G__Correction.cxx
        COMMAND ${ROOTCLING} -v -f G__Correction.cxx
        -rml libCorrection${CMAKE_SHARED_LIBRARY_SUFFIX}
        -rmf libCorrection.rootmap
        -s libCorrection
        -I${CMAKE_SOURCE_DIR}/Correction/include -I${CMAKE_SOURCE_DIR}/Correction -I${CMAKE_SOURCE_DIR}/Base -I${CMAKE_SOURCE_DIR}/Base/include
        ${CORRECTION_HEADERS}
        ${CMAKE_SOURCE_DIR}/Correction/CorrectionLinkDef.h
        )
add_library(Correction SHARED ${CORRECTION_SOURCES} G__Correction.cxx)
add_library(Qn::Correction ALIAS Correction)
target_compile_definitions(Correction PUBLIC "-DUSE_ROOT")
target_include_directories(Correction
        PRIVATE
        ${ROOT_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/Correction
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Correction/include>
        )
target_link_libraries(Correction PUBLIC Base PRIVATE ${ROOT_LIBRARIES})

#DataFrameCorrelation
add_custom_command(OUTPUT G__Correlation.cxx
        COMMAND ${ROOTCLING} -v -f G__Correlation.cxx
        -rml libCorrelation${CMAKE_SHARED_LIBRARY_SUFFIX}
        -rmf libCorrelation.rootmap
        -s libCorrelation
        -I${CMAKE_SOURCE_DIR}/Correlation -I${CMAKE_SOURCE_DIR}/Correlation/include  -I${CMAKE_SOURCE_DIR}/Base/include -I${CMAKE_SOURCE_DIR}/Base
        ${CORRELATION_HEADERS}
        ${CMAKE_SOURCE_DIR}/Correlation/CorrelationLinkDef.h
        )
add_library(Correlation SHARED ${CORRELATION_SOURCES} G__Correlation)
add_library(Qn::Correlation ALIAS Correlation)
target_compile_definitions(Correlation PUBLIC "-DUSE_ROOT")
target_include_directories(Correlation
        PRIVATE
        ${ROOT_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/Correlation
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Correlation/include>
        )
target_link_libraries(Correlation PUBLIC Base PRIVATE ${ROOT_LIBRARIES})

# ToyMC
add_library(ToyMC SHARED ${TOYMC_SOURCES})
add_library(Qn::ToyMC ALIAS ToyMC)
target_compile_definitions(ToyMC PUBLIC "-DUSE_ROOT")
target_include_directories(ToyMC
        PRIVATE
        ${ROOT_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/ToyMC
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/ToyMC/include>
        )
target_link_libraries(ToyMC PUBLIC Base Correlation Correction PRIVATE ${ROOT_LIBRARIES})

IF (CMAKE_BUILD_TYPE MATCHES DEBUG)
    # Googletest
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
    if (result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif ()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
    if (result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif ()
    # Prevent overriding the parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src ${CMAKE_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)
    add_subdirectory(test)
ENDIF (CMAKE_BUILD_TYPE MATCHES DEBUG)

add_executable(main main.cpp)
target_link_libraries(main ${ROOT_LIBRARIES} ROOTVecOps Base Correlation ToyMC Correction)
#
# Install configuration

set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)

install(DIRECTORY Base/include/ Correction/include/ Correlation/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# Installion of ROOT dictionaries
install(FILES ${CMAKE_BINARY_DIR}/libBase_rdict.pcm DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/libBase.rootmap DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/libCorrection_rdict.pcm DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/libCorrection.rootmap DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/libCorrelation_rdict.pcm DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/libCorrelation.rootmap DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS Base Correction Correlation
        EXPORT QnTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

set(INSTALL_CONFIGDIR lib/cmake/Qn)
install(EXPORT QnTargets
        FILE QnTargets.cmake
        NAMESPACE Qn::
        DESTINATION ${INSTALL_CONFIGDIR}
        )
#Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/QnConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/QnConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/QnConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR})
#Install the config, configversion
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/QnConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/QnConfigVersion.cmake
        DESTINATION ${INSTALL_CONFIGDIR})

export(TARGETS Base Correction Correlation NAMESPACE Qn:: FILE QnTargets.cmake)
export(PACKAGE Qn)


