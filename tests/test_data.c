#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "../src/data.h"

#define EPSILON 0.01

#define EXPECTED_RECORDS 300
#define EXPECTED_CATEGORIES 8

#define EXPECTED_REVENUE 172571.93
#define EXPECTED_COST 107496.10
#define EXPECTED_PROFIT 65075.83

// Compare two floating-point values.
int almostEqual(double a, double b)
{
    return fabs(a - b) < EPSILON;
}

// Test 1: CSV Loading
void testCSVLoading(void)
{
    Sale sales[MAX_SALES];

    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    assert(count == EXPECTED_RECORDS);

    assert(sales[0].id == 1);
    assert(strcmp(sales[0].category, "Electronics") == 0);

    assert(sales[count - 1].id == 300);

    printf("[PASS] CSV Loading - 300 records\n");
}

// Test 2: KPI Calculations
void testKPICalculations(void)
{
    Sale sales[MAX_SALES];

    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    assert(count == EXPECTED_RECORDS);

    DashboardStats stats = calculateStats(sales, count);

    assert(almostEqual(stats.totalRevenue, EXPECTED_REVENUE));
    assert(almostEqual(stats.totalCost, EXPECTED_COST));
    assert(almostEqual(stats.totalProfit, EXPECTED_PROFIT));

    assert(stats.totalOrders == EXPECTED_RECORDS);

    assert(almostEqual(
        stats.averageOrderValue,
        EXPECTED_REVENUE / EXPECTED_RECORDS
    ));

    assert(almostEqual(
        stats.totalRevenue - stats.totalCost,
        stats.totalProfit
    ));

    printf("[PASS] KPI Calculations\n");
}

// Test 3: Monthly Revenue
void testMonthlyRevenue(void)
{
    Sale sales[MAX_SALES];

    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    assert(count == EXPECTED_RECORDS);

    double monthlyRevenue[MONTH_COUNT];

    calculateMonthlyRevenue(
        sales,
        count,
        monthlyRevenue
    );

    double expectedMonthly[MONTH_COUNT] = {
        19879.40,  // January
        21140.77,  // February
        16748.08,  // March
        15889.38,  // April
        16356.93,  // May
        14523.26,  // June
        20330.17,  // July
        12825.82,  // August
        15589.37,  // September
        19288.75,  // October
        0.0,       // November
        0.0        // December
    };

    double total = 0.0;

    for (int i = 0; i < MONTH_COUNT; i++)
    {
        assert(almostEqual(
            monthlyRevenue[i],
            expectedMonthly[i]
        ));

        total += monthlyRevenue[i];
    }

    assert(almostEqual(total, EXPECTED_REVENUE));

    printf("[PASS] Monthly Revenue - 12 months\n");
}

// Test 4: Category Revenue
void testCategoryRevenue(void)
{
    Sale sales[MAX_SALES];

    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    assert(count == EXPECTED_RECORDS);

    CategoryRevenue categories[MAX_CATEGORIES];

    int categoryCount = calculateCategoryRevenue(
        sales,
        count,
        categories,
        MAX_CATEGORIES
    );

    assert(categoryCount == EXPECTED_CATEGORIES);

    const char *expectedNames[] = {
        "Electronics",
        "Clothing",
        "Food",
        "Home",
        "Beauty",
        "Sports",
        "Books",
        "Toys"
    };

    double expectedRevenue[] = {
        82140.38,
        16798.08,
        7491.39,
        23385.17,
        8817.93,
        24990.10,
        3309.28,
        5639.60
    };

    double total = 0.0;

    for (int i = 0; i < EXPECTED_CATEGORIES; i++)
    {
        int found = 0;

        for (int j = 0; j < categoryCount; j++)
        {
            if (strcmp(
                categories[j].name,
                expectedNames[i]
            ) == 0)
            {
                assert(almostEqual(
                    categories[j].revenue,
                    expectedRevenue[i]
                ));

                found = 1;
                break;
            }
        }

        assert(found == 1);
    }

    for (int i = 0; i < categoryCount; i++)
    {
        total += categories[i].revenue;
    }

    assert(almostEqual(total, EXPECTED_REVENUE));

    printf("[PASS] Category Revenue - 8 categories\n");
}

// Test 5: Region Filtering
void testRegionFiltering(void)
{
    Sale sales[MAX_SALES];
    Sale filteredSales[MAX_SALES];

    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    assert(count == EXPECTED_RECORDS);

    const char *regions[] = {
        "All",
        "North",
        "South",
        "East",
        "West",
        "Unknown"
    };

    int expectedOrders[] = {
        300,
        70,
        73,
        83,
        74,
        0
    };

    double expectedRevenue[] = {
        172571.93,
        40176.83,
        38907.99,
        46656.41,
        46830.70,
        0.0
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

        assert(filteredCount == expectedOrders[i]);

        DashboardStats stats = calculateStats(
            filteredSales,
            filteredCount
        );

        assert(stats.totalOrders == expectedOrders[i]);

        assert(almostEqual(
            stats.totalRevenue,
            expectedRevenue[i]
        ));

        // Verify that every filtered record
        // belongs to the selected region.
        if (strcmp(regions[i], "All") != 0)
        {
            for (int j = 0; j < filteredCount; j++)
            {
                assert(strcmp(
                    filteredSales[j].region,
                    regions[i]
                ) == 0);
            }
        }
    }

    printf("[PASS] Region Filtering - All regions\n");
}

// Test 6: Filtered Analytics
void testFilteredAnalytics(void)
{
    Sale sales[MAX_SALES];
    Sale filteredSales[MAX_SALES];

    int count = loadSales(
        "data/sales.csv",
        sales,
        MAX_SALES
    );

    assert(count == EXPECTED_RECORDS);

    const char *regions[] = {
        "North", "South", "East", "West"
    };

    for (int r = 0; r < 4; r++)
    {
        int filteredCount = filterSalesByRegion(
            sales,
            count,
            regions[r],
            filteredSales,
            MAX_SALES
        );

        DashboardStats stats = calculateStats(
            filteredSales,
            filteredCount
        );

        // Monthly revenue for selected region.
        double monthlyRevenue[MONTH_COUNT];

        calculateMonthlyRevenue(
            filteredSales,
            filteredCount,
            monthlyRevenue
        );

        double monthlyTotal = 0.0;

        for (int i = 0; i < MONTH_COUNT; i++)
        {
            monthlyTotal += monthlyRevenue[i];
        }

        // Category revenue for selected region.
        CategoryRevenue categories[MAX_CATEGORIES];

        int categoryCount = calculateCategoryRevenue(
            filteredSales,
            filteredCount,
            categories,
            MAX_CATEGORIES
        );

        double categoryTotal = 0.0;

        for (int i = 0; i < categoryCount; i++)
        {
            categoryTotal += categories[i].revenue;
        }

        // Both charts must match the KPI revenue.
        assert(almostEqual(
            monthlyTotal,
            stats.totalRevenue
        ));

        assert(almostEqual(
            categoryTotal,
            stats.totalRevenue
        ));
    }

    printf("[PASS] Filtered Analytics Consistency\n");
}

// Test 7: Error Handling
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
    assert(stats.totalCost == 0.0);
    assert(stats.totalProfit == 0.0);
    assert(stats.totalOrders == 0);

    double monthlyRevenue[MONTH_COUNT];

    calculateMonthlyRevenue(
        NULL,
        0,
        monthlyRevenue
    );

    for (int i = 0; i < MONTH_COUNT; i++)
    {
        assert(monthlyRevenue[i] == 0.0);
    }

    // Unknown region should return no records.
    int filteredCount = filterSalesByRegion(
        sales,
        0,
        "Unknown",
        sales,
        MAX_SALES
    );

    assert(filteredCount == 0);

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
    testFilteredAnalytics();
    testErrorHandling();

    printf("\nAll 7 backend tests passed successfully!\n");
    printf("========================================\n");

    return 0;
}