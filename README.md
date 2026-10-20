# BIDashboard | Business Intelligence Dashboard

A desktop Business Intelligence Dashboard prototype developed in **C** using the **raylib** graphics library.

![BIDashboard overview](docs/bidashboard-preview.png)

[Category theme preview: Food](docs/bidashboard-category-preview.png)

## Project Overview

This project is being developed as part of the Advanced Programming AI Project.

The application will visualize business data through an interactive dashboard, allowing users to monitor key performance indicators (KPIs), explore charts, and analyse sales performance.

## Features

- Dashboard interface built with raylib
- CSV data loading
- Revenue, Profit and Orders KPIs
- Monthly sales charts
- Category and region analysis
- Combined region and category filtering
- Charcoal theme, Lato typography, and category-aware accent colors
- Profit margin ring and regional revenue comparison
- Monthly bar/line chart with hover values
- Transaction search, date/revenue sorting, and pagination
- Region CSV export and PNG dashboard snapshots
- Resizable desktop UI with aligned input coordinates

## Technologies

- C
- raylib
- Git & GitHub

## Project Structure

```text
src/        Source code
data/       CSV datasets
assets/     Images, icons, fonts
docs/       Development journal and documentation
```

## Team

- Ege Aksoy
- Umut Ali Yalcin

## Status

The dashboard displays CSV-backed KPI cards, a 12-month revenue chart, dynamic
category contribution charts, region buttons, and a category dropdown. Both filters update all metrics and
charts using the backend calculation functions.

The sample dataset contains 300 transactions, eight categories, and four regions
from January through October 2026. November and December remain zero. Category
rows adapt to the category count and retain both revenue and contribution shares.
For all regions, expected revenue is $172,571.93, cost is $107,496.10, profit is
$65,075.83, and average order value is $575.24.

## Build and run

Requires CMake 3.16+, Git, and a C11 compiler (Visual Studio Build Tools,
MinGW, GCC, or Clang). CMake uses installed raylib 5.5 when available, otherwise
downloads raylib 5.5 on the first configure (internet access required).

```sh
cmake -S . -B build
cmake --build build --config Release
```

Run from the repository root so the application can read `data/sales.csv`.
On Windows with Visual Studio, run `build/Release/bi_dashboard.exe`.
With a single-configuration generator, run `build/bi_dashboard` (or
`build/bi_dashboard.exe` on Windows). `make` is an optional wrapper for the
same CMake commands.

## UI integration

`src/main.c` owns the window, `DashboardViewData`, and `DashboardFilters` state.
`src/data.h` owns the backend `DashboardStats` definition (double precision).

`src/dashboard.h` wraps it in `DashboardViewData` with 12 monthly values,
backend category records, and a category count, and exposes drawing functions;
`src/dashboard.c` renders those values and handles filters and table interaction.
`src/filters.c` supplies category filtering for the controller without changing
the backend's existing region filter or calculation functions.
The CSV is loaded once at startup. Input is processed before drawing, and backend
filtering and calculations run initially and whenever either filter changes.
No CSV processing or business calculations belong in drawing code.

## Controls

- Select a region using the left dropdown, a regional comparison row, or keys 1-5.
- Choose All categories or a product category from the matching category dropdown.
  This filter combines with the region and applies to KPIs, charts, dates,
  transaction records, and CSV exports. Category options always come from the full
  dataset, so switching regions does not remove choices from the menu.
- F1 selects All categories; F2-F9 select category options in their displayed order.
  The category dropdown shows each shortcut. F10 remains reserved for PNG snapshots.
- The accent color follows the selected category, including revenue bars/lines,
  buttons, selected states, the profitability ring, the sidebar mark, and the native
  window icon. All categories restores the neutral white theme. Category legend
  colors stay consistent across regions; the dark base and readable text stay fixed.
- Switch the monthly chart between Bars and Line. Hover to inspect exact values.
- With All regions and one selected category, Revenue mix becomes a regional
  compass: North is above, South below, East right, and West left. Petal radius
  represents revenue on the same zero-based scale as the regional bars. Each
  direction displays its exact revenue; hovering shows its order count. Other
  filter combinations keep the existing category mix view.
- Open Transactions for the full record explorer. Search by ID, date, category,
  or region. Click DATE or REVENUE to toggle sorting; use the on-screen arrows or
  Left/Right arrow keys for pagination, including the Overview's recent records.
  Page navigation stops at the first and last page.
- Search accepts typing and Ctrl+V. Delete clears the query; Escape clears it and
  leaves the search field. Searching changes the table, not the dashboard KPIs.
- Export CSV saves all records matching both filters, independent of the
  table search. Reports use unique names under exports/ and are excluded from Git.
- Enter (including numpad Enter) triggers the same CSV export. Region/category,
  pagination, and export shortcuts are disabled while typing in the search field.
  An open dropdown also blocks Enter export and table navigation.
- Save snapshot (or F10) saves a PNG to exports/. Close using the window's close button.

The initial canvas is 1440 x 960 and scales uniformly when resized. The CSV is
read only once. Margin, revenue shares, date bounds, and regional summaries are
prepared by the controller, using the backend's calculations. No growth claims
or simulated historical comparisons are displayed. Fonts and their OFL license
are bundled under assets/fonts/.

The original BIDashboard monogram is available as a scalable SVG in
assets/branding/bidashboard-mark.svg. The sidebar and native window icon use
the same Raylib geometry, so branding does not require another image library.

Regional revenue compares all four regions for the selected category. Bars start
at zero and share a ceiling 10% above the largest regional revenue. The displayed
axis updates with the category. Body and supporting text use at least 14-pixel
type at the reference canvas size, with higher contrast for secondary labels.

## Filter tests

```sh
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Tests use fixed fixtures independent of sales.csv and check all-category copying,
category matching, combined region/category selection, empty results, output
capacity, and invalid arguments. Ege's backend source and tests remain unchanged.
