#include "PrviProlaz.h"

int PrviProlaz::_tOffset;
int PrviProlaz::_tSecOffset = 0;
int PrviProlaz::_tSection = -1;
bool PrviProlaz::_end = false;

PrviProlaz::PrviProlaz(int startAddr){
    _tOffset = startAddr;
    std::cout << "[ PrviProlaz]  START\n";
}

std::vector<std::string> PrviProlaz::instrNames = {
                    "addal", "addne", "addgt", "addeq",
                    "subal", "subne", "subgt", "subeq",
                    "mulal", "mulne", "mulgt", "muleq",
                    "dival", "divne", "divgt", "diveq",
                    "cmpal", "cmpne", "cmpgt", "cmpeq",
                    "andal", "andne", "andgt", "andeq",
                    "oral",  "orne",  "orgt",  "oreq",
                    "notal", "notne", "notgt", "noteq",
                    "testal", "testne", "testgt", "testeq",
                    "pushal", "pushne", "pushgt", "pusheq",
                    "popal",  "popne",  "popgt",  "popeq",
                    "callal", "callne", "callgt", "calleq",
                    "iretal", "iretne", "iretgt", "ireteq",
                    "moval", "movne", "movgt", "moveq",
                    "shlal", "shlne", "shlgt", "shleq",
                    "shral", "shrne", "shrgt", "shreq",
                    "jmpal", "jmpne", "jmpgt", "jmpeq",
                    "retal", "retne", "retgt", "reteq"};

std::vector<std::string> PrviProlaz::readInput(std::string fileName){
    std::vector<std::string> vect;

    std::ifstream file(fileName);
	std::string s;

    print("Citam ulazni fajl \"" + fileName + "\"...");

    int tnum = 0;
	while (std::getline(file, s)) {
	    tnum++;
	    if(Util::eq(s, "\n") || Util::eq(s, "\n") || s.empty())
            continue;

	    // remove \n
	    if (!s.empty() && s[s.length()-1] == '\n') {
            s.erase(s.length()-1);
        }
        // remove \t
        int pos;
        while((pos = s.find('\t')) != std::string::npos){
            s.replace(pos, 1, " ");
        }
        // remove extra whitespace
        while((pos = s.find("  ")) != std::string::npos){
            s.replace(pos, 2, " ");
        }
        // remove lead whitespace
        if(s[0] == ' '){
            s.erase(0, 1);
        }
        Util::lineNums.push_back(tnum);
		vect.push_back(s);
	}
	if(tnum == 0) Util::error("Ulazni fajl ne postoji ili je prazan. Proverite naziv fajla.");

	// vect { linije ulaznog fajla }
	std::vector<std::string> line;
	int b=0;
    for(std::string ln : vect){
        if(!_end){
            line = Util::split_string(ln);
            processLine(line, Util::lineNums[b]);
            b++;
        }
    }
    print("Fajl procitan.");
    print("Popunjavam tabelu simbola...");
    return vect;
}

void PrviProlaz::processLine(std::vector<std::string> line, int lineNum){
    if(line[0].find(':')!= std::string::npos){ decodeLabel(line, lineNum); }
    else if (line[0].at(0) == '.') { decodeDirective(line, lineNum); }
    else { decodeInstruction(line, lineNum); }
}

void PrviProlaz::decodeLabel(std::vector<std::string> line, int lineNum){
    std::string label = line[0].substr(0, line[0].size()-1);
    // errcheck
    if(_tSection == -1) Util::error(lineNum, "Labela definisana van sekcije.");
    if(SymTab::contains(label)) Util::error(lineNum, "Labela \033[0;31m" + label + "\033[0m je vec definisana");
    // symtab insert
    SymTab::add(label, _tSection, _tSecOffset, true);
    if(line.size() > 1){
        std::vector<std::string> afterLabel = line;
        afterLabel.erase(afterLabel.begin());
        processLine(afterLabel, lineNum);
    }

}

void PrviProlaz::decodeDirective(std::vector<std::string> line, int lineNum){
    std::string dir = line[0];
    if(Util::eq(dir, ".global")){
        return;
    } else if (Util::eq(dir,".extern")) {
        for(unsigned int i=1; i< line.size(); i++){
            SymTab::add(line[i], -1, 0, false);
        }

    } else if (Util::eq(dir, ".rodata")){
        _tSection = 0;
        SymTab::add(dir, _tSection, _tOffset, true);
        _tSecOffset = 0;
    } else if (Util::eq(dir, ".data")){
        _tSection = 1;
        SymTab::add(dir, _tSection, _tOffset, true);
        _tSecOffset = 0;
    } else if (Util::eq(dir, ".text")){
        _tSection = 2;
        SymTab::add(dir, _tSection, _tOffset, true);
        _tSecOffset = 0;
    } else if (Util::eq(dir, ".bss")){
        _tSection = 3;
        SymTab::add(dir, _tSection, _tOffset, true);
        _tSecOffset = 0;
    } else if (Util::eq(dir, ".end")){
        _end = true;
    } else {
        //else toffset, tsecoffset +=sizezl
        // .char, .word, .long, .skip, .align
        if(Util::eq(dir, ".char")) {
            _tSecOffset += line.size()-1;
            _tOffset += line.size()-1;
        } else if(Util::eq(dir, ".word")){
            _tSecOffset += 2*(line.size()-1);
            _tOffset += 2*(line.size()-1);
        } else if(Util::eq(dir, ".long")){
            _tSecOffset += 4*(line.size()-1);
            _tOffset += 4*(line.size()-1);
        } else if(Util::eq(dir, ".skip")){
            int toSkip = Util::intValue(line[1], lineNum);
            _tSecOffset += toSkip;
            _tOffset += toSkip;
        } else if(Util::eq(dir, ".align")){
            int toAllign = Util::alignBytes(_tOffset, Util::intValue(line[1], lineNum));
            _tSecOffset += toAllign;
            _tOffset += toAllign;
        } else {
            Util::error(lineNum, "Nepoznata direktiva: \033[0;31m" + dir + "\033[0m");
        }
    }

}

void PrviProlaz::decodeInstruction(std::vector<std::string> line, int lineNum){
    std::string instr = line[0];

    // errcheck
    std::vector<std::string>::iterator it;
    it = std::find(instrNames.begin(), instrNames.end(), instr);
    if(it == instrNames.end())
        Util::error(lineNum, "Instrukcija \033[0;32m" + instr + "\033[0m nije definisana.");
    // errcheck
    //?

    //calc size of instr
    // toffset += size
    // tSecOffset +=size

    //is 4b instruction?
    int size = Util::is4bInstruction(line, lineNum)?4:2;
    _tOffset+=size;
    _tSecOffset+=size;


}



void PrviProlaz::print(std::string s){
    std::cout << "[ PrviProlaz]    " << s <<"\n";
}


PrviProlaz::~PrviProlaz(){}
