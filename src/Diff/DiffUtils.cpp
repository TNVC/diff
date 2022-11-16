#include "DiffUtils.h"

#include "SystemLike.h"
#include "Assert.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define ERROR(...)                              \
  do                                            \
    {                                           \
      if (error)                                \
        *error = -1;                            \
                                                \
      return __VA_ARGS__;                       \
    } while (0)

const char *const OPERATOR_CMDS[] =
  {
    "ADD",
    "SUB",
    "MUL",
    "DIV"
  };

const char *const TEMP_DIRECTORY   = ".temp/";
const char *const TEMP_FILE_PREFIX = "temp";
const char *const TEMP_FILE_SUFFIX = "asm";

const int MAX_CMD_SIZE = 256;

const int ACCURACY = 10000;

static char *getTempFileName();

static void printNode(const db::TreeNode *node, FILE *file);

db::Tree *diffExpresion(const db::Tree *tree, int *error);

void executeExpresion(const db::Tree *tree, int *error)
{
  assert(tree);

  char *tempName = getTempFileName();

  if (!tempName)
    ERROR();

  FILE *tempFile = fopen(tempName, "w");

  if (!tempFile)
    {
      free(tempName);

      ERROR();
    }

  fprintf(tempFile, "PUSH 1\nPOP rex\n");

  if (tree->root)
    {
      printNode(tree->root, tempFile);

      fprintf(tempFile, "OUT\n");
    }

  fflush(tempFile);

  char buffer[MAX_CMD_SIZE] = "";

  sprintf(buffer, "../assembler/assembler/assembler -in %s -out %s.bin", tempName, tempName);

  system(buffer);

  sprintf(buffer, "../assembler/softCPU/softcpu -in %s.bin", tempName);

  system(buffer);

  free(tempName);
}

static void printNode(const db::TreeNode *node, FILE *file)
{
  assert(node);
  assert(file);

  if (node->left)
    printNode(node->left, file);

  if (node->right)
    printNode(node->right, file);

  switch (node->type)
    {
    case db::type_t::OPERATOR:
      fprintf(file, "\t%s\n", OPERATOR_CMDS[(int)node->value.operat]);
      break;
    case db::type_t::VARIABLE:
      fprintf(file, "\tPUSH rax\n\tPUSH rbx\n");///////////////////////
      break;
    case db::type_t::NUMBER:
      {
        double value = node->value.number;
        fprintf(file, "\tPUSH %d\n\tPUSH %d\n",
                (int)(value - (int)value)*ACCURACY, (int)value);
        break;
      }
    default:
      fprintf(file, "ERROR!!\n");
      break;
    }
}

static char *getTempFileName()
{
  time_t now = 0;
  time(&now);
  char *dataString = ctime(&now);

  for (int i = 0; dataString[i]; ++i)
    if (isspace(dataString[i]) || ispunct(dataString[i]))
      dataString[i] = '_';

  size_t size =
    sizeof(TEMP_DIRECTORY)   +
    sizeof(TEMP_FILE_PREFIX) +
    sizeof(TEMP_FILE_SUFFIX) +
    strlen(dataString)       + 3;

  char *newTempFileName = (char *) calloc(size, sizeof(char));

  if (!isPointerCorrect(newTempFileName))
    return strdup(".temp/temp_defaultTempFile.asm");

  strcat (newTempFileName, TEMP_DIRECTORY);
  strcat (newTempFileName, TEMP_FILE_PREFIX);
  strcat (newTempFileName, "_");
  strncat(newTempFileName, dataString, strlen(dataString) - 1);
  strcat (newTempFileName, ".");
  strcat (newTempFileName, TEMP_FILE_SUFFIX);

  return newTempFileName;
}
