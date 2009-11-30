

if [ $# -ne 3 ]; then
    echo "Exemple d'usage : $0 rep_perfs rep_logs plumer2048bare2048 "
    exit 1
fi

PERF_REP="$1"
LOGS_REP="$2"
PLUMER="$3"


if [ ! -d ${LOGS_REP} ]; then
    echo "Le repertoire des logs : ${LOGS_REP} n'existe pas. Lancer vos calculs d'abaord !" 
    exit 1
fi

if [ ! -f ${LOGS_REP}/${PLUMER}_2proc_1machine ]; then
    echo "Le fichier ${LOGS_REP}/${PLUMER}_2proc_1machine n'existe pas."
    exit 1
fi

if [ ! -d ${PERF_REP} ]; then
    echo "Creation du repertoire ${PERF_REP}..."
    mkdir ${PERF_REP}
fi

if [ ! -d ${PERF_REP}/${PLUMER} ]; then
    echo "Creation du repertoire ${PERF_REP}/${PLUMER}..."
    mkdir ${PERF_REP}/${PLUMER}
fi


ruby generate_stats_fils.rb ${PERF_REP}/${PLUMER}/perf_${PLUMER}_MPI ${LOGS_REP}/${PLUMER}_2proc_1machine ${LOGS_REP}/${PLUMER}_2proc_2machine ${LOGS_REP}/${PLUMER}_4proc_2machine ${LOGS_REP}/${PLUMER}_4proc_4machine ${LOGS_REP}/${PLUMER}_8proc_4machine ${LOGS_REP}/${PLUMER}_8proc_8machine ${LOGS_REP}/${PLUMER}_16proc_8machine ${LOGS_REP}/${PLUMER}_16proc_16machine