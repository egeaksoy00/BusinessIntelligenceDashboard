#include <stdio.h>
#include "../src/data.h"

int main(void)
{
    Sale sale1 = {
        1,
        "2026-01-05",
        "Electronics",
        "North",
        1200.0,
        800.0
    };

    printf("Transaction ID: %d\n", sale1.id);
    printf("Date: %s\n", sale1.date);
    printf("Category: %s\n", sale1.category);
    printf("Region: %s\n", sale1.region);
    printf("Revenue: %.2f\n", sale1.revenue);
    printf("Cost: %.2f\n", sale1.cost);
    printf("Profit: %.2f\n", sale1.revenue - sale1.cost);

    return 0;
}
