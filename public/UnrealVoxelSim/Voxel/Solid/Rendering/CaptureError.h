#pragma once

namespace UnrealVoxelSim::Voxel::Solid::Rendering
{
	enum class CaptureError
	{
		InvalidRegion,
		OutOfBounds,
		SizeOverflow,
		ReadFailed,
		UnknownMaterial,
	};
}
