#!/bin/bash
#
# CLI end-to-end round-trip check (Phase 7, Task 2; verifies mdl-writer.AC1.3).
#
# Drives the built XMUtil binary the way a user would: convert a real .mdl on
# disk with --to-mdl (file in -> file out), confirm the regenerated .mdl exists
# and begins with the {UTF-8} marker, then feed that output back through
# --to-mdl --stdio to confirm the emitted .mdl re-parses (re-emits). The deeper
# structural round-trip is covered by the in-process tests (Task 1); this is the
# binary-level smoke test, since the project has no other CLI test runner.
#
# Requires a built XMUtil binary. Defaults to out/Debug/XMUtil; pass an alternate
# path as $1 (e.g. bash test/cli_roundtrip.sh out/Release/XMUtil).
#
# Run from the repository root so the fixture path resolves.

set -euo pipefail

BIN=${1:-out/Debug/XMUtil}
SRC="test/fixtures/simlin/teacup.mdl"

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

cp "$SRC" "$TMP/teacup.mdl"

# --to-mdl on a .mdl input writes <base>.regen.mdl (the clobber guard avoids
# overwriting the input, which shares the .mdl extension).
"$BIN" --to-mdl "$TMP/teacup.mdl"

test -s "$TMP/teacup.regen.mdl"  # non-empty output file exists
head -1 "$TMP/teacup.regen.mdl" | grep -q '{UTF-8}'

# Re-emit the generated .mdl through stdio to confirm it re-parses cleanly and
# the writer produces a fresh {UTF-8} document from it.
"$BIN" --to-mdl --stdio < "$TMP/teacup.regen.mdl" | head -1 | grep -q '{UTF-8}'

# --stdio stream separation on a recoverable-but-malformed model: the Vensim
# reader emits diagnostics (syntax error / skipping / warning) while still
# producing output and exiting 0. Those diagnostics must go to stderr, never
# stdout -- stdout is the document channel, so a diagnostic there would hand a
# downstream consumer invalid XML with no error signal. Assert the stdout stream
# starts with the document marker and the diagnostics landed on stderr.
cat > "$TMP/bad.mdl" <<'EOF'
{UTF-8}
x = @#$ bad tokens here
	~
	~	|

y= 1
	~
	~	|

\\\---///
EOF
"$BIN" --to-mdl --stdio < "$TMP/bad.mdl" > "$TMP/bad.out" 2> "$TMP/bad.err"
head -1 "$TMP/bad.out" | grep -q '{UTF-8}'   # stdout is the document, not a diagnostic
test -s "$TMP/bad.err"                        # the reader diagnostics landed on stderr
grep -q 'syntax error' "$TMP/bad.err"
# stdout must NOT contain the reader's diagnostic text
if grep -q 'syntax error' "$TMP/bad.out"; then
  echo "error: reader diagnostic leaked into --stdio stdout" >&2
  exit 1
fi

# A path that cannot be opened is a failure, not an empty conversion. This
# script itself runs under `set -e`, so a zero exit here would have let every
# check above run against a stale or absent file and still report success.
set +e
"$BIN" --to-mdl "$TMP/does-not-exist.mdl" > "$TMP/missing.out" 2> "$TMP/missing.err"
missing_status=$?
set -e
if [ "$missing_status" -eq 0 ]; then
  echo "error: converting a nonexistent path exited 0" >&2
  exit 1
fi
grep -q "couldn't open file" "$TMP/missing.err"

echo "CLI round-trip OK"
