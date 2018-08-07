#ifndef PRVIPROLAZ_H
#define PRVIPROLAZ_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include "Util.h"
#include "SymTab.h"

class PrviProlaz
{
    public:
        PrviProlaz(int startAddr);
        virtual ~PrviProlaz();
        std::vector<std::string> readInput(std::string filename);
        void parseLine(std::string);
        void processLine(std::vector<std::string> line, int lineNum);
        void decodeLabel(std::vector<std::string> line, int lineNum);
        void decodeDirective(std::vector<std::string> line, int lineNum);
        void decodeInstruction(std::vector<std::string> line, int lineNum);


        static std::vector<std::string> instrNames;
    protected:
    private:
        static int _tOffset;
        static int _tSecOffset;
        static int _tSection;
        static bool _end;

        void print(std::string s);
};

#endif // PRVIPROLAZ_H
