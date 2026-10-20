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

    

// Step 6: Test region filtering.
Sale filteredSales[MAX_SALES];

const char *selectedRegion = "North";

int filteredCount = filterSalesByRegion(
    sales,
    count,
    selectedRegion,
    filteredSales,
    MAX_SALES
);

printf("\n========== REGION FILTER ==========\n");
printf("Selected Region: %s\n", selectedRegion);
printf("Matching Records: %d\n", filteredCount);

// Calculate KPIs using filtered sales.
DashboardStats filteredStats = calculateStats(
    filteredSales,
    filteredCount
);

printStats(filteredStats);

// Calculate monthly revenue using filtered sales.
double filteredMonthlyRevenue[MONTH_COUNT];

calculateMonthlyRevenue(
    filteredSales,
    filteredCount,
    filteredMonthlyRevenue
);

printf("\n====== FILTERED MONTHLY REVENUE ======\n");

for (int i = 0; i < MONTH_COUNT; i++)
{
    printf(
        "%-12s %.2f\n",
        monthNames[i],
        filteredMonthlyRevenue[i]
    );
}

// Calculate category revenue using filtered sales.
CategoryRevenue filteredCategories[MAX_CATEGORIES];

int filteredCategoryCount = calculateCategoryRevenue(
    filteredSales,
    filteredCount,
    filteredCategories,
    MAX_CATEGORIES
);

printf("\n====== FILTERED CATEGORY REVENUE ======\n");

for (int i = 0; i < filteredCategoryCount; i++)
{
    printf(
        "%-15s %.2f\n",
        filteredCategories[i].name,
        filteredCategories[i].revenue
    );
}

return 0;
}