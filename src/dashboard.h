#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "raylib.h"
#include "data.h"

typedef enum {
    REGION_ALL, REGION_NORTH, REGION_SOUTH, REGION_EAST, REGION_WEST
} RegionFilter;

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
    double regionShare[5];
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
void updateDashboardInput(RegionFilter *selectedRegion);
bool takeDashboardExportRequest(void);
bool takeDashboardSnapshotRequest(void);
void setDashboardNotice(const char *message);
bool saveDashboardSnapshot(const char *filename, const DashboardViewData *data, RegionFilter *selectedRegion);
void drawFilterButtons(Rectangle bounds, RegionFilter *selectedRegion);
void drawDashboard(const DashboardViewData *data, RegionFilter *selectedRegion);

#endif
