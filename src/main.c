#include "raylib.h"
#include "dashboard.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Controller prepares display values; rendering never aggregates sales. */
static void refreshData(DashboardViewData *data, const Sale sales[], int count)
{
    data->revision++;
    data->sales = sales;
    data->saleCount = count;
    data->stats = calculateStats(sales, count);
    calculateMonthlyRevenue(sales, count, data->monthlyRevenue);
    data->categoryCount = calculateCategoryRevenue(sales, count, data->categories, MAX_CATEGORIES);
    data->profitMargin = data->stats.totalRevenue != 0
        ? data->stats.totalProfit / data->stats.totalRevenue * 100 : 0;
    data->activeMonths = 0;
    for (int i = 0; i < MONTH_COUNT; ++i)
        if (data->monthlyRevenue[i] != 0) data->activeMonths++;
    for (int i = 0; i < data->categoryCount; ++i)
        data->categoryShare[i] = data->stats.totalRevenue > 0
            ? data->categories[i].revenue / data->stats.totalRevenue * 100 : 0;
    data->firstDate[0] = data->lastDate[0] = '\0';
    for (int i = 0; i < count; ++i) {
        if (!data->firstDate[0] || strcmp(sales[i].date, data->firstDate) < 0)
            snprintf(data->firstDate, sizeof data->firstDate, "%s", sales[i].date);
        if (!data->lastDate[0] || strcmp(sales[i].date, data->lastDate) > 0)
            snprintf(data->lastDate, sizeof data->lastDate, "%s", sales[i].date);
    }
}

static void csvField(FILE *file, const char *value)
{
    fputc('"', file);
    for (; *value; ++value) {
        if (*value == '"') fputc('"', file);
        fputc(*value, file);
    }
    fputc('"', file);
}

static void exportData(const DashboardViewData *data, const char *region)
{
    /* Exclusive creation prevents overwriting an earlier report. */
    char filename[128];
    FILE *file = NULL;
    time_t timestamp = time(NULL);
    for (int attempt = 0; attempt < 100 && !file; ++attempt) {
        snprintf(filename, sizeof filename, "exports/sales-%s-%lld-%d.csv",
                 region, (long long)timestamp, attempt);
#ifdef _MSC_VER
        if (fopen_s(&file, filename, "wx") != 0) file = NULL;
#else
        file = fopen(filename, "wx");
#endif
    }
    if (!file) { setDashboardNotice("Export failed. Check the exports folder permissions."); return; }
    fprintf(file, "id,date,category,region,revenue,cost\n");
    for (int i = 0; i < data->saleCount; ++i) {
        const Sale *sale = &data->sales[i];
        fprintf(file, "%d,", sale->id); csvField(file, sale->date); fputc(',', file);
        csvField(file, sale->category); fputc(',', file); csvField(file, sale->region);
        fprintf(file, ",%.2f,%.2f\n", sale->revenue, sale->cost);
    }
    bool failed = ferror(file) != 0;
    if (fclose(file) != 0) failed = true;
    setDashboardNotice(failed ? "Could not finish writing the CSV report." : "Region report saved to exports/");
}

int main(void)
{
    static Sale sales[MAX_SALES];
    static Sale filteredSales[MAX_SALES];
    static const char *regions[] = {"All", "North", "South", "East", "West"};
    DashboardViewData data = {0};
    int count = loadSales("data/sales.csv", sales, MAX_SALES);
    if (count < 0) {
        fprintf(stderr, "Cannot load data/sales.csv. Run from the project root.\n");
        return 1;
    }
    RegionFilter selectedRegion = REGION_ALL;
    int previousRegion = -1;
    data.loadedCount = count;
    for (int i = 0; i < 5; ++i) {
        int n = filterSalesByRegion(sales, count, regions[i], filteredSales, MAX_SALES);
        data.regionStats[i] = calculateStats(filteredSales, n);
    }
    for (int i = 0; i < 5; ++i)
        data.regionShare[i] = data.regionStats[0].totalRevenue > 0
            ? data.regionStats[i].totalRevenue / data.regionStats[0].totalRevenue * 100 : 0;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1440, 960, "BIDashboard | Business Intelligence Dashboard");
    MaximizeWindow();
    SetWindowMinSize(1080, 720);
    SetExitKey(KEY_NULL);
    if (!DirectoryExists("exports")) MakeDirectory("exports");
    initDashboard();
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        updateDashboardInput(&selectedRegion);
        bool snapshotRequested = IsKeyPressed(KEY_F10) || takeDashboardSnapshotRequest();
        if ((int)selectedRegion != previousRegion) {
            int filteredCount = filterSalesByRegion(sales, count, regions[selectedRegion],
                                                    filteredSales, MAX_SALES);
            refreshData(&data, filteredSales, filteredCount);
            previousRegion = (int)selectedRegion;
        }
        if (takeDashboardExportRequest()) exportData(&data, regions[selectedRegion]);
        BeginDrawing();
        drawDashboard(&data, &selectedRegion);
        EndDrawing();
        if (snapshotRequested) {
            char filename[128];
            time_t timestamp = time(NULL);
            for (int attempt = 0; attempt < 100; ++attempt) {
                snprintf(filename, sizeof filename, "exports/dashboard-%lld-%d.png", (long long)timestamp, attempt);
                if (!FileExists(filename)) {
                    bool saved = saveDashboardSnapshot(filename, &data, &selectedRegion);
                    setDashboardNotice(saved ? "Dashboard snapshot saved to exports/" : "Could not save the dashboard snapshot.");
                    break;
                }
            }
        }
    }
    closeDashboard();
    CloseWindow();
    return 0;
}
