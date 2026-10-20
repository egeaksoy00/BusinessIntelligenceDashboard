#include "filters.h"
#include <string.h>

int filterSalesByCategory(const Sale sales[], int count, const char *category,
                          Sale filteredSales[], int capacity)
{
    if (sales == NULL || filteredSales == NULL || count < 0 || capacity <= 0)
        return -1;

    int matched = 0;
    for (int i = 0; i < count && matched < capacity; ++i) {
        if (category == NULL || strcmp(sales[i].category, category) == 0)
            filteredSales[matched++] = sales[i];
    }
    return matched;
}
