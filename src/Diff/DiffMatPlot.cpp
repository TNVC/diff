#include "Tree.h"

#include "Settings.h"

#include <vector>
#include <string.h>
#include <cmath>

#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wconditionally-supported"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wundef"

#include "matplotlibcpp.h"

double calculateNode(const Settings *settings, const db::TreeNode *node, double value);

char *buildGraphics(const db::Tree *tree)
{
  namespace plt = matplotlibcpp;

  Settings settings{};
  getSettings(&settings);

  //char *name = strdup("graphics.png");

  int n = 5000;

  std::vector<double> x(n), y(n);
  for (int i = 0; i < n; ++i)
    {
      double t = 2*M_PI * i/n;

      x.at(i) = t;
      y.at(i) = calculateNode(&settings, tree->root, t);
    }

  plt::plot(x, y, "r-");

  plt::show();

  //return name;

  return nullptr;
}
