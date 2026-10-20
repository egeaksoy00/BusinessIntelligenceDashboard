#ifndef DATA_H
#define DATA_H

#define MAX_SALES 1000
#define MAX_CATEGORIES 10
#define MONTH_COUNT 12

typedef struct {
    int id;
    char date[11];
    char category[30];
    char region[20];
    double revenue;
    double cost;
} Sale;

typedef struct {
    double totalRevenue;
    double totalCost;
    double totalProfit;
    int totalOrders;
    double averageOrderValue;
} DashboardStats;

// Load sales records from a CSV file.
// Returns the number of successfully loaded records.
// Returns -1 if the file cannot be opened or arguments are invalid.
int loadSales(const char *filename, Sale sales[], int maxSales);

// Calculate dashboard KPIs using the loaded sales.
DashboardStats calculateStats(const Sale sales[], int count);


// Print KPI results to the terminal for testing.
void printStats(DashboardStats stats);

typedef struct {
    char name[30];
    double revenue;
} CategoryRevenue;

// Calculate total revenue for each month (January to December).
void calculateMonthlyRevenue(
    const Sale sales[],
    int count,
    double monthlyRevenue[MONTH_COUNT]
);

int calculateCategoryRevenue(
    const Sale sales[],
    int count,
    CategoryRevenue categories[],
    int maxCategories
);

#endif