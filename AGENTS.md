# Repository Guidelines

## Project Structure & Module Organization
- Source is organized by package under `packages/<name>/` with typical subfolders like `src/`, `test/`, and `example/`.
- Top-level CMake entry is `CMakeLists.txt`; TriBITS lives under `cmake/`.
- Docs and demos: `doc/` and `demos/`; helper scripts in `sampleScripts/`.

## Build, Test, and Development Commands
- Configure (out-of-source):
  - `cmake -S . -B build -DTrilinos_ENABLE_ALL_PACKAGES=ON -DTrilinos_ENABLE_TESTS=ON`
  - With MPI: add `-DTPL_ENABLE_MPI=ON -DMPI_BASE_DIR=<mpi-install>`
- Build: `cmake --build build -j`
- Test: `ctest --test-dir build -j` (after enabling tests in configure)
- Install: `cmake --install build --prefix <install-prefix>`
See `INSTALL.rst` and `sampleScripts/` for more options (e.g., Ninja, shared libs).

## Coding Style & Naming Conventions
- Primary languages: C++ and CMake. Prefer consistent style within each package.
- Indentation/spaces: follow existing files in the target package; avoid tabs.
- Filenames: use clear, package-aligned names (e.g., `MueLu_Foo.hpp`, `Tpetra_Bar.cpp`).
- Formatting: if available, use package scripts like `packages/kokkos/scripts/apply-clang-format` or `packages/muelu/utils/run_clang_format.sh` before committing.

## Testing Guidelines
- Framework: CTest via TriBITS. Enable with `-DTrilinos_ENABLE_TESTS=ON`.
- Add unit tests under the package’s `test/` or `unit-test/` directories.
- Naming: keep tests descriptive (e.g., `CrsMatrix_ImportExport_UnitTest.cpp`).
- Run focused tests via CTest labels or by directory using `ctest -R <regex>`.

## Commit & Pull Request Guidelines
- Commits: sign off every commit (`git commit -s`) and reference related issues (e.g., `Fixes #1234`).
- Branching: branch from `develop`; include issue number in the branch name when possible.
- PRs: open against `trilinos/Trilinos:develop`. Provide a clear description, link issues, and include build/test results or scripts. Keep changes scoped to a package when feasible.

## Security & Configuration Tips
- Use out-of-source builds; avoid committing generated files or large binaries.
- When enabling many packages, ensure required TPLs are discoverable or explicitly disabled as indicated by CMake output.

