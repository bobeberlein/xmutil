# Test fixtures

Model files the test suite and CLI smoke scripts load from disk. Each file is an
unmodified byte-for-byte copy of its upstream original; xmutil does not edit
them. Paths are resolved relative to the repo root (via `XMUTIL_SRC_ROOT` in the
C++ tests, and directly in the shell scripts).

## Subtrees

- `sdeverywhere/` -- Vensim `.mdl` and XMILE `.xmile` single-feature models from
  the SDEverywhere project's test corpus (`test/sdeverywhere/` upstream). The
  `.xmile` models are xmutil-produced conversions checked into the simlin test
  tree; they exercise the XMILE reader. License: see `sdeverywhere/LICENSE`.
- `test-models/` -- Vensim `.mdl` and XMILE `.xmile`/`.stmx` models from the
  SDXorg `test-models` corpus (`test/test-models/` upstream), covering builtins,
  subscripts, macros, lookups, delays, and sample models. The `.stmx` files under
  `samples/` are the native Stella exports of the sample models (`teacup`, `SIR`,
  `arrays`, `display`, `bpowers-hares_and_lynxes_modules`), including the legacy
  1.0-format `*_legacy.stmx` variants. License: see `test-models/LICENSE`.
- `simlin/` -- fixtures from the simlin project: the `default_projects` XMILE
  models (`fishbanks.xmile`, `logistic-growth.xmile`, `reliability.xmile`) and
  the pysimlin `teacup.mdl` test fixture. License: see `simlin/LICENSE`.
- `simlin-test/` -- standalone models from the simlin test tree, mirroring its
  per-feature directories (`test/<dir>/` upstream): the wave-1 `.xmile` models
  (`delays/model.xmile`, `lookup_minimal/lookup_minimal.xmile`) plus the wave-2
  Stella `.stmx` models exercising the `.stmx` dialect (`alias1`,
  `arms_race_3party`, `builtin_init`, `circular-dep-1`, `decoupled_stocks`,
  `logistic_growth_ltm`, and the deferred/rejected `ai-information`, `arrays1`,
  `land_model`, `previous`, `step_into_smth1`, `subscript_index_name_values`).
  License: see `simlin-test/LICENSE`.

The large `C-LEARN v77 for Vensim` model (a 1.4 MB real-world climate policy
export with macros, arrays, lookups and a full sketch) was used throughout
development and testing of the writers, and `CorpusRoundTrip_large_model_stress`
in the mdl suite still reads it -- from upstream's own `test_models/` directory
at the repository root, which already ships it. It is deliberately NOT vendored
here: a second copy would add 53k lines to the history for a file every
checkout already has.

The XMILE corpus is exercised by `test/xmile/XmileCorpusTest.cpp`: every copied
`.xmile`/`.stmx` model is either on the round-trip allow-list (asserted to
convert cleanly XMILE->XMILE and XMILE->MDL), in the documented deferred table
(a handful of sketch-geometry / Stella-dialect / external-data / builtin-semantic
gaps, each with a specific reason), or in the rejection table (module-submodel
documents the reader rejects by design). Arrayed models round-trip as of wave 2.

Suite-specific golden outputs (expected emission results) live next to their
tests under `test/mdl/fixtures/` and `test/xmile/fixtures/`, not here.
