# Business Intelligence Dashboard

A desktop Business Intelligence Dashboard prototype developed in **C** using the **raylib** graphics library.

## Project Overview

This project is being developed as part of the Advanced Programming AI Project.

The application will visualize business data through an interactive dashboard, allowing users to monitor key performance indicators (KPIs), explore charts, and analyse sales performance.

## Planned Features

- Dashboard interface built with raylib
- CSV data loading
- Revenue, Profit and Orders KPIs
- Monthly sales charts
- Category and region analysis
- Interactive filtering
- Clean and responsive desktop UI

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

The first UI milestone is implemented with mock data: four KPI cards, monthly
and category bar charts, and selectable region buttons. Region selection only
changes the highlighted button; backend filtering will be connected later.
The mock chart values are independent examples and do not reconcile to the KPIs.

## Build and run

Requires CMake 3.16+, Git, and a C99 compiler (Visual Studio Build Tools,
MinGW, GCC, or Clang). CMake uses installed raylib 5.5 when available, otherwise
downloads raylib 5.5 on the first configure (internet access required).

```sh
cmake -S . -B build
cmake --build build --config Release
```

On Windows with Visual Studio, run `build/Release/bi_dashboard.exe`.
With a single-configuration generator, run `build/bi_dashboard` (or
`build/bi_dashboard.exe` on Windows). `make` is an optional wrapper for the
same CMake commands.

## UI integration

`src/main.c` owns the window, mock `DashboardViewData`, and selected `RegionFilter`.
`src/data.h` owns the backend `DashboardStats` definition (double precision).

`src/dashboard.h` wraps it in `DashboardViewData` with 12 monthly values and separate category chart values, and exposes drawing functions;
`src/dashboard.c` only renders those values and handles region button clicks.
Replace the mock structure with backend-prepared statistics and use the selected
region when requesting new statistics. No CSV processing belongs in drawing code.
