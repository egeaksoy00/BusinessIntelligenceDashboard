#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "raylib.h"
#include "data.h"

#define DASHBOARD_CATEGORY_COUNT 3

typedef enum {
    REGION_ALL, REGION_NORTH, REGION_SOUTH, REGION_EAST, REGION_WEST
} RegionFilter;

/* Prepared values only: the UI never reads sales or calculates business totals. */
typedef struct {
    DashboardStats stats;
    double monthlyRevenue[MONTH_COUNT];
    double categoryRevenue[DASHBOARD_CATEGORY_COUNT];
} DashboardViewData;

void drawKpiCard(Rectangle bounds, const char *title, const char *value);
void drawMonthlyChart(Rectangle bounds, const double values[MONTH_COUNT]);
void drawCategoryChart(Rectangle bounds, const double values[DASHBOARD_CATEGORY_COUNT]);
void drawFilterButtons(Rectangle bounds, RegionFilter *selectedRegion);
void drawDashboard(const DashboardViewData *data, RegionFilter *selectedRegion);

#endif
