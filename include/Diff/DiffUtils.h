#pragma once

#include "Tree.h"

db::Tree *diffExpresion(const db::Tree *tree, int *error = nullptr);

void executeExpresion(const db::Tree *tree, int *error = nullptr);
