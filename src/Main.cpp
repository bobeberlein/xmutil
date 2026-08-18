// XMUtil.cpp : Defines the entry point for the console application.
//

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#include "Model.h"
#include "OutputPath.h"
#include "Unicode.h"
#include "Vensim/VensimParse.h"
#include "XMUtil.h"

#ifdef WITH_UI
#include <QApplication>

#include "UI/Main_Window.h"
#endif

static const char *argv0;

std::string ReadStream(std::istream &input, int &error) {
  std::string content{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
  // badbit is the only failure the iterator range can leave behind (eofbit is
  // the normal end). Reporting it is what makes the caller's error branch
  // reachable at all -- without this it never fired, so a truncated read looked
  // exactly like an empty model.
  if (input.bad())
    error = EIO;
  return content;
}

// Content-sniff a buffer for XMILE: an XMILE document is XML, so after an
// optional UTF-8 BOM and leading whitespace its first byte is '<'. A Vensim
// .mdl starts with the "{UTF-8}" marker or bare equation text, and Dynamo with
// a note line -- none of which begin with '<'. Used for --stdio, where there is
// no filename extension to dispatch on; a false negative would silently route
// XMILE through the lenient Vensim parser and yield garbage with exit 0.
static bool contentLooksLikeXmile(const std::string &contents) {
  size_t i = 0;
  const size_t n = contents.size();
  if (n >= 3 && static_cast<unsigned char>(contents[0]) == 0xEF && static_cast<unsigned char>(contents[1]) == 0xBB &&
      static_cast<unsigned char>(contents[2]) == 0xBF)
    i = 3;
  while (i < n && std::isspace(static_cast<unsigned char>(contents[i])))
    ++i;
  return i < n && contents[i] == '<';
}

// True when path's final extension (the text after its last '.') equals ext
// case-insensitively; ext is passed without the dot.
static bool extensionMatchesInsensitive(const char *path, const char *ext) {
  const char *dot = path ? strrchr(path, '.') : nullptr;
  return dot && StringMatch(dot + 1, ext);
}

void cliUsage() {
  log("Usage: %s [OPTION...] PATH\n"
      "Convert Vensim .mdl, Dynamo .dyn, or XMILE .xmile/.stmx files to XMILE or .mdl.\n\n"
      "Options:\n"
      "  --help:\tshow this message\n"
      "  --stdio:\tread from stdin, write to stdout (assumes Vensim input)\n"
      "  --to-mdl:\twrite Vensim MDL instead of XMILE\n",
      argv0);

  exit(EXIT_FAILURE);
}

int cliMain(int argc, char *argv[], Model *m) {
  int ret = 0;
  const char *path = nullptr;
  bool useStdio = false;
  bool wantComplete = false;
  int longNames = -1;
  bool sectors = false;
  bool toMdl = false;

  for (argv0 = argv[0], argv++, argc--; argc > 0; argv++, argc--) {
    char const *arg = argv[0];
    if (strcmp("--help", arg) == 0) {
      cliUsage();
    } else if (strcmp("--stdio", arg) == 0) {
      useStdio = true;
    } else if (strcmp("--want-complete", arg) == 0) {
      wantComplete = true;
    } else if (strcmp("--longnames", arg) == 0) {
      longNames = 1;
    } else if (strcmp("--shortnames", arg) == 0) {
      longNames = 0;
    } else if (strcmp("--sectors", arg) == 0) {
      sectors = true;
    } else if (strcmp("--to-mdl", arg) == 0) {
      toMdl = true;
    } else if (arg[0] == '-') {
      log("unknown arg '%s'\n", arg);
      cliUsage();
    } else {
      if (!path) {
        path = arg;
      } else {
        log("specify a single path to a model\n");
        cliUsage();
      }
    }
  }

  if (useStdio) {
    path = "STDIN";
  } else if (!useStdio && path == nullptr) {
    log("ERROR: specify a path to a model or use --stdio\n");
    cliUsage();
  }

  std::ifstream fileInput;
  if (!useStdio) {
    fileInput = std::ifstream{path, std::ios::in | std::ios::binary};
    if (!fileInput.is_open()) {
      log("couldn't open file \"%s\" for reading\n", path);
      // `false` here converted to 0 -- EXIT_SUCCESS -- so a mistyped path was
      // reported on stderr and then announced as a successful conversion, which
      // any `set -e` script driving the binary would walk straight past.
      return 1;
    }
  }
  int err = 0;
  auto contents = ReadStream(useStdio ? std::cin : fileInput, err);
  if (err) {
    log("ReadStream(): %d (%s)\n", err, strerror(err));
    return 1;
  }

  char *output;
  // File inputs dispatch on extension; --stdio has no extension (path is the
  // "STDIN" placeholder), so sniff the buffer's first non-whitespace byte
  // instead. Dynamo via --stdio is not sniffable ('<'-vs-not only separates
  // XMILE from the Vensim/Dynamo text grammars); it keeps routing through the
  // non-XMILE branch, matching the documented "--stdio assumes Vensim input"
  // default for non-XML content.
  const bool xmileInput =
      useStdio ? contentLooksLikeXmile(contents)
               : (extensionMatchesInsensitive(path, "xmile") || extensionMatchesInsensitive(path, "stmx"));
  if (xmileInput) {
    if (toMdl) {
      output = convert_xmile_to_mdl(contents.c_str(), contents.size(), path, longNames);
    } else {
      output = convert_xmile_to_xmile(contents.c_str(), contents.size(), path, longNames, sectors);
    }
  } else {
    if (toMdl) {
      output = convert_to_mdl(contents.c_str(), contents.size(), path, longNames);
    } else {
      output = convert_mdl_to_xmile(contents.c_str(), contents.size(), path, false, longNames, sectors);
    }
  }
  if (output == nullptr) {
    // The convert_* entry points already emitted the specific reason to stderr;
    // this adds a final summary line so a failure is never silent even if the
    // conversion produced no other diagnostic.
    log("error trying to convert the model\n");
    return 1;
  }

  std::ofstream fileOutput;
  if (!useStdio) {
    // A Vensim->Vensim or XMILE->XMILE conversion derives an output name equal
    // to its input's; DeriveOutputPath interposes ".regen" in that case so the
    // model being read is not truncated out from under the reader.
    const std::string p = DeriveOutputPath(path, toMdl ? "mdl" : "xmile");
    fileOutput = std::ofstream{p, std::ofstream::out | std::ios::binary | std::ios::trunc};
    if (!fileOutput.is_open()) {
      log("ERROR: couldn't open '%s' for writing.\n", p.c_str());
      exit(EXIT_FAILURE);
    }
  }

  (useStdio ? std::cout : fileOutput) << output;
  (useStdio ? std::cout : fileOutput).flush();

  // convert_to_mdl/convert_mdl_to_xmile hand back a strdup'd heap buffer (C
  // allocation), so it must be released with free, not delete. output is known
  // non-null here (the nullptr case returned above); the only paths that skip
  // this free are the exit(EXIT_FAILURE) branches, where the process is already
  // terminating and the OS reclaims the allocation.
  free(output);

  return ret;
}

#ifdef _DEBUG
void CheckMemoryTrack(int clear);
#endif

int main(int argc, char *argv[]) {
  if (!OpenUnicode()) {
    return -1;
  }

  int ret = 0;
  Model *m = new Model();
#ifndef WITH_UI
  ret = cliMain(argc, argv, m);
#else
  QApplication app(argc, argv);
  // QApplication::setWindowIcon(QIcon(":icons/icon.svg"));
  QApplication::setOrganizationName("XMUtil");
  QApplication::setOrganizationDomain("github.com/xmutil");
  QApplication::setApplicationName("MDL to XMILE");

  Main_Window window;
  window.show();

  ret = app.exec();
#endif
  delete m;
  CloseUnicode();

  // CheckMemoryTrack(1) ;

  // log("Size of symbol is %d\n",sizeof(Symbol)) ;
  // log("Size of variable is %d\n",sizeof(Variable)) ;
  // _CrtDumpMemoryLeaks() ;

  // if want to look at terminal

  return ret;
}

#if defined(_DEBUG) && defined(wantownmemorytesting)
#include <assert.h>

#include <unordered_map>
#undef new     // regular new used in this section
#undef delete  // same for delete

typedef struct {
  size_t size;
  int line_no;
  char file[32];
} AllocInfo;

typedef std::unordered_map<void *, AllocInfo> MemTrackMap;

MemTrackMap *AllocList = 0;

void AddTrack(void *addr, size_t size, const char *fname, int lnum) {
  if (!AllocList)
    AllocList = new MemTrackMap();
  AllocInfo ai;
  ai.size = size;
  ai.line_no = lnum;
  if (strlen(fname) > 31)
    strcpy(ai.file, fname + strlen(fname) - 31);
  else
    strcpy(ai.file, fname);
  (*AllocList)[addr] = ai;
};

static int Uk = 0;
void RemoveTrack(void *addr) {
  if (AllocList) {
    MemTrackMap::iterator node = AllocList->find(addr);
    if (node != AllocList->end()) {
      AllocList->erase(node);
      return;
    }
  }
  // log("%x %d\n",addr,++Uk) ;
  // ignore things that may have been allocated elsewhere - boost is not controllable
}

void CheckMemoryTrack(int clear) {
  if (!AllocList)
    return;
  MemTrackMap::iterator node = AllocList->begin();
  for (; node != AllocList->end(); node++) {
    log("Uncleared Memory at %u size %d from %s(%d)\n", node->first, node->second.size, node->second.file,
        node->second.line_no);
  }
  if (clear) {
    MemTrackMap *a = AllocList;
    AllocList = NULL;
    delete a;
  }
}
#endif
