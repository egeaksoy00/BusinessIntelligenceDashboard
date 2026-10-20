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

    // Step 3: Print KPI results.
    printStats(stats);

    // Step 4: Calculate monthly revenue.
    double monthlyRevenue[MONTH_COUNT];

    calculateMonthlyRevenue(
        sales,
        count,
        monthlyRevenue
    );

    // Step 5: Print monthly revenue.
    const char *monthNames[MONTH_COUNT] = {
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"
    };

    printf("\n========== MONTHLY REVENUE ==========\n");

    for (int i = 0; i < MONTH_COUNT; i++)
    {
        printf(
            "%-12s %.2f\n",
            monthNames[i],
            monthlyRevenue[i]
        );
    }

    printf("=====================================\n");

    // Calculate category revenue.
CategoryRevenue categories[MAX_CATEGORIES];

int categoryCount = calculateCategoryRevenue(
    sales,
    count,
    categories,
    MAX_CATEGORIES
);

printf("\n========== CATEGORY REVENUE ==========\n");

double categoryTotal = 0.0;

for (int i = 0; i < categoryCount; i++)
{
    printf(
        "%-15s %.2f\n",
        categories[i].name,
        categories[i].revenue
    );

    categoryTotal += categories[i].revenue;
}

printf("--------------------------------------\n");
printf("Total:           %.2f\n", categoryTotal);
printf("======================================\n");

    return 0;
}