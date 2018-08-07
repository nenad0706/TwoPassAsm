#ifndef SYMTAB_H
#define SYMTAB_H

#include <vector>
#include "Util.h"
#include <fstream>

class SymTab
{
    public:
        SymTab();
        virtual ~SymTab();

        static std::vector<Util::SymTablRecord*> symTab;

        static int rbr;
        static void add(std::string, int, int, bool);
        static void add(std::string, int, int, bool, int);
        static bool contains(std::string);
        static Util::SymTablRecord *get(std::string name, int lineNum);
        static void setGlobal(std::string symbol);
        static void print(std::ostream&);

    protected:
    private:
};

#endif // SYMTAB_H
