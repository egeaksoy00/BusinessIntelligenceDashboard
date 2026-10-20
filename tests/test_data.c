#include <stdio.h>
#include "../src/data.h"

int main(void)
{
    Sale sales[MAX_SALES];

    // Step 1: Load CSV data.
    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    if (count < 0)
    {
        printf("Failed to load sales data.\n");
        return 1;
    }

    printf("Successfully loaded %d sales records.\n", count);

    // Step 2: Calculate dashboard statistics.
    DashboardStats stats = calculateStats(sales, count);

    // Step 3: Print results.
    printStats(stats);

    return 0;
}