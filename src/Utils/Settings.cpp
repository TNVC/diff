#include "Settings.h"

#include <stdlib.h>
#include <string.h>
#include "Assert.h"

static Settings GlobalSettings = {};

void setSettings(const Settings *settings)
{
  assert(settings);

  GlobalSettings = *settings;
}

void getSettings(Settings *settings)
{
  assert(settings);

  *settings = GlobalSettings;
}

char *addDirectory(const char *name)
{
  char *newString = (char *)calloc(strlen(DEFAULT_DIRECTORY) + strlen(name) + 1, sizeof(char));

  strcat(newString, DEFAULT_DIRECTORY);

  strcat(newString, name);

  return newString;
}
