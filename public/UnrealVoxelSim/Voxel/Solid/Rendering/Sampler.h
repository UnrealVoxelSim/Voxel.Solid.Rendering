#pragma once

#include "UnrealVoxelSim/Voxel/Api/IBounds.h"
#include "UnrealVoxelSim/Voxel/Rendering/Api/MaterialSurfaceBinding.h"
#include "UnrealVoxelSim/Voxel/Rendering/Api/Snapshot.h"
#include "UnrealVoxelSim/Voxel/Solid/Api/IRegionReader.h"
#include "UnrealVoxelSim/Voxel/Solid/Rendering/CaptureError.h"

#include <expected>
#include <span>

namespace UnrealVoxelSim::Voxel::Solid::Rendering
{
	class Sampler final
	{
	public:
		// Bindings are borrowed read-only data and must outlive this sampler.
		Sampler(const UnrealVoxelSim::Voxel::Api::IBounds& bounds,
		        const UnrealVoxelSim::Voxel::Solid::Api::IRegionReader& reader,
		        std::span<const UnrealVoxelSim::Voxel::Rendering::Api::MaterialSurfaceBinding> bindings) noexcept;

		[[nodiscard]] std::expected<UnrealVoxelSim::Voxel::Rendering::Api::Snapshot, CaptureError> Capture(
			UnrealVoxelSim::Voxel::Api::Region target) const;

	private:
		const UnrealVoxelSim::Voxel::Api::IBounds& m_Bounds;
		const UnrealVoxelSim::Voxel::Solid::Api::IRegionReader& m_Reader;
		std::span<const UnrealVoxelSim::Voxel::Rendering::Api::MaterialSurfaceBinding> m_Bindings;
	};
}
