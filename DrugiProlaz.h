#ifndef DRUGIPROLAZ_H
#define DRUGIPROLAZ_H

#include "Util.h"
#include "SymTab.h"

class DrugiProlaz
{
    public:
        DrugiProlaz(int startAddr, std::vector<std::string>);
        virtual ~DrugiProlaz();

        void process();
        void processInstruction(std::vector<std::string>, int);
        void processDirective(std::vector<std::string>, int);
        void processLabel(std::vector<std::string>, int);
        void processLine(std::vector<std::string> line, int lineNum);

        Word16 op1(std::string, int);
        Word16 op2(std::string, int);

        std::string getAddrMode(std::string s);
        std::string getInstr(std::string s);
        int extractChar(std::string s, int lineNum);

        void validateInstruction(std::vector<std::string> line, int lineNum);
        void incOffset(int val);
        void checkSection(int lineNum);
        void changeSection(int);

        void writeSection(Byte8 code);
        void writeSection(Word16 code);
        void writeSectionLE(Word16 code);
        void writeSectionLE(Long32 code);

        Word16 reloc(std::string op, int toffset, int lineNum);

    protected:
    private:
        std::vector<std::string> _input;
        bool _end;
        static int _tSekcija;
        static int _tOffset;
        static int _tOffsetUSekciji;
        void print(std::string s);
};

#endif // DRUGIPROLAZ_H
