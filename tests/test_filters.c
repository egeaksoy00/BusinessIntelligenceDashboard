#include "filters.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* Fixed fixtures keep this test independent of changes to sales.csv.
   Explicit checks remain enabled in Release builds. */
#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "Check failed at line %d: %s\n", __LINE__, #condition); \
        return 1; \
    } \
} while (0)

int main(void)
{
    const Sale source[] = {
        {1, "2026-01-05", "Electronics", "North", 1200, 800},
        {2, "2026-01-08", "Clothing", "South", 500, 250},
        {3, "2026-02-01", "Electronics", "South", 1500, 950},
        {4, "2026-02-02", "Books", "North", 200, 80},
        {5, "2026-03-01", "Food", "West", 100, 60},
        {6, "2026-03-03", "Electronics", "East", 2200, 1400}
    };
    Sale categories[6], regions[6];
    int count = filterSalesByCategory(source, 6, NULL, categories, 6);
    CHECK(count == 6);
    CHECK(memcmp(source, categories, sizeof source) == 0);
    CHECK(fabs(calculateStats(categories, count).totalRevenue - 5700) < 0.001);

    count = filterSalesByCategory(source, 6, "Electronics", categories, 6);
    CHECK(count == 3);
    CHECK(fabs(calculateStats(categories, count).totalRevenue - 4900) < 0.001);
    CHECK(categories[0].id == 1 && categories[1].id == 3 && categories[2].id == 6);

    int n = filterSalesByRegion(categories, count, "North", regions, 6);
    CHECK(n == 1 && regions[0].id == 1);
    CHECK(fabs(calculateStats(regions, n).totalProfit - 400) < 0.001);
    n = filterSalesByRegion(categories, count, "South", regions, 6);
    CHECK(n == 1 && regions[0].id == 3);
    n = filterSalesByRegion(categories, count, "All", regions, 6);
    CHECK(n == 3);

    count = filterSalesByCategory(source, 6, "Books", categories, 6);
    n = filterSalesByRegion(categories, count, "West", regions, 6);
    CHECK(n == 0);
    DashboardStats empty = calculateStats(regions, n);
    CHECK(empty.totalOrders == 0 && empty.totalRevenue == 0 && empty.averageOrderValue == 0);
    double monthly[MONTH_COUNT];
    calculateMonthlyRevenue(regions, n, monthly);
    for (int i = 0; i < MONTH_COUNT; ++i) CHECK(monthly[i] == 0);
    CategoryRevenue aggregated[MAX_CATEGORIES];
    CHECK(calculateCategoryRevenue(regions, n, aggregated, MAX_CATEGORIES) == 0);

    CHECK(filterSalesByCategory(source, 6, "Unknown", categories, 6) == 0);
    CHECK(filterSalesByCategory(source, 0, NULL, categories, 6) == 0);
    CHECK(filterSalesByCategory(source, 6, NULL, categories, 2) == 2);
    CHECK(categories[0].id == 1 && categories[1].id == 2);
    CHECK(filterSalesByCategory(NULL, 6, NULL, categories, 6) == -1);
    CHECK(filterSalesByCategory(source, -1, NULL, categories, 6) == -1);
    CHECK(filterSalesByCategory(source, 6, NULL, NULL, 6) == -1);
    CHECK(filterSalesByCategory(source, 6, NULL, categories, 0) == -1);

    puts("PASS: category, combined region/category, empty results, capacities, and invalid inputs.");
    return 0;
}
