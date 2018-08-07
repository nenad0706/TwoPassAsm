#include "SymTab.h"

SymTab::SymTab(){}
SymTab::~SymTab(){}

std::vector<Util::SymTablRecord*> SymTab::symTab;
int SymTab::rbr = 0;

void SymTab::add(std::string name, int section, int offset, bool local, int lineNum){
    if(contains(name)){
        Util::error(lineNum, "Simbol \033[0;32m" + name + "\033[0m je vec definisan.");
    }
    Util::SymTablRecord *str = new Util::SymTablRecord();
    str->name = name;
    str->section = section;
    str->offset = offset;
    str->local = local;
    str->rbr = ++rbr;

    symTab.push_back(str);
}

void SymTab::add(std::string name, int section, int offset, bool local){
    Util::SymTablRecord *str = new Util::SymTablRecord();
    str->name = name;
    str->section = section;
    str->offset = offset;
    str->local = local;
    str->rbr = ++rbr;

    symTab.push_back(str);
}

bool SymTab::contains(std::string name){
    for(Util::SymTablRecord *str : symTab){
        if(Util::eq(str->name, name)) return true;
    }
    return false;
}

Util::SymTablRecord* SymTab::get(std::string name, int lineNum){
    for(Util::SymTablRecord *str : symTab){
        if(Util::eq(name, str->name)) return str;
    }
    Util::error(lineNum, "Simbol '" + name + "' nije definisan.");
    return nullptr;
}

void SymTab::setGlobal(std::string symbol){
    if(!contains(symbol))Util::error("Tabela simbola: Simbol '" + symbol + "' nije nadjen.");
    //        static std::vector<Util::SymTablRecord*> symTab;
    for(Util::SymTablRecord* str: symTab){
        if(Util::eq(str->name, symbol)){
            str->local=false;
            return;
        }
    }
}

void SymTab::print(std::ostream& stream){
    stream<< "# Tabela simbola\n";
    if(stream == std::cout)stream << inverse;
    stream  << "#    Labela     Sek.    Offs.   L/G\n" ;
    if(stream == std::cout)stream <<reset;
    for(Util::SymTablRecord *str : symTab){

        stream << str->rbr << "    " << str->name << ((str->name.size() < 3)?"\t\t":"\t")
                << str->section << "\t" << str->offset << "\t" << str->local << "\n";
    }
}
