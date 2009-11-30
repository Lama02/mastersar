set term postscript eps enhanced color
set output "perf_plumer4096bare4096.ps"

set title "TEMPS MOYEN DE CALCUL - MPI + openMP avec recouvrement - plumer4096bare4096"
set xlabel "Nombre de processus MPI"
set ylabel "Temps en secondes"

set size 2.0, 1
plot "perf_plumer4096bare4096" using 1:2 title "Temps de calcul avec 1 processus MPI par coeur" with linespoints 7
