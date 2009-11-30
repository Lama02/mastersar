set term postscript eps enhanced color
set output "perf_plumer8192bare8192.ps"

set title "TEMPS MOYEN DE CALCUL - MPI-openMP avec recouvrement VS MPI avec recouvrement - plumer8192bare8192"
set xlabel "Nombre de processus MPI"
set ylabel "Temps en secondes"

set size 2.0, 1
plot "perf_plumer8192bare8192_openMP" using 1:2 title "Temps de calcul avec 1 processus MPI-MP par noeud" with linespoints 10, \
"perf_plumer8192bare8192_MPI" using 1:3 title "Temps de calcul avec 1 proc MPI par noeud" with linespoints 7, \
"perf_plumer8192bare8192_MPI" using 1:2 title "Temps de calcul avec 1 proc MPI par coeur" with linespoints 5