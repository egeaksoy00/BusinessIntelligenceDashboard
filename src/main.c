#include "raylib.h"
#include "dashboard.h"

int main(void)
{
    const DashboardViewData mockData = {
      .stats = {
        .totalRevenue = 124500.0f,
        .totalCost = 93300.0,
        .totalProfit = 31200.0f,
        .totalOrders = 1248,
        .averageOrderValue = 99.76f,
      },
        .monthlyRevenue = {1200, 1800, 2400, 1700, 3000, 2600,
                           2800, 3100, 2700, 3400, 3600, 4200},
        .categoryRevenue = {5000, 3200, 2100}
    };
    RegionFilter selectedRegion = REGION_ALL;

    InitWindow(1200, 700, "Business Intelligence Dashboard");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();
        drawDashboard(&mockData, &selectedRegion);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
