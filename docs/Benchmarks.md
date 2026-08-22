# Greedy meshing benchmarks

The release benchmark measures one 32-cubed target tile sampled with a one-cell halo.

- `FlatTerrain` represents a large uniform half-filled tile and exercises maximum greedy face merging.
- `FragmentedTerrain` represents a deliberately irregular mixture of empty cells and three surfaces.

Run the checked-in release benchmark preset. Record compiler, CPU, scenario, mesh size, and timing before drawing
conclusions or changing the public query boundary.

## Initial Windows baseline

Measured 2026-08-21 with MSVC 19.44 and the static MSVC runtime on an AMD Ryzen 5 7600X. Values are the mean of three
Google Benchmark repetitions using a 0.2-second minimum run time.

| Scenario | Wall time | CPU time | Output |
| --- | ---: | ---: | ---: |
| Flat terrain | 2.214 ms | 2.199 ms | 4 vertices, 2 triangles |
| Fragmented terrain | 7.169 ms | 7.234 ms | 196,608 vertices, 98,304 triangles |

The flat case is a target cut from an occupied half-space, including occupied lateral/bottom halo samples, so only its
top surface belongs in the mesh. The fragmented case is intentionally close to the greedy mesher's unfavorable case.
