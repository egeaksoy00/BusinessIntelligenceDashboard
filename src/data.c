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