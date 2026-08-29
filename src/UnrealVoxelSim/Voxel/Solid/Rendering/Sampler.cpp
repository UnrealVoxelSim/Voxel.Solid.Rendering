#include "UnrealVoxelSim/Voxel/Solid/Rendering/Sampler.h"

#include "UnrealVoxelSim/Voxel/Solid/Api/Cell.h"

#include <vector>

namespace UnrealVoxelSim::Voxel::Solid::Rendering
{
	Sampler::Sampler(const UnrealVoxelSim::Voxel::Api::IBounds& bounds,
	                 const UnrealVoxelSim::Voxel::Solid::Api::IRegionReader& reader) noexcept
		: m_Bounds(bounds), m_Reader(reader)
	{
	}

	std::expected<UnrealVoxelSim::Voxel::Rendering::Api::Snapshot, CaptureError> Sampler::Capture(
		const UnrealVoxelSim::Voxel::Api::Region target) const
	{
		if (!target.IsValid() || target.IsEmpty())
		{
			return std::unexpected{CaptureError::InvalidRegion};
		}

		const auto bounds = m_Bounds.Bounds();
		if (!bounds.Contains(target))
		{
			return std::unexpected{CaptureError::OutOfBounds};
		}

		UnrealVoxelSim::Voxel::Api::Region samples{
			{
				target.Min.X > bounds.Min.X ? target.Min.X - 1 : target.Min.X,
				target.Min.Y > bounds.Min.Y ? target.Min.Y - 1 : target.Min.Y,
				target.Min.Z > bounds.Min.Z ? target.Min.Z - 1 : target.Min.Z
			},
			{
				target.Max.X < bounds.Max.X ? target.Max.X + 1 : target.Max.X,
				target.Max.Y < bounds.Max.Y ? target.Max.Y + 1 : target.Max.Y,
				target.Max.Z < bounds.Max.Z ? target.Max.Z + 1 : target.Max.Z
			},
		};

		const auto sampleCount = samples.CellCount();
		if (!sampleCount)
		{
			return std::unexpected{CaptureError::SizeOverflow};
		}

		std::vector<UnrealVoxelSim::Voxel::Solid::Api::Cell> cells(*sampleCount);
		if (!m_Reader.ReadRegion(samples, cells))
		{
			return std::unexpected{CaptureError::ReadFailed};
		}

		UnrealVoxelSim::Voxel::Rendering::Api::Snapshot snapshot;
		snapshot.Target = target;
		snapshot.Samples = samples;
		snapshot.Cells.reserve(cells.size());
		for (const auto cell : cells)
		{
			snapshot.Cells.emplace_back(cell.IsEmpty() ? 0U : cell.Material().Value());
		}
		return snapshot;
	}
}
