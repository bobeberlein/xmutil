// XMUtil.cpp : Defines the entry point for the console application.
//

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "Model.h"
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

    return content;
}

void cliUsage() {
    log("Usage: %s [OPTION...] PATH\n"
        "Convert Vensim MDL files or Dynamoc DYN files to XMILE.\n\n"
        "Options:\n"
        "  --help:\tshow this message\n"
        "  --stdio:\tread from stdin, write to stdout\n",
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
        // couldn't open file, exit
        if (!fileInput.is_open()) {
            log("couldn't open file \"%s\" for reading\n", path);
            return false;
        }
    }
    int err = 0;
    auto contents = ReadStream(useStdio ? std::cin : fileInput, err);
    if (err) {
        log("ReadStream(): %d (%s)\n", err, strerror(err));
        return false;
    }

    auto xmile = convert_mdl_to_xmile(contents.c_str(), contents.size(), path, false, longNames, sectors);
    if (xmile == nullptr) {
        log("error trying to convert the mdl to xmile\n");
        return 1;
    }

    std::ofstream fileOutput;
    if (!useStdio) {
#ifdef __APPLE__
        std::string p(path);
        for (std::string::iterator it = p.end();it-- > p.begin();)
        {
            if (*it == '.')
            {
                p = p.substr(0,it-p.begin());
                break;
            }
            else if (*it == '/' || *it == '\\' || *it == ':')
                break;
        }
        p += ".xmile";
        fileOutput = std::ofstream{p, std::ofstream::out | std::ios::binary | std::ios::trunc};
        if (!fileOutput.is_open()) {
            log("ERROR: couldn't open '%s' for writing.\n", p.c_str());
            exit(EXIT_FAILURE);
        }
#else
        std::filesystem::path p(path);
        p.replace_extension(".xmile");
        fileOutput = std::ofstream{p.string(), std::ofstream::out | std::ios::binary | std::ios::trunc};
        if (!fileOutput.is_open()) {
            log("ERROR: couldn't open '%s' for writing.\n", p.string().c_str());
            exit(EXIT_FAILURE);
        }
#endif
    }

    (useStdio ? std::cout : fileOutput) << xmile;
    (useStdio ? std::cout : fileOutput).flush();

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
        log("Uncleared Memory at %u size %d from %s(%d)\n", node->first, node->second.size,
                node->second.file, node->second.line_no);
    }
    if (clear) {
        MemTrackMap *a = AllocList;
        AllocList = NULL;
        delete a;
    }
}
#endif
