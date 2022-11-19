#include "Diff.h"
#include "DiffUtils.h"

#include "Tree.h"
#include "Settings.h"
#include <stdio.h>
#include <string.h>

#include "Logging.h"
#include "ErrorHandler.h"
#include "Assert.h"
#include "StringsUtils.h"
#include "SystemLike.h"
#include "ColorOutput.h"

#include "ResourceBundle.h"

enum {
  READ,
  CHANGE,
  EXECUTE,
  DIFF,
  QUIT,
  SHOW,
  CHANGE_SAVE,
};

const int MAX_ANSWER_SIZE = 30;

const int MAX_FILE_NAME = 256;

static db::ResourceBundle Bundle{};

static int menu(Settings *settings, bool needSave, bool wasRead);

static bool checkAnswer  (const char *answer, int answerSize, bool needSave, bool wasRead);
static int  convertAnswer(const char *answer, int answerSize);

static FILE *changeFile(Settings *settings);

bool init()
{
  db::getBundle(&Bundle, "messages");

  return true;
}

db::ResourceBundle *getBundle()
{
  return &Bundle;
}

void start()
{
  Settings settings{};

  getSettings(&settings);

  db::Tree tree{};
  db::Tree diffTree{};

  db::createTree(&tree);
  db::createTree(&diffTree);

  FILE *source = fopen(settings.source, "r");

  FILE *target = fopen(".temp/temp_tex.tex", "w");

  bool needSave   = true;
  bool wasRead    = false;
  bool wasDiff    = false;

  char *buildGraphics(const db::Tree *tree);

  while (true)
    {
      switch (menu(&settings, needSave, wasRead))
        {
        case READ:
          {
            wasRead = true;

            if (tree.root)
              db::removeNode(tree.root);
            rewind(source);
            db::loadTree(&tree, source);
            if (!tree.root)
              {
                printf(ITALIC FG_RED "%s\n" RESET, db::getString(&Bundle, "input.empty"));

                wasRead = false;
              }
            break;
          }
        case CHANGE:
          {
            wasRead = wasDiff = false;

            FILE *file = changeFile(&settings);

            if (file)
              source = file;

            break;
          }
        case EXECUTE:
          {
            if (!tree.root) continue;
            db::saveTree(&tree, stdout);
            fflush(0);
            executeExpresion(&tree);
            buildGraphics(&tree);
            break;
          }
        case DIFF:
          {
            if (!tree.root) continue;
            wasDiff = true;
            if (diffTree.root)
              db::removeNode(diffTree.root);
            diffTree = diffExpresion(&tree, target);
            db::saveTree(&diffTree, stdout);
            executeExpresion(&diffTree);
            break;
          }
        case SHOW:
          {
            if (!tree.root) continue;
            if (!wasRead) continue;
            settings.saveType = Save::TEX;
            setSettings(&settings);

            rewind(target);
            db::saveTree(&tree, target);

            if (wasDiff) db::saveTree(&diffTree, target);

            settings.saveType = Save::TEXT;
            setSettings(&settings);

            fprintf(target, " \\end");
            fflush(target);
            system("pdftex .temp/temp_tex.tex > .temp/output");
            system("open temp_tex.pdf");
            break;
          }
        case QUIT:
          printf("Goodbye.\n"); goto exit;
        case CHANGE_SAVE:
          needSave = !needSave; break;
        default:
          printf("ERROR!!\nGodbye.\n"); return;
        }
    }

 exit:

  setSettings(&settings);

  fclose(source);

  fclose(target);

  dumpTree(&diffTree, 0, fopen(".log/temp.html", "w"));

  db::destroyTree(&tree);

  db::destroyTree(&diffTree);

  system("rm -f .temp/output temp_tex.pdf temp_tex.log");
}

static int menu(Settings *settings, bool needSave, bool wasRead)
{
  printf("%s\n", db::getString(&Bundle, "separator"));

  printf(ITALIC "%s\n" RESET, needSave ?
         db::getString(&Bundle, "menu.save") :
         db::getString(&Bundle, "menu.unsave"));

  printf("%s\"" FG_BRIGHT_GREEN  "%s" RESET "\"\n",
         db::getString(&Bundle, "menu.file"), settings->source);

  printf("%s", getString(&Bundle, "menu.choice"));

  if (wasRead)
    printf("%s", db::getString(&Bundle, "menu.read"));
  printf("%s\n",
         needSave ?
         db::getString(&Bundle, "menu.choiceUnsave") :
         db::getString(&Bundle, "menu.choiceSave"));

  printf("%s", db::getString(&Bundle, "menu.input"));

  char buffer[MAX_ANSWER_SIZE] = "";
  int offset     = 0;
  int answerSize = 0;

  while (
         scanf(" %n%16[^ \t\n]%n", &offset, buffer, &answerSize) != 1 ||
         !checkAnswer(buffer, answerSize - offset, needSave, wasRead)
        )
    {
      while (getchar() != '\n') continue;

      printf("%s", db::getString(&Bundle, "input.incorrect"));
    }

  while (getchar() != '\n') continue;

  return convertAnswer(buffer, answerSize - offset);
}

static bool checkAnswer(const char *answer, int answerSize, bool needSave, bool wasRead)
{
  assert(answer);

  return
    (             !strincmp(db::getString(&Bundle, "answer.read"   ), answer, answerSize)) ||
    (             !strincmp(db::getString(&Bundle, "answer.change" ), answer, answerSize)) ||
    (wasRead &&   !strincmp(db::getString(&Bundle, "answer.execute"), answer, answerSize)) ||
    (wasRead &&   !strincmp(db::getString(&Bundle, "answer.diff"   ), answer, answerSize)) ||
    (wasRead &&   !strincmp(db::getString(&Bundle, "answer.show"   ), answer, answerSize)) ||
    (             !strincmp(db::getString(&Bundle, "answer.quit"   ), answer, answerSize)) ||
    (!needSave && !strincmp(db::getString(&Bundle, "answer.save"   ), answer, answerSize)) ||
    ( needSave && !strincmp(db::getString(&Bundle, "answer.unsave" ), answer, answerSize));
}

static int  convertAnswer(const char *answer, int answerSize)
{
  assert(answer);

  if (!strincmp(db::getString(&Bundle, "answer.read"   ), answer, answerSize))
    return READ;
  if (!strincmp(db::getString(&Bundle, "answer.change" ), answer, answerSize))
    return CHANGE;
  if (!strincmp(db::getString(&Bundle, "answer.execute"), answer, answerSize))
    return EXECUTE;
  if (!strincmp(db::getString(&Bundle, "answer.diff"   ), answer, answerSize))
    return DIFF;
  if (!strincmp(db::getString(&Bundle, "answer.show"   ), answer, answerSize))
    return SHOW;
  if (!strincmp(db::getString(&Bundle, "answer.quit"   ), answer, answerSize))
    return QUIT;
  if (!strincmp(db::getString(&Bundle, "answer.save"   ), answer, answerSize))
    return CHANGE_SAVE;
  if (!strincmp(db::getString(&Bundle, "answer.unsave" ), answer, answerSize))
    return CHANGE_SAVE;

  return QUIT;
}

static FILE *changeFile(Settings *settings)
{
  assert(settings);

  printf("%s", db::getString(&Bundle, "input.file"));

  char buffer[MAX_FILE_NAME] = "";
  int offset = 0;
  int answerSize = 0;

  if (scanf(" %n%100[^\n]%n", &offset, buffer, &answerSize) != 1)
    {
      handleError("%s", db::getString(&Bundle, "error.failRead"));

      while (getchar() != '\n') continue;

      return nullptr;
    }

  while (getchar() != '\n') continue;

  const char *consoleName = db::getString(&Bundle, "file.stdin");

  if (!strincmp(consoleName, buffer, (int)strlen(consoleName)))
    {
      free(settings->source);
      settings->source = strdup(consoleName);
      return stdin;
    }

  char *fileName = addDirectory(buffer);

  if (!isFileExists(fileName))
    {
      printf("%s[%s]\n", db::getString(&Bundle, "error.notSuchFile"), fileName);

      free(fileName);

      return nullptr;
    }

  FILE *file = fopen(fileName, "r");

  free(settings->source);
  settings->source = fileName;

  return file;
}
