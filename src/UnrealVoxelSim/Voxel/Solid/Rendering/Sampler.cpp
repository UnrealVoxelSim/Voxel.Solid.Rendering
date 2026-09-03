#include "UnrealVoxelSim/Voxel/Solid/Rendering/Sampler.h"

#include "UnrealVoxelSim/Voxel/Solid/Api/Cell.h"

#include <algorithm>
#include <ranges>
#include <vector>

namespace UnrealVoxelSim::Voxel::Solid::Rendering
{
	Sampler::Sampler(const UnrealVoxelSim::Voxel::Api::IBounds& bounds,
	                 const UnrealVoxelSim::Voxel::Solid::Api::IRegionReader& reader,
	                 const std::span<const UnrealVoxelSim::Voxel::Rendering::Api::MaterialSurfaceBinding> bindings) noexcept
		: m_Bounds(bounds), m_Reader(reader), m_Bindings(bindings)
	{
	}

	namespace
	{
		using UnrealVoxelSim::Voxel::Rendering::Api::SurfaceId;
		using UnrealVoxelSim::Voxel::Rendering::Api::MaterialSurfaceBinding;
		using UnrealVoxelSim::Voxel::Solid::Api::Cell;

		[[nodiscard]] SurfaceId ResolveSurface(const Cell& cell,
		                                      const std::span<const MaterialSurfaceBinding> bindings) noexcept
		{
			const auto iterator = std::ranges::find(bindings, cell.Material(), &MaterialSurfaceBinding::Material);
			return iterator == bindings.end() ? SurfaceId{} : iterator->Surface;
		}
	}

	std::expected<UnrealVoxelSim::Voxel::Rendering::Api::Snapshot, CaptureError> Sampler::Capture(
		const UnrealVoxelSim::Voxel::Api::Region target) const
	{
		if (!target.IsValid() || target.IsEmpty())
		{
			return std::unexpected{CaptureError::InvalidRegion};
		}

		const auto bounds = m_Bounds.GetBounds();
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
			if (cell.IsEmpty())
			{
				snapshot.Cells.emplace_back();
				continue;
			}

			const auto surface = ResolveSurface(cell, m_Bindings);
			if (!surface.IsValid())
			{
				return std::unexpected{CaptureError::UnknownMaterial};
			}
			snapshot.Cells.emplace_back(surface);
		}
		return snapshot;
	}
}
