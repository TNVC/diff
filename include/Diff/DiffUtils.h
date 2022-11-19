#pragma once

#include "Tree.h"
#include <stdio.h>

db::Tree diffExpresion(const db::Tree *tree, FILE *file = stdout, int *error = nullptr);

void executeExpresion(const db::Tree *tree, double value = NAN, int *error = nullptr);
