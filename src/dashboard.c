#include "dashboard.h"
#include <stdio.h>

static const Color background = {242, 245, 249, 255};
static const Color ink = {27, 42, 65, 255};
static const Color muted = {100, 116, 139, 255};
static const Color border = {222, 229, 238, 255};
static const Color accent = {44, 111, 215, 255};

static void panel(Rectangle bounds, const char *title)
{
    DrawRectangleRec(bounds, WHITE);
    DrawRectangleLinesEx(bounds, 1, border);
    DrawText(title, (int)bounds.x + 22, (int)bounds.y + 20, 20, ink);
}

void drawKpiCard(Rectangle bounds, const char *title, const char *value)
{
    DrawRectangleRec(bounds, WHITE);
    DrawRectangleLinesEx(bounds, 1, border);
    DrawRectangle((int)bounds.x, (int)bounds.y, 4, (int)bounds.height, accent);
    DrawText(title, (int)bounds.x + 20, (int)bounds.y + 20, 17, muted);
    DrawText(value, (int)bounds.x + 20, (int)bounds.y + 52, 30, ink);
}

/* Scaling a chart is presentation logic; inputs are already aggregated. */
void drawMonthlyChart(Rectangle bounds, const double values[MONTH_COUNT])
{
    static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    double maximum = 1;
    panel(bounds, "Revenue by Month");
    for (int i = 0; i < MONTH_COUNT; ++i)
        if (values[i] > maximum) maximum = values[i];
    maximum *= 1.2f;
    float left = bounds.x + 68, top = bounds.y + 70;
    float width = bounds.width - 90, height = bounds.height - 114;
    for (int tick = 0; tick <= 4; ++tick) {
        float y = top + height * tick / 4;
        char label[32];
        snprintf(label, sizeof label, "%.0f", maximum * (4 - tick) / 4);
        DrawText(label, (int)left - 58, (int)y - 6, 13, muted);
        DrawLine((int)left, (int)y, (int)(left + width), (int)y, border);
    }
    float slot = width / MONTH_COUNT;
    for (int i = 0; i < MONTH_COUNT; ++i) {
        float barHeight = (values[i] > 0 ? values[i] : 0) / maximum * height;
        float x = left + slot * i + slot * 0.22f;
        Rectangle bar = {x, top + height - barHeight, slot * 0.56f, barHeight};
        DrawRectangleRec(bar, accent);
        DrawText(months[i], (int)(left + slot * i + slot / 2) - 12,
                 (int)(top + height + 14), 15, muted);
    }
}

void drawCategoryChart(Rectangle bounds, const double values[DASHBOARD_CATEGORY_COUNT])
{
    static const char *labels[] = {"Electronics", "Clothing", "Food"};
    double maximum = 1;
    panel(bounds, "Revenue by Category");
    for (int i = 0; i < DASHBOARD_CATEGORY_COUNT; ++i)
        if (values[i] > maximum) maximum = values[i];
    for (int i = 0; i < DASHBOARD_CATEGORY_COUNT; ++i) {
        char value[32];
        float y = bounds.y + 78 + i * 76;
        float width = bounds.width - 44;
        snprintf(value, sizeof value, "$%.0f", values[i]);
        DrawText(labels[i], (int)bounds.x + 22, (int)y, 16, muted);
        DrawText(value, (int)(bounds.x + bounds.width - 22) - MeasureText(value, 16),
                 (int)y, 16, ink);
        Rectangle track = {bounds.x + 22, y + 27, width, 18};
        DrawRectangleRec(track, background);
        track.width *= (values[i] > 0 ? values[i] : 0) / maximum;
        DrawRectangleRec(track, (Color){42, 151, 140, 255});
    }
}

void drawFilterButtons(Rectangle bounds, RegionFilter *selectedRegion)
{
    static const char *labels[] = {"ALL", "NORTH", "SOUTH", "EAST", "WEST"};
    for (int i = 0; i < 5; ++i) {
        Rectangle button = {bounds.x + i * 104, bounds.y, 92, bounds.height};
        bool hover = CheckCollisionPointRec(GetMousePosition(), button);
        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            *selectedRegion = (RegionFilter)i;
        bool active = *selectedRegion == (RegionFilter)i;
        DrawRectangleRec(button, active ? accent : (hover ? border : WHITE));
        DrawRectangleLinesEx(button, 1, active ? accent : border);
        DrawText(labels[i], (int)(button.x + (button.width - MeasureText(labels[i], 16)) / 2),
                 (int)button.y + 13, 16, active ? WHITE : ink);
    }
}

void drawDashboard(const DashboardViewData *data, RegionFilter *selectedRegion)
{
    char value[64];
    ClearBackground(background);
    DrawText("Business Intelligence Dashboard", 32, 26, 30, ink);
    DrawText("Sales performance overview  |  Mock data", 33, 66, 16, muted);
    const float cardWidth = 272;
    snprintf(value, sizeof value, "$%.2f", data->stats.totalRevenue);
    drawKpiCard((Rectangle){32, 108, cardWidth, 112}, "Total Revenue", value);
    snprintf(value, sizeof value, "$%.2f", data->stats.totalProfit);
    drawKpiCard((Rectangle){320, 108, cardWidth, 112}, "Total Profit", value);
    snprintf(value, sizeof value, "%d", data->stats.totalOrders);
    drawKpiCard((Rectangle){608, 108, cardWidth, 112}, "Total Orders", value);
    snprintf(value, sizeof value, "$%.2f", data->stats.averageOrderValue);
    drawKpiCard((Rectangle){896, 108, cardWidth, 112}, "Average Order Value", value);
    drawMonthlyChart((Rectangle){32, 244, 700, 326}, data->monthlyRevenue);
    drawCategoryChart((Rectangle){752, 244, 416, 326}, data->categoryRevenue);
    DrawText("Region", 32, 596, 18, ink);
    drawFilterButtons((Rectangle){132, 584, 508, 42}, selectedRegion);
    DrawText("Region selection is ready. Data filtering will be connected during backend integration.",
             32, 652, 15, muted);
}
