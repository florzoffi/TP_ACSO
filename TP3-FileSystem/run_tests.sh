#!/usr/bin/env bash
set -e

TEST_DIR=samples/testdisks
MY_OUT=outputs/my
GOLD_OUT=outputs/gold

mkdir -p $MY_OUT/ip $GOLD_OUT/ip
rm -f $MY_OUT/ip/*.txt $GOLD_OUT/ip/*.txt

IMAGES=($(ls $TEST_DIR | grep -v '\.gold$'))

echo "🔍 Ejecutando pruebas con las siguientes imágenes:"
printf ' - %s\n' "${IMAGES[@]}"

echo "▶ Ejecutando pruebas -ip..."
for img in "${IMAGES[@]}"; do
  ./diskimageaccess -ip $TEST_DIR/$img > $MY_OUT/ip/${img}.txt
  cp $TEST_DIR/${img}.gold $GOLD_OUT/ip/${img}.txt
done

echo "🔍 Comparando salidas con la cátedra..."

DIFF_FOUND=0

for file in "$MY_OUT/ip"/*.txt; do
  fname=$(basename "$file")
  gold_file="$GOLD_OUT/ip/$fname"
  if [ ! -f "$gold_file" ]; then
    echo "❌ Falta archivo de referencia: $gold_file"
    DIFF_FOUND=1
  elif ! diff -q "$file" "$gold_file" > /dev/null; then
    echo "❌ Diferencia encontrada: ip/$fname"
    DIFF_FOUND=1
  fi
done

if [ "$DIFF_FOUND" -eq 0 ]; then
  echo "✅ Todas las salidas coinciden con las de la cátedra."
else
  echo "⚠️ Algunas salidas no coinciden. Revisá los archivos marcados."
fi