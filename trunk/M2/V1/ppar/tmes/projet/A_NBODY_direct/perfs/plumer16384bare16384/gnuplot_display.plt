set term postscript eps enhanced color
set output "perf_plumer16384bare16384.ps"

set title "TEMPS MOYEN DE CALCUL - MPI avec recouvrement - plumer16384bare16384"
set xlabel "Nombre de processus MPI"
set ylabel "Temps en secondes"

set size 2.0, 1
plot "perf_plumer16384bare16384" using 1:3 title "Temps de calcul avec 1 proc MPI par noeud" with linespoints 10, \
"perf_plumer16384bare16384" using 1:2 title "Temps de calcul avec 1 proc MPI par coeur" with linespoints 7
