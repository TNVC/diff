#include "ConsoleArgsUtils.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "ResourceBundle.h"
#include "StringsUtils.h"
#include "SystemLike.h"
#include "ErrorHandler.h"
#include "GarbageCollector.h"
#include "StringsUtils.h"
#include "Assert.h"

#define HANDLE_IF(name, handler)                    \
  if (!strcmp(argv[i], FLAGS[name]))                \
    do                                              \
      {                                             \
        if (++i < argc && argv[i][0] != '-')        \
          {                                         \
            int error = handler(argv[i], settings); \
                                                    \
            if (error)                              \
              return error;                         \
          }                                         \
        else                                        \
          INCORRECT_ARGUMENT;                       \
      } while (0)

#define ELSE_HANDLE_IF(name, handler) else HANDLE_IF(name, handler)

#define INCORRECT_ARGUMENT                                              \
  do                                                                    \
    {                                                                   \
      handleIncorrectArgument(                                          \
                              argv[i - 1],                              \
                              i < argc ? argv[i] : "nothing"            \
                             );                                         \
                                                                        \
      return CONSOLE_INCORRECT_ARGUMENTS;                               \
    } while (0)

#define NO_INPUT_FILES                          \
  do                                            \
    {                                           \
      handleError("No input files");            \
                                                \
      return CONSOLE_NO_INPUT_FILES;            \
    } while (0)

#define HANDLE_FILE_NAME(name, type)                               \
  do                                                               \
    {                                                              \
      if (!settings->type)                                         \
        {                                                          \
          settings->type = addDirectory(name);                     \
                                                                   \
          /*addElementForFree(settings->type);*/                   \
        }                                                          \
      else                                                         \
        {                                                          \
          handleWarning("Too many "#type" files [%s]", name);      \
        }                                                          \
    } while (0)

/// Consts-indexes in FLAGS array
enum Flags {
  LOAD,
  SAVE,
  VAR,
  HELP,
  LANG,
};

/// Type of indefity console flags
const char *FLAGS[] = {
  "-load",
  "-save",
  "-var",
  "-help",
  "-lang",
};

static void setDefaultSettings(Settings *settings);

/// Handle flag -in
/// @param [in] argument Argument for -in
/// @return Zero is all was Ok or NO_SUCH_FILE_FOUND if argument isn`t a file name
static int handleLoad(const char *argument, Settings *settings);

/// Handle flag -out
/// @param [in] argument Argument for -out
/// @return Error`s code
static int handleSave(const char *argument, Settings *settings);

static int handleHelp(Settings *settings);

static int handleLang(const char *argument, Settings *settings);

static int handleVar(const char *argument, Settings *settings);

/// Handle incorrect arguments for flags
/// @param [in] flag Name of flag wicth geted incorrect argument
/// @param [in] argument Geted argument
static void handleIncorrectArgument(const char *flag, const char *argument);

/// Handle unknown flags
/// @param [in] flag Name of unknown flag
static void handleUnknownFlag      (const char *flag);

#include <stdio.h>

int parseConsoleArgs(const int argc, const char * const argv[], Settings *settings)
{
  assert(argv);
  assert(argc > 0);
  assert(settings);

  setDefaultSettings(settings);
  settings->programName = argv[0];

  setSettings(settings);

  for (int i = 1; i < argc; ++i)
    {
      if (!strcmp(argv[i], FLAGS[HELP]))
        {
          handleHelp(settings);

          return CONSOLE_HELP;
        }
      ELSE_HANDLE_IF(LOAD, handleLoad);
      ELSE_HANDLE_IF(SAVE, handleSave);
      ELSE_HANDLE_IF(LANG, handleLang);
      ELSE_HANDLE_IF(VAR , handleVar );
      else if (argv[i][0] == '-')
          handleUnknownFlag(argv[i]);
      else
        {
          if (handleLoad(argv[i], settings))
            return CONSOLE_NO_SUCH_FILE_FOUND;
        }
    }

  if (!settings->source)
    handleLoad(DEFAULT_SOURCE_FILE_NAME, settings);

  if (!settings->target)
    handleSave(DEFAULT_TARGET_FILE_NAME, settings);

  return 0;
}

static void setDefaultSettings(Settings *settings)
{
  assert(settings);

  settings->programName  = nullptr;
  settings->source       = nullptr;
  settings->target       = nullptr;
  settings->saveType     = Save::TEXT;
  settings->locale       = db::Locale::EN;
}

static int handleLoad(const char *argument, Settings *settings)
{
  char *fileName = addDirectory(argument);

  if (!isFileExists(fileName))
    {
      handleError("No such file [%s]", fileName);

      free(fileName);

      return CONSOLE_NO_SUCH_FILE_FOUND;
    }

  free(fileName);

  HANDLE_FILE_NAME(argument, source);

  return 0;
}

static int handleSave(const char *argument, Settings *settings)
{
  HANDLE_FILE_NAME(argument, target);

  return 0;
}

static int handleHelp(Settings *settings)
{
  db::ResourceBundle bundle{};

  db::getBundle(&bundle, "help", settings->locale);

  const char *separator = db::getString(&bundle, "help.separator");

  printf(
         "%s%s%s%s%s%s%s%s%s%s%s", settings->programName,
         db::getString(&bundle, "help.first"),
         separator,
         db::getString(&bundle, "help.second"),
         separator,
         db::getString(&bundle, "help.third"),
         separator,
         db::getString(&bundle, "help.fourth"),
         separator,
         db::getString(&bundle, "help.fifth"),
         separator
         );

  db::destroyBundle(&bundle);

  return 0;
}

static int handleVar(const char *argument, Settings *settings)
{
  int size = 0;
  for ( ; argument[size] && !isspace(argument[size]) && argument[size] != '='; ++size)
    continue;

  if (argument[size] != '=')
    {
      handleError("Invalid argument[%s]!!", argument);

      return 1;
    }

  Variable *temp =
    (Variable *)recalloc(settings->variables, (size_t)++settings->variableCount, sizeof(Variable));

  if (!temp)
    {
      handleError("Out of memory!!");

      free(settings->variables);

      return 1;
    }

  int index = settings->variableCount - 1;

  settings->variables = temp;

  settings->variables[index].name = strndup(argument, size);

  addElementForFree(settings->variables[index].name);

  ++size;

  int offset = 0;

  sscanf(argument + size, "%lg%n", &settings->variables[index].value, &offset);

  if (offset != (int)strlen(argument) - size)//!isDigitString(argument))
    {
      handleError("Argument isn`t a number[%s]!!", argument + size);

      return CONSOLE_INCORRECT_ARGUMENTS;
    }

  return 0;
}

static int handleLang(const char *argument, Settings *settings)
{
  if (!strcmp(argument, "ru"))
    settings->locale = db::Locale::RU;
  else if (!strcmp(argument, "en"))
    settings->locale = db::Locale::EN;
  else if (!strcmp(argument, "pl"))
    settings->locale = db::Locale::PL;
  else
    {
      settings->locale = db::Locale::EN;

      handleWarning("Unknown language[%s]!!", argument);
    }

  return 0;
}

static void handleIncorrectArgument(const char *flag, const char *argument)
{
  handleError("%s expeced argument, but geted %s", flag, argument);
}

static void handleUnknownFlag(const char *flag)
{
  handleWarning("Unknow flag [%s]", flag);
}
