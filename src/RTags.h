#ifndef RTags_h
#define RTags_h

#include <rct/String.h>
#include "Location.h"
#include <rct/Log.h>
#include <rct/Memory.h>
#include "FixIt.h"
#include <rct/Path.h>
#include "SourceInformation.h"
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <typeinfo>

class Database;
class Project;
namespace RTags {

enum DatabaseLockType {
    Read = ReadWriteLock::Read,
    Write = ReadWriteLock::Write,
    Erase
};

enum UnitType {
    CompileC,
    CompileCPlusPlus
};
enum CursorType {
    Include,
    Cursor,
    Reference,
    Other
};
}

class CursorInfo;
typedef Map<Location, CursorInfo> SymbolMap;
typedef Map<String, Set<Location> > UsrMap;
typedef Map<Location, Set<Location> > ReferenceMap;
typedef Map<String, Set<Location> > SymbolNameMap;
typedef Map<uint32_t, Set<uint32_t> > DependencyMap;
typedef Map<uint32_t, SourceInformation> SourceInformationMap;
typedef Map<Path, Set<String> > FilesMap;
typedef Map<uint32_t, Set<FixIt> > FixItMap;
typedef Map<uint32_t, List<String> > DiagnosticsMap;

namespace RTags {
void dirtySymbolNames(SymbolNameMap &map, const Set<uint32_t> &dirty);
void dirtySymbols(SymbolMap &map, const Set<uint32_t> &dirty);
void dirtyUsr(UsrMap &map, const Set<uint32_t> &dirty);

String backtrace(int maxFrames = -1);


template <typename Container, typename Value>
inline bool addTo(Container &container, const Value &value)
{
    const int oldSize = container.size();
    container += value;
    return container.size() != oldSize;
}

inline bool encodePath(Path &path)
{
    int size = path.size();
    enum { EncodedUnderscoreLength = 12 };
    for (int i=0; i<size; ++i) {
        char &ch = path[i];
        switch (ch) {
        case '/':
            ch = '_';
            break;
        case '_':
            path.replace(i, 1, "<underscore>");
            size += EncodedUnderscoreLength - 1;
            i += EncodedUnderscoreLength - 1;
            break;
        case '<':
            if (i + EncodedUnderscoreLength <= size && !strncmp(&ch + 1, "underscore>", EncodedUnderscoreLength - 1)) {
                error("Invalid folder name %s", path.constData());
                return false;
            }
            break;
        }
    }
    return true;
}

inline void decodePath(Path &path)
{
    int size = path.size();
    enum { EncodedUnderscoreLength = 12 };
    for (int i=0; i<size; ++i) {
        char &ch = path[i];
        switch (ch) {
        case '_':
            ch = '/';
            break;
        case '<':
            if (i + EncodedUnderscoreLength <= size && !strncmp(&ch + 1, "underscore>", EncodedUnderscoreLength - 1)) {
                path.replace(i, EncodedUnderscoreLength, "_");
                size -= EncodedUnderscoreLength - 1;
            }
            break;
        }
    }
}

inline int digits(int len)
{
    int ret = 1;
    while (len >= 10) {
        len /= 10;
        ++ret;
    }
    return ret;
}

String shortOptions(const option *longOptions);
int readLine(FILE *f, char *buf = 0, int max = -1);
inline int fileSize(FILE *f)
{
    assert(f);
    const int pos = ftell(f);
    fseek(f, 0, SEEK_END);
    const int ret = ftell(f);
    fseek(f, pos, SEEK_SET);
    return ret;
}
String filterPreprocessor(const Path &path);
void removeDirectory(const Path &path);
int canonicalizePath(char *path, int len);
bool startProcess(const Path &dotexe, const List<String> &dollarArgs);
void findApplicationDirPath(const char *argv0);
Path applicationDirPath();
Path findProjectRoot(const Path &path);
}

#define eintrwrap(VAR, BLOCK)                   \
    do {                                        \
        VAR = BLOCK;                            \
    } while (VAR == -1 && errno == EINTR);

#endif
