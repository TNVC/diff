#pragma once

#include "Locale.h"
#include "Variable.h"

/// Name of default directory for files
const char * const DEFAULT_DIRECTORY = "./resources/";
/// Name of target file if didn`t input anything
const char * const DEFAULT_TARGET_FILE_NAME = "save.tex";
/// Name of source file if didn`t input anything
const char * const DEFAULT_SOURCE_FILE_NAME = "save.db";

enum class Save {
  TEXT,
  TEX,
};

struct Settings {
  char       *source;
  char       *target;
  const char *programName;
  Save   saveType;
  db::VarTable *table;
  db::Locale locale;
};

void setSettings(const Settings *settings);

void getSettings(Settings *settings);

/// Adder prefix
/// @param [in] name C-like string
/// @return Dimanic allocate C-like with DEFAULT_DIRECTORY like prefix
char *addDirectory(const char *name);
