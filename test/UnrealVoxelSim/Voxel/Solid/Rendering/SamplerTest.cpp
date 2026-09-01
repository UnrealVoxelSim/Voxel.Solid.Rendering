#include "UnrealVoxelSim/Voxel/Solid/Rendering/Sampler.h"

#include "UnrealVoxelSim/Voxel/Solid/Api/StandardMaterials.h"

#include <gtest/gtest.h>

#include <expected>
#include <span>
#include <vector>

namespace UnrealVoxelSim::Voxel::Solid::Rendering
{

namespace
{

class FakeField final : public UnrealVoxelSim::Voxel::Api::IBounds,
                        public UnrealVoxelSim::Voxel::Solid::Api::IRegionReader
{
  public:
    explicit FakeField(const UnrealVoxelSim::Voxel::Api::Region bounds) : m_Bounds(bounds), m_Cells(*bounds.CellCount())
    {
    }

    [[nodiscard]] UnrealVoxelSim::Voxel::Api::Region GetBounds() const noexcept override
    {
        return m_Bounds;
    }

    void Set(const UnrealVoxelSim::Voxel::Api::Position position,
             const UnrealVoxelSim::Voxel::Solid::Api::MaterialId material)
    {
        m_Cells[Index(position)] = UnrealVoxelSim::Voxel::Solid::Api::Cell{material};
    }

    [[nodiscard]] std::expected<void, UnrealVoxelSim::Voxel::Api::ReadError> ReadRegion(
        const UnrealVoxelSim::Voxel::Api::Region region,
        const std::span<UnrealVoxelSim::Voxel::Solid::Api::Cell> output) const override
    {
        if (!m_Bounds.Contains(region))
        {
            return std::unexpected{UnrealVoxelSim::Voxel::Api::ReadError::OutOfBounds};
        }
        if (output.size() != *region.CellCount())
        {
            return std::unexpected{UnrealVoxelSim::Voxel::Api::ReadError::OutputSizeMismatch};
        }

        std::size_t index{};
        for (auto z = region.Min.Z; z < region.Max.Z; ++z)
        {
            for (auto y = region.Min.Y; y < region.Max.Y; ++y)
            {
                for (auto x = region.Min.X; x < region.Max.X; ++x)
                {
                    output[index++] = m_Cells[Index({x, y, z})];
                }
            }
        }
        return {};
    }

  private:
    [[nodiscard]] std::size_t Index(const UnrealVoxelSim::Voxel::Api::Position position) const noexcept
    {
        const auto width = static_cast<std::size_t>(m_Bounds.Max.X - m_Bounds.Min.X);
        const auto height = static_cast<std::size_t>(m_Bounds.Max.Y - m_Bounds.Min.Y);
        const auto x = static_cast<std::size_t>(position.X - m_Bounds.Min.X);
        const auto y = static_cast<std::size_t>(position.Y - m_Bounds.Min.Y);
        const auto z = static_cast<std::size_t>(position.Z - m_Bounds.Min.Z);
        return (z * height + y) * width + x;
    }

    UnrealVoxelSim::Voxel::Api::Region m_Bounds;
    std::vector<UnrealVoxelSim::Voxel::Solid::Api::Cell> m_Cells;
};

}

TEST(SamplerTest, CapturesOneCellHaloAndMapsMaterialsToSurfaces)
{
    FakeField field{{{0, 0, 0}, {4, 4, 4}}};
    field.Set({1, 1, 1}, UnrealVoxelSim::Voxel::Solid::Api::StandardMaterials::Grass);
    const Sampler sampler{field, field};

    const auto result = sampler.Capture({{1, 1, 1}, {2, 2, 2}});

    ASSERT_TRUE(result);
    EXPECT_EQ(result->Samples, (UnrealVoxelSim::Voxel::Api::Region{{0, 0, 0}, {3, 3, 3}}));
    ASSERT_EQ(result->Cells.size(), 27U);
    EXPECT_EQ(result->Cells[13].Value(), UnrealVoxelSim::Voxel::Solid::Api::StandardMaterials::Grass.Value());
}

TEST(SamplerTest, ClipsHaloAtLogicalWorldBounds)
{
    FakeField field{{{-2, -2, -2}, {2, 2, 2}}};
    const Sampler sampler{field, field};

    const auto result = sampler.Capture({{-2, -2, -2}, {-1, -1, -1}});

    ASSERT_TRUE(result);
    EXPECT_EQ(result->Samples, (UnrealVoxelSim::Voxel::Api::Region{{-2, -2, -2}, {0, 0, 0}}));
}

TEST(SamplerTest, RejectsTargetsOutsideLogicalBounds)
{
    FakeField field{{{0, 0, 0}, {4, 4, 4}}};
    const Sampler sampler{field, field};

    const auto result = sampler.Capture({{-1, 0, 0}, {1, 1, 1}});

    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), CaptureError::OutOfBounds);
}

}
