#include "UnrealVoxelSim/Voxel/Solid/Rendering/GreedyMesher.h"

#include <benchmark/benchmark.h>

#include <cstdint>

namespace UnrealVoxelSim::Voxel::Solid::Rendering
{

namespace
{

[[nodiscard]] UnrealVoxelSim::Voxel::Rendering::Api::Snapshot FlatSnapshot()
{
    UnrealVoxelSim::Voxel::Rendering::Api::Snapshot snapshot;
    snapshot.Target = {{0, 0, 0}, {32, 32, 32}};
    snapshot.Samples = {{-1, -1, -1}, {33, 33, 33}};
    snapshot.Cells.resize(*snapshot.Samples.CellCount());

    std::size_t index{};
    for (auto z = snapshot.Samples.Min.Z; z < snapshot.Samples.Max.Z; ++z)
    {
        for (auto y = snapshot.Samples.Min.Y; y < snapshot.Samples.Max.Y; ++y)
        {
            for (auto x = snapshot.Samples.Min.X; x < snapshot.Samples.Max.X; ++x)
            {
                static_cast<void>(x);
                static_cast<void>(y);
                snapshot.Cells[index++] = UnrealVoxelSim::Voxel::Rendering::Api::SurfaceId{z < 16 ? 1U : 0U};
            }
        }
    }
    return snapshot;
}

[[nodiscard]] UnrealVoxelSim::Voxel::Rendering::Api::Snapshot FragmentedSnapshot()
{
    auto snapshot = FlatSnapshot();
    std::size_t index{};
    for (auto z = snapshot.Samples.Min.Z; z < snapshot.Samples.Max.Z; ++z)
    {
        for (auto y = snapshot.Samples.Min.Y; y < snapshot.Samples.Max.Y; ++y)
        {
            for (auto x = snapshot.Samples.Min.X; x < snapshot.Samples.Max.X; ++x)
            {
                const auto hash = static_cast<std::uint32_t>(x * 73856093 ^ y * 19349663 ^ z * 83492791);
                snapshot.Cells[index++] =
                    UnrealVoxelSim::Voxel::Rendering::Api::SurfaceId{(hash & 3U) == 0U ? (hash % 3U) + 1U : 0U};
            }
        }
    }
    return snapshot;
}

void FlatTerrain(benchmark::State &state)
{
    const auto snapshot = FlatSnapshot();
    const GreedyMesher mesher;
    for (auto _ : state)
    {
        const auto mesh = mesher.Build(snapshot);
        benchmark::DoNotOptimize(mesh->Vertices.data());
        benchmark::ClobberMemory();
    }
    const auto mesh = mesher.Build(snapshot);
    state.counters["vertices"] = static_cast<double>(mesh->Vertices.size());
    state.counters["triangles"] = static_cast<double>(mesh->Indices.size() / 3);
}

void FragmentedTerrain(benchmark::State &state)
{
    const auto snapshot = FragmentedSnapshot();
    const GreedyMesher mesher;
    for (auto _ : state)
    {
        const auto mesh = mesher.Build(snapshot);
        benchmark::DoNotOptimize(mesh->Vertices.data());
        benchmark::ClobberMemory();
    }
    const auto mesh = mesher.Build(snapshot);
    state.counters["vertices"] = static_cast<double>(mesh->Vertices.size());
    state.counters["triangles"] = static_cast<double>(mesh->Indices.size() / 3);
}

BENCHMARK(FlatTerrain)->Unit(benchmark::kMicrosecond);
BENCHMARK(FragmentedTerrain)->Unit(benchmark::kMicrosecond);

}

}
