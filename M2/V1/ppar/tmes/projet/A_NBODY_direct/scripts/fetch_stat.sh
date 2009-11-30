

if [ $# -ne 3 ]; then
    echo "Exemple d'usage : $0 rep_perfs rep_logs plumer16384bare16384 "
    exit 1
fi

PERF_REP="$1"
LOGS_REP="$2"
PLUMER="$3"

ruby generate_stats_fils.rb ${PERF_REP}/${PLUMER}/perf_${PLUMER} ${LOGS_REP}/${PLUMER}_2proc_1machine ${LOGS_REP}/${PLUMER}_2proc_2machine ${LOGS_REP}/${PLUMER}_4proc_2machine ${LOGS_REP}/${PLUMER}_4proc_4machine ${LOGS_REP}/${PLUMER}_8proc_4machine ${LOGS_REP}/${PLUMER}_8proc_8machine ${LOGS_REP}/${PLUMER}_16proc_8machine ${LOGS_REP}/${PLUMER}_16proc_16machine