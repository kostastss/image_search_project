#!/bin/bash

# Ορισμός των μεθόδων και των μεγεθών D
METHODS=("exact" "lsh" "hypercube" "ivfflat" "ivfpq")
D_VALUES=(1000 5000 10000 25000)

# Δημιουργία φακέλου για τα αποτελέσματα
mkdir -p results_test

echo "Ξεκινάει η εκτέλεση των πειραμάτων στο Test Set..."
echo "Τα αποτελέσματα θα αποθηκευτούν στον φάκελο 'results_test/'"
echo "---------------------------------------------------"

for d in "${D_VALUES[@]}"; do
    for method in "${METHODS[@]}"; do
        output_file="results_test/output_${method}_D${d}.txt"
        
        echo "[$(date +'%H:%M:%S')] Εκτέλεση: Μέθοδος = ${method}, Distractors (D) = ${d}"
        
        # Εκτέλεση της εντολής
        ./bin/search-hp -hp data/hpatches -mir data/mir -split data/split.txt -vocab 256 -set test -D ${d} -${method} -o "${output_file}"
        
        echo " -> Αποθηκεύτηκε στο ${output_file}"
        echo "---------------------------------------------------"
    done
done

echo "Όλα τα πειράματα ολοκληρώθηκαν με επιτυχία!"
