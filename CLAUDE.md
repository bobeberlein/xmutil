# xmutil

Last verified: 2026-07-06

C++ tool that converts system-dynamics models between formats: Vensim `.mdl`,
Dynamo `.dyn`, and XMILE (`.xmile` / `.stmx`) in; XMILE or Vensim `.mdl` out.
CLI binary `XMUtil`, plus a WASM build and (optionally) a Qt UI.

## Tech Stack
- C++, linking the system tinyxml2 and ICU libraries on Linux (matching
  upstream): local builds need the dev packages (`tinyxml2-devel` +
  `libicu-devel` on Fedora, `tinyxml2-dev` + `icu-dev` on Alpine), or run the
  build and tests in a container that has them.
- Build: gyp + ninja only. There is no CMake in the main repo.
- Parsers: bison/flex grammars under `src/Vensim/` and `src/Dynamo/`
  (generated `*.tab.*` are committed; the `.y` sources are the source of truth
  for operator precedence).

## Commands
- Configure (regenerate the ninja build): `./configure.sh`
- Build the CLI: `ninja -C out/Debug XMUtil`
- Build the tests: `ninja -C out/Debug xmutil_test`
- Run the tests: `out/Debug/xmutil_test` (exit code 0 == all passed)
- CLI smoke round-trip (mdl): `bash test/cli_roundtrip.sh` (run from repo root)
- CLI smoke round-trip (xmile): `bash test/cli_xmile_roundtrip.sh`
- Format: `./format.sh` (clang-format in place; run before committing).
  Generated `*.tab.{cpp,hpp}` are excluded via `find -not -name` so the
  filter works under BSD `find` on macOS too.
- Binary is at `out/Debug/XMUtil`; `out/Release/` mirrors it.

### Windows / Visual Studio
- Generate the solution (from Git Bash, needs Python 2.7 on PATH):
  `./configure.sh --use-msvs` -> `XMUtil.sln` with the CLI and test projects
  (`*.sln`/`*.vcxproj` are gitignored; the wasm target is excluded on Windows
  because its action shells out to emcc).
- `build/environment.sh` locates the newest VS with the C++ toolset via
  `vswhere` and hands gyp `GYP_MSVS_OVERRIDE_PATH`, since VS 2017+ no longer
  writes the registry keys gyp probes. Override `GYP_MSVS_VERSION` to pin a
  different one.
- Build: `MSBuild.exe XMUtil.sln -p:Configuration=Debug -p:Platform=x64`, or
  open the solution. Output lands in `Debug/` (`Release/` mirrors it) next to
  the ICU DLLs a post-build copy step puts there.
- tinyxml2 has no prebuilt Windows lib, so `third_party/include/tinyxml2.cpp`
  is compiled into each target (`platform_sources` in `XMUtil.gyp`).
- Fixtures checked out with CRLF make `MdlXmileByteIdentity_teacup` fail: the
  writer copies input line endings into `<doc>` text, so the golden no longer
  matches byte for byte. With LF fixtures the output is byte-identical.

## CLI
- `XMUtil <model>` converts to XMILE (writes `<base>.xmile`). An XMILE->XMILE
  run on a `.xmile` input writes `<base>.regen.xmile` to avoid clobbering its
  own input (a `.stmx` input yields a `.xmile` output, so no rename is needed).
- `XMUtil --to-mdl <model>` converts to Vensim `.mdl`. A Vensim->Vensim run
  writes `<base>.regen.mdl` to avoid clobbering its own input.
- `--stdio` reads stdin / writes stdout; `--sectors`, `--names`/`--no-names`
  as before. Input format is chosen by file extension (case-insensitive):
  `.dyn` -> Dynamo, `.xmile` or `.stmx` -> XMILE, else Vensim.

## Project Structure
- `src/` - engine + tools. `Main.cpp` is the CLI entry; `XMUtil.{h,cpp}` holds
  the `extern "C"` conversion entry points: `convert_mdl_to_xmile`,
  `convert_to_mdl` (Vensim/Dynamo in), and `convert_xmile_to_xmile`,
  `convert_xmile_to_mdl` (XMILE in). All four return a strdup'd heap buffer
  the caller must `free`, or NULL on failure.
- `src/Vensim/`, `src/Dynamo/` - input parsers (the readers).
- `src/Symbol/`, `src/Function/` - the in-memory model: `Expression` trees,
  `Variable`, `Symbol`, namespaces.
- `src/Xmile/` - XMILE reader AND writer (see its CLAUDE.md).
- `src/Mdl/` - the Vensim `.mdl` writer (see its CLAUDE.md).
- `src/Model.{h,cpp}` - the `Model` container. `Model::ParseXMILE` is the
  convenience entry that constructs an `XmileReader` and drives one parse;
  `Model::RunPostParsePipeline` is the shared post-parse sequence
  (`ConfirmAllAllocations` -> `MarkVariableTypes` (main + every macro) ->
  `AdjustGroupNames` -> `CheckGhostOwners` -> `ResolveWildcardSubscripts`) that
  every conversion entry point invokes after parsing and before serialization.
  `ResolveWildcardSubscripts` binds the XMILE reader's bare `*` wildcard
  subscripts to their referenced variable's dimension family; it runs last
  because it needs the element/family ownership `MarkVariableTypes` establishes,
  and is a no-op for Vensim/Dynamo input (no wildcard entries).
- `test/` - the test suite (see Testing below).
- `third_party/` - upstream's prebuilt-library scaffolding for the Windows and
  macOS builds (nothing is vendored for Linux; tinyxml2 and ICU come from the
  system there). A gitignored `third_party/simlin/` reference checkout of the
  simlin project may exist locally; do NOT edit it (it has its own
  CLAUDE.md/AGENTS.md files that apply only to that subtree).

## Testing
The project had zero tests before the mdl-writer work. Tests now live under
`test/` and build into the `xmutil_test` gyp target (defined in `XMUtil.gyp`),
which links the same engine sources as `XMUtil` but swaps `Main.cpp` for a
minimal self-registering harness (`test/TestHarness.{h,cpp}`: `TEST(name)`,
`CHECK`, `CHECK_EQ_STR`). New test `.cpp` files must be added to the
`xmutil_test` `sources` list in `XMUtil.gyp`, then re-run `./configure.sh`.
The suite is organized by direction: `test/mdl/` covers the .mdl writer and
shares `ModelComparator` / `RoundTrip` helpers; `test/xmile/` covers the
XMILE reader (round-trips XMILE -> Model -> XMILE/MDL, with the helpers in
`test/xmile/RoundTrip.{h,cpp}`). Fixtures the tests load from disk resolve
via the `XMUTIL_SRC_ROOT` define (set from `<(cwd)` by `./configure.sh`).

Real model fixtures live under `test/fixtures/` (byte-identical vendored
copies; provenance in `test/fixtures/README.md`). Two corpus tests drive them:
`test/xmile/XmileCorpusTest.cpp` round-trips an allow-list of `.xmile`/`.stmx`
models in both directions and has a coverage walk that fails if any fixture on
disk is in none of its allow/deferred/rejected tables; `test/mdl/
CorpusRoundTripTest.cpp` round-trips an allow-list of Vensim `.mdl` models and
stress-tests the large `C-LEARN v77` export (read from upstream's own
`test_models/` directory, deliberately not vendored under `test/fixtures/`).
Adding a `.xmile`/`.stmx` fixture therefore requires triaging it into one of
the XMILE corpus tables.

## Conventions
- Generated parser tables (`src/*/*.tab.*`) are committed; regenerate via the
  grammar, do not hand-edit the tables.
- No emoji in commit messages, code comments, or docs. Comments explain WHY /
  the non-obvious, never restate the next line of code.

## Boundaries
- Safe to edit: `src/`, `test/`.
- Do not edit `third_party/` (upstream build scaffolding, plus any local
  simlin reference checkout).
