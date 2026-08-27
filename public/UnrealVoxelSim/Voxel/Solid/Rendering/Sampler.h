#pragma once

#include "UnrealVoxelSim/Voxel/Api/IBounds.h"
#include "UnrealVoxelSim/Voxel/Rendering/Api/Snapshot.h"
#include "UnrealVoxelSim/Voxel/Solid/Api/IRegionReader.h"
#include "UnrealVoxelSim/Voxel/Solid/Rendering/CaptureError.h"

#include <expected>

namespace UnrealVoxelSim::Voxel::Solid::Rendering
{
	class Sampler final
	{
	public:
		Sampler(const UnrealVoxelSim::Voxel::Api::IBounds& bounds,
		        const UnrealVoxelSim::Voxel::Solid::Api::IRegionReader& reader) noexcept;

		[[nodiscard]] std::expected<UnrealVoxelSim::Voxel::Rendering::Api::Snapshot, CaptureError> Capture(
			UnrealVoxelSim::Voxel::Api::Region target) const;

	private:
		const UnrealVoxelSim::Voxel::Api::IBounds& m_Bounds;
		const UnrealVoxelSim::Voxel::Solid::Api::IRegionReader& m_Reader;
	};
}
