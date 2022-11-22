#pragma once

#include "Tree.h"
#include <stddef.h>

namespace db {

  struct Range {
    double min;
    double max;
  };

  struct Coord {
    double x;
    double y;
  };

  struct Expression {
    const Tree *expression;
    const char *name;
  };

  struct Plot {
    Expression *expressions;
    size_t expressionsCount;
    Range xRange;
    Range yRange;
    unsigned density;
  };

  inline bool isValidPlot(const Plot *plot)
  {
    return
      (plot) &&
      (plot->xRange.min <= plot->xRange.max) &&
      (plot->yRange.min <= plot->yRange.max) &&
      (plot->density) &&
      (plot->expressionsCount ? (bool)plot->expressions : !plot->expressions);
  }

}
