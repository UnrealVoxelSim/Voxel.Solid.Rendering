#pragma once

#include "UnrealVoxelSim/Voxel/Rendering/Api/Mesh.h"
#include "UnrealVoxelSim/Voxel/Rendering/Api/Snapshot.h"
#include "UnrealVoxelSim/Voxel/Solid/Rendering/BuildError.h"

#include <expected>

namespace UnrealVoxelSim::Voxel::Solid::Rendering
{
	class GreedyMesher final
	{
	public:
		[[nodiscard]] std::expected<UnrealVoxelSim::Voxel::Rendering::Api::Mesh, BuildError> Build(
			const UnrealVoxelSim::Voxel::Rendering::Api::Snapshot& snapshot) const;
	};
}
