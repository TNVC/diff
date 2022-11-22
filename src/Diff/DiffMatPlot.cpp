#include <matplot/matplot.h>

#include "Coordinate.h"
#include "Variable.h"
#include "Settings.h"
#include "ErrorHandler.h"

#include <vector>
#include <string.h>
#include <cmath>
#include <array>

#include "DiffUtils.h"
#include "GenerateName.h"
#include "Assert.h"
#include "Error.h"

char *buildGraphics(const db::Plot *plot, int *error)
{
  if (!db::isValidPlot(plot))
    ERROR(nullptr);

  char *name = generateName(".temp/image", "png", "_", ".");

  if (!name)
    ERROR(nullptr);

  Settings settings{};
  getSettings(&settings);

  db::VarTable *table = settings.table;

  double *mainValue = db::searchMainVariable(table);

  if (!mainValue)
    {
      handleError("No main variable!!");

      ERROR(nullptr);
    }

  double originMainValue = *mainValue;

  namespace plt = matplot;

  std::array<double, 2> xRange{plot->xRange.min, plot->xRange.max};
  std::array<double, 2> yRange{plot->yRange.min, plot->yRange.max};

  plt::ylim(xRange);
  plt::ylim(yRange);

  std::vector<double> x(plot->density);

  double range = plot->xRange.max - plot->xRange.min;
  for (size_t i = 0; i < plot->density; ++i)
    x.at(i) = plot->xRange.min + range * (double)i/plot->density;

  std::vector<std::vector<double>>
    y(plot->expressionsCount, std::vector<double>(plot->density));

  for (size_t i = 0; i < plot->expressionsCount; ++i)
    y[i] = plt::transform(x,
                          [main=mainValue, &table, &plot, i](auto val)
                          {
                            *main = (double)val;

                            return calculateNode(table, plot->expressions[i].expression->root);
                          });

  plt::plot(y);

  std::vector<std::string> legends{};

  for (size_t i = 0; i < plot->expressionsCount; ++i)
    legends.push_back(plot->expressions[i].name);

  plt::legend(legends);

  plt::save(name);

  *mainValue = originMainValue;

  return name;
}
