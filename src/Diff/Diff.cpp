#include "Diff.h"
#include "DiffUtils.h"

#include "Tree.h"
#include "Settings.h"
#include <stdio.h>

#include "Logging.h"

#include "StringsUtils.h"

bool init()
{
  return true;////
}

void start()
{
  Settings settings{};

  getSettings(&settings);

  db::Tree tree{};

  db::createTree(&tree);

  FILE *source = fopen(settings.source, "r");

  db::loadTree(&tree, source);

  fclose(source);

  FILE *target = stdout;//fopen(settings.target, "w");

  db::saveTree(&tree, target);

  //  fclose(target);

  dumpTree(&tree, 0, getLogFile());

  executeExpresion(&tree);

  db::destroyTree(&tree);
}
