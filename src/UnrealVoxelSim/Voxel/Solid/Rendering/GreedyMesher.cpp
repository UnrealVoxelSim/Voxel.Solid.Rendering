#include "UnrealVoxelSim/Voxel/Solid/Rendering/GreedyMesher.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace UnrealVoxelSim::Voxel::Solid::Rendering
{
	namespace
	{
		using UnrealVoxelSim::Voxel::Api::Position;
		using UnrealVoxelSim::Voxel::Rendering::Api::Mesh;
		using UnrealVoxelSim::Voxel::Rendering::Api::Snapshot;
		using UnrealVoxelSim::Voxel::Rendering::Api::SurfaceId;
		using UnrealVoxelSim::Voxel::Rendering::Api::Vertex;

		struct MaskCell final
		{
			SurfaceId Surface{};
			std::int8_t Direction{};

			auto operator<=>(const MaskCell&) const = default;
		};

		struct TextureBasis final
		{
			std::size_t UAxis{};
			std::int8_t UDirection{};
			std::size_t VAxis{};
			std::int8_t VDirection{};
		};

		[[nodiscard]] TextureBasis FaceTextureBasis(const std::size_t axis,
		                                           const std::int8_t direction) noexcept
		{
			if (axis == 0)
			{
				return {1, direction, 2, 1};
			}
			if (axis == 1)
			{
				return {0, static_cast<std::int8_t>(-direction), 2, 1};
			}
			return {0, 1, 1, direction};
		}

		[[nodiscard]] std::optional<std::array<std::int32_t, 3>> Dimensions(
			const UnrealVoxelSim::Voxel::Api::Region region) noexcept
		{
			if (!region.IsValid())
			{
				return std::nullopt;
			}

			const std::array<std::int64_t, 3> dimensions{
				static_cast<std::int64_t>(region.Max.X) - region.Min.X,
				static_cast<std::int64_t>(region.Max.Y) - region.Min.Y,
				static_cast<std::int64_t>(region.Max.Z) - region.Min.Z,
			};
			constexpr auto maximum = std::numeric_limits<std::int32_t>::max();
			if (dimensions[0] > maximum || dimensions[1] > maximum || dimensions[2] > maximum)
			{
				return std::nullopt;
			}
			return std::array<std::int32_t, 3>{
				static_cast<std::int32_t>(dimensions[0]),
				static_cast<std::int32_t>(dimensions[1]),
				static_cast<std::int32_t>(dimensions[2])
			};
		}

		[[nodiscard]] SurfaceId Sample(const Snapshot& snapshot, const std::array<std::int32_t, 3>& local) noexcept
		{
			const std::array<std::int64_t, 3> global{
				static_cast<std::int64_t>(snapshot.Target.Min.X) + local[0],
				static_cast<std::int64_t>(snapshot.Target.Min.Y) + local[1],
				static_cast<std::int64_t>(snapshot.Target.Min.Z) + local[2],
			};
			constexpr auto minimum = std::numeric_limits<std::int32_t>::min();
			constexpr auto maximum = std::numeric_limits<std::int32_t>::max();
			if (global[0] < minimum || global[0] > maximum || global[1] < minimum || global[1] > maximum ||
				global[2] < minimum || global[2] > maximum)
			{
				return {};
			}

			const Position position{
				static_cast<std::int32_t>(global[0]),
				static_cast<std::int32_t>(global[1]),
				static_cast<std::int32_t>(global[2])
			};
			if (!snapshot.Samples.Contains(position))
			{
				return {};
			}

			const auto dimensions = *Dimensions(snapshot.Samples);
			const auto x = static_cast<std::size_t>(position.X - snapshot.Samples.Min.X);
			const auto y = static_cast<std::size_t>(position.Y - snapshot.Samples.Min.Y);
			const auto z = static_cast<std::size_t>(position.Z - snapshot.Samples.Min.Z);
			const auto index = (z * static_cast<std::size_t>(dimensions[1]) + y) * static_cast<std::size_t>(dimensions[
				0]) + x;
			return snapshot.Cells[index];
		}

		[[nodiscard]] Vertex MakeVertex(const std::array<std::int32_t, 3>& position,
		                                const std::size_t axis,
		                                const std::int8_t direction,
		                                const SurfaceId surface,
		                                const float u,
		                                const float v) noexcept
		{
			std::array<std::int8_t, 3> normal{};
			normal[axis] = direction;
			return Vertex{position[0], position[1], position[2], normal[0], normal[1], normal[2], surface, u, v};
		}

		[[nodiscard]] bool AppendQuad(Mesh& mesh,
		                              const std::array<std::int32_t, 3>& origin,
		                              const std::size_t axis,
		                              const std::size_t u,
		                              const std::size_t v,
		                              const std::int32_t width,
		                              const std::int32_t height,
		                              const MaskCell cell)
		{
			constexpr auto maximumIndex = std::numeric_limits<std::uint32_t>::max();
			if (mesh.Vertices.size() > static_cast<std::size_t>(maximumIndex) - 4)
			{
				return false;
			}

			auto uEnd = origin;
			auto vEnd = origin;
			auto uvEnd = origin;
			uEnd[u] += width;
			vEnd[v] += height;
			uvEnd[u] += width;
			uvEnd[v] += height;

			const auto base = static_cast<std::uint32_t>(mesh.Vertices.size());
			const auto basis = FaceTextureBasis(axis, cell.Direction);
			const auto minimum = [](const std::int32_t first,
			                       const std::int32_t second,
			                       const std::int32_t third,
			                       const std::int32_t fourth) noexcept
			{
				return std::min(std::min(first, second), std::min(third, fourth));
			};
			const auto maximum = [](const std::int32_t first,
			                       const std::int32_t second,
			                       const std::int32_t third,
			                       const std::int32_t fourth) noexcept
			{
				return std::max(std::max(first, second), std::max(third, fourth));
			};
			const auto uMinimum = minimum(origin[basis.UAxis], uEnd[basis.UAxis], vEnd[basis.UAxis], uvEnd[basis.UAxis]);
			const auto uMaximum = maximum(origin[basis.UAxis], uEnd[basis.UAxis], vEnd[basis.UAxis], uvEnd[basis.UAxis]);
			const auto vMinimum = minimum(origin[basis.VAxis], uEnd[basis.VAxis], vEnd[basis.VAxis], uvEnd[basis.VAxis]);
			const auto vMaximum = maximum(origin[basis.VAxis], uEnd[basis.VAxis], vEnd[basis.VAxis], uvEnd[basis.VAxis]);
			const auto textureCoordinates = [&](const std::array<std::int32_t, 3>& position) noexcept
			{
				const auto u = basis.UDirection > 0 ? position[basis.UAxis] - uMinimum
				                                  : uMaximum - position[basis.UAxis];
				const auto vCoordinate = basis.VDirection > 0 ? position[basis.VAxis] - vMinimum
				                                            : vMaximum - position[basis.VAxis];
				return std::pair{static_cast<float>(u), static_cast<float>(vCoordinate)};
			};
			const auto originTexture = textureCoordinates(origin);
			const auto uEndTexture = textureCoordinates(uEnd);
			const auto vEndTexture = textureCoordinates(vEnd);
			const auto uvEndTexture = textureCoordinates(uvEnd);
			if (cell.Direction > 0)
			{
				mesh.Vertices.push_back(
					MakeVertex(origin, axis, cell.Direction, cell.Surface, originTexture.first, originTexture.second));
				mesh.Vertices.push_back(
					MakeVertex(uEnd, axis, cell.Direction, cell.Surface, uEndTexture.first, uEndTexture.second));
				mesh.Vertices.push_back(
					MakeVertex(uvEnd, axis, cell.Direction, cell.Surface, uvEndTexture.first, uvEndTexture.second));
				mesh.Vertices.push_back(
					MakeVertex(vEnd, axis, cell.Direction, cell.Surface, vEndTexture.first, vEndTexture.second));
			}
			else
			{
				mesh.Vertices.push_back(
					MakeVertex(origin, axis, cell.Direction, cell.Surface, originTexture.first, originTexture.second));
				mesh.Vertices.push_back(
					MakeVertex(vEnd, axis, cell.Direction, cell.Surface, vEndTexture.first, vEndTexture.second));
				mesh.Vertices.push_back(
					MakeVertex(uvEnd, axis, cell.Direction, cell.Surface, uvEndTexture.first, uvEndTexture.second));
				mesh.Vertices.push_back(
					MakeVertex(uEnd, axis, cell.Direction, cell.Surface, uEndTexture.first, uEndTexture.second));
			}

			mesh.Indices.insert(mesh.Indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
			return true;
		}
	}

	std::expected<UnrealVoxelSim::Voxel::Rendering::Api::Mesh, BuildError> GreedyMesher::Build(
		const UnrealVoxelSim::Voxel::Rendering::Api::Snapshot& snapshot) const
	{
		const auto targetDimensions = Dimensions(snapshot.Target);
		const auto sampleDimensions = Dimensions(snapshot.Samples);
		const auto sampleCount = snapshot.Samples.CellCount();
		if (!targetDimensions || !sampleDimensions || snapshot.Target.IsEmpty() ||
			!snapshot.Samples.Contains(snapshot.Target) || !sampleCount || *sampleCount != snapshot.Cells.size())
		{
			return std::unexpected{BuildError::InvalidSnapshot};
		}

		Mesh mesh;
		mesh.Bounds = snapshot.Target;
		const auto dimensions = *targetDimensions;

		for (std::size_t axis = 0; axis < 3; ++axis)
		{
			const auto u = (axis + 1) % 3;
			const auto v = (axis + 2) % 3;
			const auto maskWidth = static_cast<std::size_t>(dimensions[u]);
			const auto maskHeight = static_cast<std::size_t>(dimensions[v]);
			if (maskWidth != 0 && maskHeight > std::numeric_limits<std::size_t>::max() / maskWidth)
			{
				return std::unexpected{BuildError::SizeOverflow};
			}
			std::vector<MaskCell> mask(maskWidth * maskHeight);

			for (std::int32_t plane = 0; plane <= dimensions[axis]; ++plane)
			{
				for (std::int32_t j = 0; j < dimensions[v]; ++j)
				{
					for (std::int32_t i = 0; i < dimensions[u]; ++i)
					{
						std::array<std::int32_t, 3> before{};
						std::array<std::int32_t, 3> after{};
						before[axis] = plane - 1;
						after[axis] = plane;
						before[u] = after[u] = i;
						before[v] = after[v] = j;

						const auto first = Sample(snapshot, before);
						const auto second = Sample(snapshot, after);
						auto& entry = mask[static_cast<std::size_t>(j) * maskWidth + static_cast<std::size_t>(i)];
						entry = {};
						if (first.IsValid() == second.IsValid())
						{
							continue;
						}
						if (first.IsValid() && plane > 0)
						{
							entry = {first, 1};
						}
						else if (second.IsValid() && plane < dimensions[axis])
						{
							entry = {second, -1};
						}
					}
				}

				for (std::int32_t j = 0; j < dimensions[v]; ++j)
				{
					for (std::int32_t i = 0; i < dimensions[u];)
					{
						const auto index = static_cast<std::size_t>(j) * maskWidth + static_cast<std::size_t>(i);
						const auto cell = mask[index];
						if (!cell.Surface.IsValid())
						{
							++i;
							continue;
						}

						std::int32_t width = 1;
						while (i + width < dimensions[u] && mask[index + static_cast<std::size_t>(width)] == cell)
						{
							++width;
						}

						std::int32_t height = 1;
						for (; j + height < dimensions[v]; ++height)
						{
							bool matches = true;
							const auto row = static_cast<std::size_t>(j + height) * maskWidth + static_cast<std::size_t>
								(i);
							for (std::int32_t offset = 0; offset < width; ++offset)
							{
								if (mask[row + static_cast<std::size_t>(offset)] != cell)
								{
									matches = false;
									break;
								}
							}
							if (!matches)
							{
								break;
							}
						}

						std::array<std::int32_t, 3> origin{};
						origin[axis] = plane;
						origin[u] = i;
						origin[v] = j;
						if (!AppendQuad(mesh, origin, axis, u, v, width, height, cell))
						{
							return std::unexpected{BuildError::SizeOverflow};
						}

						for (std::int32_t y = 0; y < height; ++y)
						{
							const auto row = static_cast<std::size_t>(j + y) * maskWidth + static_cast<std::size_t>(i);
							for (std::int32_t x = 0; x < width; ++x)
							{
								mask[row + static_cast<std::size_t>(x)] = {};
							}
						}
						i += width;
					}
				}
			}
		}

		return mesh;
	}
}
