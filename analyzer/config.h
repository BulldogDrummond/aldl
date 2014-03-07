#define ANL_VERSION "0.7"
#define ANL_CONFIGFILE "/etc/aldl/analyzer.conf"

/* configure analysis grids */
#define GRID_RPM_RANGE 7000
#define GRID_RPM_INTERVAL 400
#define GRID_MAP_RANGE 100
#define GRID_MAP_INTERVAL 10
#define GRID_MAF_RANGE 250
#define GRID_MAF_INTERVAL 5

/* reject blm cell 17, so decel events dont fuck up results */
#define REJECTDECEL

#define MIN_RPM 200
