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

#endif
