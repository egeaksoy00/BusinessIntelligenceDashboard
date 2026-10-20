#ifndef DASHBOARD_FILTERS_H
#define DASHBOARD_FILTERS_H

#include "data.h"

/* NULL category includes all categories. Returns -1 for invalid arguments.
   This controller helper complements Ege's existing region filter. */
int filterSalesByCategory(const Sale sales[], int count, const char *category,
                          Sale filteredSales[], int capacity);

#endif
