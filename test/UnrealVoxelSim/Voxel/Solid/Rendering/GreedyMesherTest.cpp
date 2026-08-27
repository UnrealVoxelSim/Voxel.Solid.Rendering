#include "UnrealVoxelSim/Voxel/Solid/Rendering/GreedyMesher.h"

#include <gtest/gtest.h>

#include <algorithm>

namespace UnrealVoxelSim::Voxel::Solid::Rendering
{

namespace
{

using UnrealVoxelSim::Voxel::Api::Region;
using UnrealVoxelSim::Voxel::Rendering::Api::Snapshot;
using UnrealVoxelSim::Voxel::Rendering::Api::SurfaceId;

[[nodiscard]] Snapshot FilledSnapshot(const Region target, const Region samples, const SurfaceId surface)
{
    return Snapshot{target, samples, std::vector<SurfaceId>(*samples.CellCount(), surface)};
}

}

TEST(GreedyMesherTest, SingleCellProducesSixQuads)
{
    const auto snapshot = FilledSnapshot({{0, 0, 0}, {1, 1, 1}}, {{0, 0, 0}, {1, 1, 1}}, SurfaceId{1});

    const auto result = GreedyMesher{}.Build(snapshot);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->Vertices.size(), 24U);
    EXPECT_EQ(result->Indices.size(), 36U);
}

TEST(GreedyMesherTest, MergesAUniformRunIntoSixQuads)
{
    const auto snapshot = FilledSnapshot({{0, 0, 0}, {4, 1, 1}}, {{0, 0, 0}, {4, 1, 1}}, SurfaceId{2});

    const auto result = GreedyMesher{}.Build(snapshot);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->Vertices.size(), 24U);
    EXPECT_EQ(result->Indices.size(), 36U);
}

TEST(GreedyMesherTest, MaterialChangesSplitVisibleQuadsButNotInternalFaces)
{
    auto snapshot = FilledSnapshot({{0, 0, 0}, {2, 1, 1}}, {{0, 0, 0}, {2, 1, 1}}, SurfaceId{1});
    snapshot.Cells[1] = SurfaceId{2};

    const auto result = GreedyMesher{}.Build(snapshot);

    ASSERT_TRUE(result);
    EXPECT_EQ(result->Vertices.size(), 40U);
    EXPECT_EQ(result->Indices.size(), 60U);
}

TEST(GreedyMesherTest, OccupiedHaloSuppressesFacesOwnedByNeighboringTiles)
{
    const auto snapshot = FilledSnapshot({{1, 1, 1}, {2, 2, 2}}, {{0, 0, 0}, {3, 3, 3}}, SurfaceId{3});

    const auto result = GreedyMesher{}.Build(snapshot);

    ASSERT_TRUE(result);
    EXPECT_TRUE(result->Vertices.empty());
    EXPECT_TRUE(result->Indices.empty());
}

TEST(GreedyMesherTest, RejectsInconsistentSnapshotStorage)
{
    auto snapshot = FilledSnapshot({{0, 0, 0}, {1, 1, 1}}, {{0, 0, 0}, {1, 1, 1}}, SurfaceId{1});
    snapshot.Cells.clear();

    const auto result = GreedyMesher{}.Build(snapshot);

    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), BuildError::InvalidSnapshot);
}

}
