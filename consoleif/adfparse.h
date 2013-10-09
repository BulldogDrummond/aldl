/*
 * adfparse.h
 *
 *  Some ideas right now for functions available from parsing the adf.
 */

#ifndef ADFPARSE_H_
#define ADFPARSE_H_

#include "../aldl-io/aldldata.h"

struct gauge_info {
    char *title;    /* Name of gauge */
    char *unit;     /* Unit type as string */
    int item_id;    /* Item identifier in adx */
    int gauge_type; /* Type of gauge; right now support status gauges and treat digital num/analog as same */
    int range_low;  /* Lowest value for gauge */
    int range_high; /* Highest value for gauge */
    int left;       /* Next 4 would currently be unused */
    int right;
    int top;
    int bottom;
};

int adf_open(char *afile); /* Open the adx file and return 0 for success */

/* Would return a pointer to a structure containing info to extract values fron the datastream */
struct aldl_info *adf_item_aldl_info(int item_id);
/* Would return pointer to info for all gauges to be used.  Dashname specifies dash in the adx...
 * could use a hardcoded dash name maybe instead so that the dash is specifically configured for
 * the raspberry pi */
struct gauge_info **adf_getdash(char *dashname);

#endif /* ADFPARSE_H_ */
