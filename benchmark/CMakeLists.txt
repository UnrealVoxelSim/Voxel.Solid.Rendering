find_package(benchmark CONFIG REQUIRED)

add_executable(
    ${PROJECT_NAME}.Benchmark
    UnrealVoxelSim/Voxel/Solid/Rendering/GreedyMesherBenchmark.cpp
)

target_link_libraries(
    ${PROJECT_NAME}.Benchmark
    PRIVATE
        ${PROJECT_NAME}
        benchmark::benchmark_main
)

UnrealVoxelSimConfigureTarget(${PROJECT_NAME}.Benchmark)
