#include <stdio.h>
#include <string.h>
#include "data.h"

int loadSales(const char *filename, Sale sales[], int maxSales)
{
    if (filename == NULL || sales == NULL || maxSales <= 0) {
        return -1;
    }

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error: Could not open %s\n", filename);
        return -1;
    }

    char line[256];
    int count = 0;

    // Skip the CSV header.
    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return 0;
    }

    while (count < maxSales && fgets(line, sizeof(line), file) != NULL)
    {
        Sale sale;

        int fields = sscanf(
            line,
            "%d,%10[^,],%29[^,],%19[^,],%lf,%lf",
            &sale.id,
            sale.date,
            sale.category,
            sale.region,
            &sale.revenue,
            &sale.cost
        );

        if (fields == 6)
        {
            sales[count] = sale;
            count++;
        }
    }

    fclose(file);

    return count;
}

DashboardStats calculateStats(const Sale sales[], int count)
{
    DashboardStats stats = {0};

    if (sales == NULL || count <= 0) {
        return stats;
    }

    for (int i = 0; i < count; i++)
    {
        stats.totalRevenue += sales[i].revenue;
        stats.totalCost += sales[i].cost;
    }

    stats.totalProfit = stats.totalRevenue - stats.totalCost;
    stats.totalOrders = count;
    stats.averageOrderValue = stats.totalRevenue / count;

    return stats;
}

void printStats(DashboardStats stats)
{
    printf("\n========== BUSINESS INTELLIGENCE ==========\n");

    printf("Total Revenue:      %.2f\n", stats.totalRevenue);
    printf("Total Cost:         %.2f\n", stats.totalCost);
    printf("Total Profit:       %.2f\n", stats.totalProfit);
    printf("Total Orders:       %d\n", stats.totalOrders);
    printf("Average Order:      %.2f\n", stats.averageOrderValue);

    printf("===========================================\n");
}

void calculateMonthlyRevenue(
    const Sale sales[],
    int count,
    double monthlyRevenue[MONTH_COUNT]
)
{
    // Initialize all months to zero.
    for (int i = 0; i < MONTH_COUNT; i++)
    {
        monthlyRevenue[i] = 0.0;
    }

    if (sales == NULL || count <= 0)
    {
        return;
    }

    // Process each sale.
    for (int i = 0; i < count; i++)
    {
        int year;
        int month;
        int day;

        // Extract year, month and day from YYYY-MM-DD.
        int parsed = sscanf(
            sales[i].date,
            "%4d-%2d-%2d",
            &year,
            &month,
            &day
        );

        if (parsed == 3 && month >= 1 && month <= 12)
        {
            monthlyRevenue[month - 1] += sales[i].revenue;
        }
    }

    
}

int calculateCategoryRevenue(
    const Sale sales[],
    int count,
    CategoryRevenue categories[],
    int maxCategories
)
{
    if (sales == NULL || categories == NULL ||
        count <= 0 || maxCategories <= 0)
    {
        return 0;
    }

    int categoryCount = 0;

    for (int i = 0; i < count; i++)
    {
        int categoryIndex = -1;

        // Check whether the category already exists.
        for (int j = 0; j < categoryCount; j++)
        {
            if (strcmp(categories[j].name, sales[i].category) == 0)
            {
                categoryIndex = j;
                break;
            }
        }

        // Add a new category if necessary.
        if (categoryIndex == -1)
        {
            if (categoryCount >= maxCategories)
            {
                fprintf(stderr, "Warning: Category limit reached.\n");
                continue;
            }

            categoryIndex = categoryCount;

            snprintf(
                categories[categoryIndex].name,
                sizeof(categories[categoryIndex].name),
                "%s",
                sales[i].category
            );

            categories[categoryIndex].revenue = 0.0;

            categoryCount++;
        }

        // Add revenue to the matching category.
        categories[categoryIndex].revenue += sales[i].revenue;
    }

    return categoryCount;
}