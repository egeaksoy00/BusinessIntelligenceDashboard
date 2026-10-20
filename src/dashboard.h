#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "raylib.h"
#include "data.h"

typedef enum {
    REGION_ALL, REGION_NORTH, REGION_SOUTH, REGION_EAST, REGION_WEST
} RegionFilter;

typedef struct {
    RegionFilter region;
    int categoryIndex; /* 0 = all; 1..categoryOptionCount = option index + 1. */
} DashboardFilters;

/* Prepared analytics and immutable records for the transaction table. */
typedef struct {
    DashboardStats stats;
    double monthlyRevenue[MONTH_COUNT];
    CategoryRevenue categories[MAX_CATEGORIES];
    int categoryCount;
    const Sale *sales;
    int saleCount;
    int loadedCount;
    DashboardStats regionStats[5];
    double categoryShare[MAX_CATEGORIES];
    double regionalRevenueCeiling;
    CategoryRevenue categoryOptions[MAX_CATEGORIES];
    int categoryOptionCount;
    double profitMargin;
    int activeMonths;
    char firstDate[11];
    char lastDate[11];
    unsigned int revision;
} DashboardViewData;

/* Call after InitWindow, and release fonts before CloseWindow. */
void initDashboard(void);
void closeDashboard(void);
void drawKpiCard(Rectangle bounds, const char *title, const char *value);
void drawMonthlyChart(Rectangle bounds, const double values[MONTH_COUNT]);
void drawCategoryChart(Rectangle bounds, const CategoryRevenue categories[], int categoryCount);
void updateDashboardInput(DashboardFilters *filters, const DashboardViewData *data);
bool takeDashboardExportRequest(void);
bool takeDashboardSnapshotRequest(void);
void setDashboardNotice(const char *message);
bool saveDashboardSnapshot(const char *filename, const DashboardViewData *data, DashboardFilters *filters);
void drawDashboard(const DashboardViewData *data, DashboardFilters *filters);

#endif
