#include <iostream>
#include <stdlib.h>
#include "Util.h"
#include "PrviProlaz.h"
#include "DrugiProlaz.h"
#include "SymTab.h"

using namespace std;

void info(std::string s){
    cout<<"\033[0;94m[ INFO]\033[0m " << s << "\n";
}

void printASM(){
    std::cout <<reset << magenta << "\u001b[7m Sistemski    d"  <<reset << magenta  <<  "@E" <<ltblue << "E"                        << magenta<<"   d@MEEWZ" << ltblue << "Z  "             << magenta <<"MA" << ltblue << "k" << magenta <<" MA" <<ltblue << "k\n"
              <<reset << magenta << "\u001b[7m softver     d" <<reset<< magenta    << "@MM" <<ltblue << "E"                         << magenta<<"  d@KMEWZ"<< ltblue <<"Z   "               <<magenta  << "MEA" <<ltblue<<"k" <<magenta<<"MEA" <<ltblue<<"k\n"
              <<reset << magenta << "\u001b[7m 2018       d"  <<reset<<magenta    <<"@KMM" <<ltblue << "E"                          << magenta<<"  K"<<"@V"<< ltblue <<"E       "           <<magenta << "MEEXEMEA"<<ltblue<<"k\n"
              <<reset << magenta << "\u001b[7m           d"  << reset<<magenta <<  "@W" <<ltblue <<"Z"<<magenta"MM"<<ltblue<<"E"    << magenta<<"  V@ZMMEA"<<ltblue<<"k   "                 <<magenta << "MERMMEEMA"<<ltblue<<"k\n"
              <<reset << ltblue <<  "\u001b[7m          d"  << ltblue  <<  "@W" << reset <<"Z"<<ltblue<<" EM" << reset << "E"       << ltblue<<"   V@MEEMR" << reset <<"k  "                <<ltblue <<    "MEXMEEXEMA"<<reset<<"k\n"
              <<reset << ltblue <<  "\u001b[7m         a"  << ltblue <<  "@KMAMEM" << reset << "E"                                  << ltblue <<"       X"<< ltblue <<"@M"<<reset<<"Z  "    <<ltblue << "MEE"<<reset<<"E"<<ltblue"VER"<<reset<<"i"<<ltblue"VMA"<<reset<<"k\n"
              <<reset << ltblue <<  "\u001b[7m        d"   << ltblue  << "@EW"<<reset<<"Z" <<ltblue <<"  EM" << reset << "E"        << ltblue <<"   d"<< ltblue <<"@MNEEV"<<reset<<"Z  "    <<ltblue <<  "MEE"<<reset<<"E"<<ltblue<<" MQ"<<reset<<"k"<<ltblue" VMA" <<reset<<"k\n"
              <<reset << ltblue <<  "\u001b[7m       d" << ltblue   << "@WV"<<reset<<"Z"<<ltblue<<"   EW" << reset << "E"           << ltblue <<"  Z"<< ltblue <<"@MEEWV" <<reset<<"7   "   <<ltblue << "MEE"<<reset<<"E"<<ltblue<<" YM"<<reset<<"E"<<ltblue<<"  VMA"<<reset<<"k\n"
                <<"\n" reset;
}

std::string pt(std::string color, std::string s){ // color text
    return "" + color + s + reset;
}

std::string d(std::string color, int num){ // color space
    std::string print = color + inverse;
    for(int i=0;i<num;i++){
        print+= " ";
    }
    print += reset;
    return print;
}

std::string b(int num){ // black space
    std::string print = "";
    for(int i=0;i<num;i++){
        print+= " ";
    }
    return print;
}

void printImg(){
    std::cout
        << d(magenta, 24) << d(ltblue, 3) << d(white, 5) << std::endl
        << d(green, 23) << d(ltblue, 3) << d(white, 6) << std::endl
        << d(green, 22) << d(ltblue, 3) << d(white, 7) << std::endl
        << d(green, 21) << d(ltblue, 3) << d(white, 8) << std::endl
        << d(green, 20) << d(ltblue, 3) << d(white, 9) << std::endl
        << d(green, 19) << d(ltblue, 3) << d(white, 10);
}

int main(int argc, char *argv[])
{
    printImg();
    return 0;

    printASM();

    try{
        info("Inicijalizujem Utility klasu.");
        Util::init();
        SymTab st;
        info("Pokrecem prvi prolaz.");
        int startAddr = 0;
        if(argc > 1) startAddr = atoi(argv[1]);

        PrviProlaz *pp = new PrviProlaz( startAddr );
        std::string filename = (argc>2)?argv[2]:"_input";
        std::vector<std::string> vect = pp->readInput(filename);
        info("Prvi prolaz zavrsen.");
        info("Pokrecem drugi prolaz.");
        DrugiProlaz *dp = new DrugiProlaz(startAddr, vect);
        dp->process();

        std::ofstream outfile;
        outfile.open("izlaz");
        st.print(outfile);
        for(int i=0;i<4; i++){
            Util::printRelSection(i, outfile);
            Util::printSection(i, outfile);
        }
        outfile.close();


    } catch (...){
        Util::error(-1, " GRESKA");
    }
    cout<<"\033[0;94m[ INFO]\033[0m "  << "Izlazni fajl \"izlaz\" generisan."<<reset << "\n";
    cout<<"\033[0;94m[ INFO]\033[0m " << greenblink << "*** USPESNO" << reset << "\n";
    return 0;
}
