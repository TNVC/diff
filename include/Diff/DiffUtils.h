#pragma once

#include "Tree.h"
#include "Variable.h"
#include "Coordinate.h"
#include <stdio.h>

db::Tree diffExpresion(const db::Tree *tree, FILE *file = stdout, int *error = nullptr);

void executeExpresion(const db::Tree *tree, int *error = nullptr);

double calculateNode(const db::VarTable *table, const db::TreeNode *node);

char *buildGraphics(const db::Plot *plot, int *error = nullptr);

db::Tree calculateTanget(const db::VarTable *table, const db::Tree *tree, int *error = nullptr);

db::Tree calculateSeries(const db::VarTable *table, const db::Tree *tree, int power, int *error = nullptr);

