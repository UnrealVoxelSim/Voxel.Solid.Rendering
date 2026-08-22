find_package(GTest CONFIG REQUIRED)

add_executable(
    ${PROJECT_NAME}.Test
    UnrealVoxelSim/Voxel/Solid/Rendering/GreedyMesherTest.cpp
    UnrealVoxelSim/Voxel/Solid/Rendering/SamplerTest.cpp
)

target_link_libraries(
    ${PROJECT_NAME}.Test
    PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
)

UnrealVoxelSimConfigureTarget(${PROJECT_NAME}.Test)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}.Test)
