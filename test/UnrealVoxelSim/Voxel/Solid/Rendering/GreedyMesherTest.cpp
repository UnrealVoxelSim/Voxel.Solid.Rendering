#include "UnrealVoxelSim/Voxel/Solid/Rendering/GreedyMesher.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <utility>
#include <vector>

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

TEST(GreedyMesherTest, EmitsCellSpaceUvsThatRepeatAcrossMergedQuads)
{
    const auto snapshot = FilledSnapshot({{0, 0, 0}, {4, 1, 1}}, {{0, 0, 0}, {4, 1, 1}}, SurfaceId{2});

    const auto result = GreedyMesher{}.Build(snapshot);

    ASSERT_TRUE(result);
    std::vector<std::pair<float, float>> topFaceCoordinates;
    for (const auto& vertex : result->Vertices)
    {
        if (vertex.NormalZ == 1)
        {
            topFaceCoordinates.emplace_back(vertex.U, vertex.V);
        }
    }

    ASSERT_EQ(topFaceCoordinates.size(), 4U);
    EXPECT_EQ(topFaceCoordinates,
              (std::vector<std::pair<float, float>>{{0.0F, 0.0F}, {4.0F, 0.0F}, {4.0F, 1.0F}, {0.0F, 1.0F}}));
}

TEST(GreedyMesherTest, MapsGameUpToTextureVOnEveryVerticalFace)
{
    const auto snapshot = FilledSnapshot({{0, 0, 0}, {1, 1, 2}}, {{0, 0, 0}, {1, 1, 2}}, SurfaceId{2});

    const auto result = GreedyMesher{}.Build(snapshot);

    ASSERT_TRUE(result);
    for (const auto &vertex : result->Vertices)
    {
        if (vertex.NormalX == 0 && vertex.NormalY == 0)
        {
            continue;
        }
        EXPECT_FLOAT_EQ(vertex.V, static_cast<float>(vertex.Z));
    }
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
