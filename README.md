# UnrealVoxelSim.Voxel.Solid.Rendering

Portable derivation of renderer-neutral surface meshes from solid voxel state.

`Sampler` performs one coarse-grained solid-region read for a target tile and its clipped one-cell halo. It maps empty
cells to surface zero and material identifiers to opaque rendering surface identifiers. Sampling is thread-affine with
the injected voxel capabilities.

`GreedyMesher` consumes only an owned `Voxel.Rendering.Api::Snapshot`. It removes internal faces and merges coplanar
faces that share orientation and surface identity. The mesher is stateless and snapshots may be moved to worker threads.
Greedy merges deliberately stop at caller-selected logical tile boundaries so rebuild and culling granularity remain
bounded and independent of storage chunks.

The output is reconstructible derived state. Qt and Unreal adapters remain responsible for residency, invalidation,
camera culling, GPU resources, shaders, and materials.

Windows presets disable shared-library generation, use vcpkg's `x64-windows-static` triplet, and select the static MSVC
runtime. Interface-only internal dependencies remain header-only; linkable internal and external dependencies are static.
