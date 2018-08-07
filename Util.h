#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <iomanip>

// Terminal color
#define reset "\033[0m"
#define inverse "\u001b[7m"
#define bold "\u001b[1m"


#define red "\033[0;31m"
#define redz \033[0;31m
#define green "\033[0;32m"
#define greenblink "\E[0;5;32m"
#define ltblue "\033[0;94m"
#define cyan "\u001b[36m"
#define magenta "\u001b[35m"
#define blue "\u001b[34m"
#define orange "\u001b[31;1m"
#define white ""

#define bckgreen "\u001b[42m"
#define bckltblue "\u001b[43m"
#define bckyellow "\u001b[43m"

// Sekcije
#define RODATA 0
#define DATA 1
#define TEXT 2
#define BSS 3

#define STD_RELOK 0
#define PC_RELOK 0

typedef unsigned char Byte8;
typedef unsigned short Word16;
typedef unsigned int Long32;

class Util
{
    public:

        Util();
        virtual ~Util();
        static void init();
        static bool eq(std::string s1, std::string s2);
        static std::vector<std::string> split_string(std::string s);
        static void error(int, std::string);
        static void error(std::string);

        static Word16 op1(std::vector<std::string> line, int lineNum);
        static Word16 op2(std::vector<std::string> line, int lineNum);

        static int alignBytes(int tOffset, int val);
        static int intValue(std::string, int); // uradi ove 2
        static bool isPureReg(std::string);
        static bool is4bInstruction(std::vector<std::string> line, int lineNum);
        static bool isInteger(std::string s);
        static bool isPureInteger(std::string s);
        static bool has(std::string str, std::string substr);
        static std::vector<std::string> splitIzraz(std::string s, int lineNum);

        static struct SectionRecord {
            int offset;
            int sadrzaj;
            SectionRecord(int o, int s):offset(o), sadrzaj(s){}
        } sectionRecord;

        static struct RelocRecord {
            int offset;
            int tipRelokacije;
            int rbSimbola;
            char sign;
            RelocRecord(int o, int t, int r, char c):offset(o),tipRelokacije(t), rbSimbola(r), sign(c){}
        } relocRecord;
        static struct SymTablRecord {
            int rbr;
            std::string name;
            int section;
            int offset;
            bool local;
        } symTableRecord;


        static std::vector<std::vector<SectionRecord*>> sekcije;
        static std::vector<std::vector<RelocRecord*>> relSekcije;

        static void printSection(int sekcija, std::ostream& stream);
        static void printRelSection(int sekcija, std::ostream& stream);

        static void addSectionRecord(int sekcija, int offset, int sadrzaj);
        static void addRelocRecord(int sekcija, int offset, int tipR, int rbrSimb, char sign);

        static std::string sectionName(int num);

        static std::vector<int> lineNums;
        static std::string getPomeraj(std::string op);
        static std::string getReg(std::string op);
        static std::map<std::string, Word16> opcode;

    protected:
    private:
};

#endif // UTIL_H
