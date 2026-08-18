#!/bin/bash
#
# CLI end-to-end XMILE round-trip smoke test (Phase 8, Task 3).
#
# Drives the built XMUtil binary the way a user would when the input is XMILE:
# confirms .xmile/.STMX inputs dispatch to the XMILE reader, the XMILE->XMILE
# .regen.xmile clobber guard fires for an in-place rewrite, --to-mdl produces a
# .mdl that begins with the {UTF-8} marker and re-emits cleanly through stdio,
# and the case-insensitive .STMX dispatch produces a .xmile output (no clash
# with the input extension means no .regen rename is needed).
#
# Requires a built XMUtil binary. Defaults to out/Debug/XMUtil; pass an alternate
# path as $1 (e.g. bash test/cli_xmile_roundtrip.sh out/Release/XMUtil).
#
# Run from the repository root so the fixture path resolves.

set -euo pipefail

BIN=${1:-out/Debug/XMUtil}
SRC="test/fixtures/simlin/logistic-growth.xmile"

if [ ! -x "$BIN" ]; then
  echo "error: XMUtil binary not found or not executable at '$BIN'" >&2
  echo "       build it first (ninja -C out/Debug XMUtil) or pass its path as \$1" >&2
  exit 1
fi
if [ ! -f "$SRC" ]; then
  echo "error: fixture not found at '$SRC' (run from the repo root)" >&2
  exit 1
fi

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

cp "$SRC" "$TMP/m.xmile"

# XMILE -> XMILE on a .xmile input: the .regen.xmile clobber guard fires so the
# input is not overwritten by its own emission.
"$BIN" "$TMP/m.xmile"
test -s "$TMP/m.regen.xmile"
head -1 "$TMP/m.regen.xmile" | grep -q '<xmile'

# XMILE -> MDL: produces a .mdl that begins with the {UTF-8} marker.
"$BIN" --to-mdl "$TMP/m.xmile"
test -s "$TMP/m.mdl"
head -1 "$TMP/m.mdl" | grep -q '{UTF-8}'

# Re-emit the generated .mdl through stdio to confirm it re-parses cleanly and
# the writer produces a fresh {UTF-8} document from it.
"$BIN" --to-mdl --stdio < "$TMP/m.mdl" | head -1 | grep -q '{UTF-8}'

# --stdio has no filename to dispatch on, so the input format is content-sniffed.
# Piping the XMILE model through --stdio must reach the XMILE reader (not the
# lenient Vensim parser): the XMILE->XMILE output starts with the <xmile> marker
# and carries a known variable through the round trip.
"$BIN" --stdio < "$TMP/m.xmile" > "$TMP/stdio.xmile"
head -1 "$TMP/stdio.xmile" | grep -q '<xmile'
grep -qi 'population' "$TMP/stdio.xmile"

# The same piped XMILE with --to-mdl must sniff as XMILE and emit a {UTF-8} .mdl.
"$BIN" --to-mdl --stdio < "$TMP/m.xmile" | head -1 | grep -q '{UTF-8}'

# Case-insensitive .STMX dispatch: copy with an uppercase .STMX extension and
# re-run. The .stmx input yields a .xmile output (different extension, no
# .regen rename needed), exercising the extension-case branch in Main.cpp.
cp "$SRC" "$TMP/m2.STMX"
"$BIN" "$TMP/m2.STMX"
test -s "$TMP/m2.xmile"
head -1 "$TMP/m2.xmile" | grep -q '<xmile'

# Failure path: a model whose only equation references a bare `*` wildcard on a
# scalar cannot be rendered as valid MDL/XMILE, so the conversion must fail
# cleanly -- non-zero exit AND at least one diagnostic on stderr (never a silent
# exit 1). Regression guard for the diagnostics-to-stderr routing.
cat > "$TMP/bad.xmile" <<'EOF'
<?xml version="1.0" encoding="utf-8"?>
<xmile version="1.0" xmlns="http://docs.oasis-open.org/xmile/ns/XMILE/v1.0">
  <sim_specs method="euler"><start>0</start><stop>10</stop><dt>1</dt></sim_specs>
  <model><variables>
    <aux name="x"><eqn>1</eqn></aux>
    <aux name="y"><eqn>SUM(x[*])</eqn></aux>
  </variables></model>
</xmile>
EOF
set +e
"$BIN" --to-mdl "$TMP/bad.xmile" >"$TMP/bad.stdout" 2>"$TMP/bad.stderr"
bad_rc=$?
set -e
if [ "$bad_rc" -eq 0 ]; then
  echo "error: expected non-zero exit converting malformed model" >&2
  exit 1
fi
if [ ! -s "$TMP/bad.stderr" ]; then
  echo "error: malformed-model conversion failed with EMPTY stderr" >&2
  exit 1
fi

echo "XMILE CLI round-trip OK"
