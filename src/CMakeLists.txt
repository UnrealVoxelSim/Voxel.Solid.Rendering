add_library(
    ${PROJECT_NAME}
    UnrealVoxelSim/Voxel/Solid/Rendering/GreedyMesher.cpp
    UnrealVoxelSim/Voxel/Solid/Rendering/Sampler.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/public>"
        "$<INSTALL_INTERFACE:include>"
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        UnrealVoxelSim.Voxel.Api
        UnrealVoxelSim.Voxel.Rendering.Api
        UnrealVoxelSim.Voxel.Solid.Api
)

UnrealVoxelSimConfigureTarget(${PROJECT_NAME})
