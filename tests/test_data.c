#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "../src/data.h"

#define EPSILON 0.001

void testCSVLoading(void)
{
    Sale sales[MAX_SALES];

    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    assert(count == 12);
    assert(sales[0].id == 1);
    assert(strcmp(sales[0].category, "Electronics") == 0);

    printf("[PASS] CSV Loading\n");
}

void testKPICalculations(void)
{
    Sale sales[MAX_SALES];

    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    assert(count == 12);

    DashboardStats stats = calculateStats(sales, count);

    assert(fabs(stats.totalRevenue - 11000.0) < EPSILON);
    assert(fabs(stats.totalCost - 6530.0) < EPSILON);
    assert(fabs(stats.totalProfit - 4470.0) < EPSILON);
    assert(stats.totalOrders == 12);
    assert(fabs(stats.averageOrderValue - 916.6667) < EPSILON);

    printf("[PASS] KPI Calculations\n");
}

void testMonthlyRevenue(void)
{
    Sale sales[MAX_SALES];

    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    assert(count == 12);

    double monthlyRevenue[MONTH_COUNT];

    calculateMonthlyRevenue(
        sales,
        count,
        monthlyRevenue
    );

    assert(fabs(monthlyRevenue[0] - 2000.0) < EPSILON);
    assert(fabs(monthlyRevenue[1] - 2770.0) < EPSILON);
    assert(fabs(monthlyRevenue[2] - 3330.0) < EPSILON);
    assert(fabs(monthlyRevenue[3] - 2900.0) < EPSILON);

    for (int i = 4; i < MONTH_COUNT; i++)
    {
        assert(fabs(monthlyRevenue[i]) < EPSILON);
    }

    printf("[PASS] Monthly Revenue\n");
}

void testCategoryRevenue(void)
{
    Sale sales[MAX_SALES];

    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    assert(count == 12);

    CategoryRevenue categories[MAX_CATEGORIES];

    int categoryCount = calculateCategoryRevenue(
        sales,
        count,
        categories,
        MAX_CATEGORIES
    );

    assert(categoryCount == 3);

    double electronics = 0.0;
    double clothing = 0.0;
    double food = 0.0;

    for (int i = 0; i < categoryCount; i++)
    {
        if (strcmp(categories[i].name, "Electronics") == 0)
        {
            electronics = categories[i].revenue;
        }
        else if (strcmp(categories[i].name, "Clothing") == 0)
        {
            clothing = categories[i].revenue;
        }
        else if (strcmp(categories[i].name, "Food") == 0)
        {
            food = categories[i].revenue;
        }
    }

    assert(fabs(electronics - 6600.0) < EPSILON);
    assert(fabs(clothing - 2850.0) < EPSILON);
    assert(fabs(food - 1550.0) < EPSILON);

    printf("[PASS] Category Revenue\n");
}

void testRegionFiltering(void)
{
    Sale sales[MAX_SALES];
    Sale filteredSales[MAX_SALES];

    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    assert(count == 12);

    const char *regions[] = {
        "All", "North", "South", "East", "West", "Unknown"
    };

    double expectedRevenue[] = {
        11000.0, 2200.0, 2420.0, 3420.0, 2960.0, 0.0
    };

    int expectedOrders[] = {
        12, 3, 3, 3, 3, 0
    };

    for (int i = 0; i < 6; i++)
    {
        int filteredCount = filterSalesByRegion(
            sales,
            count,
            regions[i],
            filteredSales,
            MAX_SALES
        );

        DashboardStats stats = calculateStats(
            filteredSales,
            filteredCount
        );

        assert(filteredCount == expectedOrders[i]);
        assert(stats.totalOrders == expectedOrders[i]);

        assert(
            fabs(stats.totalRevenue - expectedRevenue[i]) < EPSILON
        );
    }

    printf("[PASS] Region Filtering\n");
}

void testErrorHandling(void)
{
    Sale sales[MAX_SALES];

    int count = loadSales(
        "data/nonexistent.csv",
        sales,
        MAX_SALES
    );

    assert(count == -1);

    DashboardStats stats = calculateStats(NULL, 0);

    assert(stats.totalRevenue == 0.0);
    assert(stats.totalOrders == 0);

    double monthlyRevenue[MONTH_COUNT];

    calculateMonthlyRevenue(NULL, 0, monthlyRevenue);

    for (int i = 0; i < MONTH_COUNT; i++)
    {
        assert(monthlyRevenue[i] == 0.0);
    }

    printf("[PASS] Error Handling\n");
}

int main(void)
{
    printf("\n========== BACKEND TEST SUITE ==========\n\n");

    testCSVLoading();
    testKPICalculations();
    testMonthlyRevenue();
    testCategoryRevenue();
    testRegionFiltering();
    testErrorHandling();

    printf("\nAll backend tests passed successfully!\n");
    printf("========================================\n");

    return 0;
}