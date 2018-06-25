#!/bin/sh
mkdir -p "$PWD/out/generated"

input_dir="$PWD/src"
output_dir="$PWD/out/generated"

echo "Generate QT files: $input_dir"

find $input_dir -name '*.h' | xargs grep -l 'Q_OBJECT' | sed 's/\.h$//' | while read f; do
    output_file="${output_dir}/moc_$(basename $f).cpp"
    if ! test -f ${output_file} || test $f.h -nt ${output_file}; then
    	$QTDIR/bin/moc -o ${output_file} $f.h
    fi
done

find $input_dir -name '*.ui' | sed -e 's/\.ui$//' | while read f; do
	output_file="${output_dir}/ui_$(basename $f).h"
    if ! test -f ${output_file} || test $f.ui -nt ${output_file}; then
    	$QTDIR/bin/uic -o ${output_file} $f.ui
    fi
done

find $input_dir -name '*.qrc' | sed -e 's/\.qrc$//' | while read f; do
    output_file="${output_dir}/qrc_$(basename $f).cpp"
    $QTDIR/bin/rcc -o ${output_file} $f.qrc -name $(basename $f)
done
