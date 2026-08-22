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
    explicit FakeField(const UnrealVoxelSim::Voxel::Api::Region bounds) : Bounds_(bounds), Cells_(*bounds.CellCount())
    {
    }

    [[nodiscard]] UnrealVoxelSim::Voxel::Api::Region Bounds() const noexcept override
    {
        return Bounds_;
    }

    void Set(const UnrealVoxelSim::Voxel::Api::Position position,
             const UnrealVoxelSim::Voxel::Solid::Api::MaterialId material)
    {
        Cells_[Index(position)] = UnrealVoxelSim::Voxel::Solid::Api::Cell{material};
    }

    [[nodiscard]] std::expected<void, UnrealVoxelSim::Voxel::Api::ReadError> ReadRegion(
        const UnrealVoxelSim::Voxel::Api::Region region,
        const std::span<UnrealVoxelSim::Voxel::Solid::Api::Cell> output) const override
    {
        if (!Bounds_.Contains(region))
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
                    output[index++] = Cells_[Index({x, y, z})];
                }
            }
        }
        return {};
    }

  private:
    [[nodiscard]] std::size_t Index(const UnrealVoxelSim::Voxel::Api::Position position) const noexcept
    {
        const auto width = static_cast<std::size_t>(Bounds_.Max.X - Bounds_.Min.X);
        const auto height = static_cast<std::size_t>(Bounds_.Max.Y - Bounds_.Min.Y);
        const auto x = static_cast<std::size_t>(position.X - Bounds_.Min.X);
        const auto y = static_cast<std::size_t>(position.Y - Bounds_.Min.Y);
        const auto z = static_cast<std::size_t>(position.Z - Bounds_.Min.Z);
        return (z * height + y) * width + x;
    }

    UnrealVoxelSim::Voxel::Api::Region Bounds_;
    std::vector<UnrealVoxelSim::Voxel::Solid::Api::Cell> Cells_;
};

} // namespace

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

} // namespace UnrealVoxelSim::Voxel::Solid::Rendering
