set term postscript eps enhanced color
set output "perf_plumer2048bare2048.ps"

set title "TEMPS MOYEN DE CALCUL - MPI avec recouvrement - Plummer 2048"
set xlabel "Nombre de processus"
set ylabel "Temps en secondes"

set size 2.0, 1
plot "perf_plumer2048bare2048" using 1:3 title "Temps de calcul avec 1 proc MPI par noeud" with linespoints 10, \
"perf_plumer2048bare2048" using 1:2 title "Temps de calcul avec 1 proc MPI par coeur" with linespoints 7