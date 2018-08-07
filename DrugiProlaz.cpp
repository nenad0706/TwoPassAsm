#include "DrugiProlaz.h"

int DrugiProlaz::_tOffset = 0;
int DrugiProlaz::_tOffsetUSekciji = 0;
int DrugiProlaz::_tSekcija = -1;

DrugiProlaz::DrugiProlaz(int startAddr, std::vector<std::string> inp):_input(inp){
    _tOffset = startAddr;
    _end = false;
    std::cout << "[ DrugiProlaz]  START\n";


}
DrugiProlaz::~DrugiProlaz(){}

void DrugiProlaz::process(){
    std::vector<std::string> line;

	int lineNum = 0;
    for(std::string ln : _input){
        if(!_end){

            line = Util::split_string(ln);
            //dbg
            //print(line[0]);

            processLine(line, Util::lineNums[lineNum]);

            lineNum++;
        }
    }
    print("Upisujem u izlazni fajl.");

}

void DrugiProlaz::processLine(std::vector<std::string> line, int lineNum){
    if(line[0].find(':')!= std::string::npos){ processLabel(line, lineNum); }
    else if (line[0].at(0) == '.') { processDirective(line, lineNum); }
    else { processInstruction(line, lineNum); }


}

/*
    ===================================================================================================
    ====================================    I N S T R U C T I O N   ===================================
    ===================================================================================================
*/

void DrugiProlaz::processInstruction(std::vector<std::string> line, int lineNum){
    std::string instr = line[0];
        // TODO errcheck?
    validateInstruction(line, lineNum);

    if(Util::has(instr, "jmp"))
    {
        // pseudo jump
        if(Util::has(line[1], "$")){
            // jmp $x -> add r7, x
            line[0]= "add" + getAddrMode(instr);
            line.push_back(line[1].substr(1, line[1].size()));
            line[1] = "r7";
            processInstruction(line, lineNum);
            return;
        } else {
            // jmp x -> mov r7, x
            line[0] = "mov" + getAddrMode(instr);
            line.push_back(line[1]);
            line[1] = "r7";
            processInstruction(line, lineNum);
            return;
        }
    }
    else if (Util::has(instr, "ret") && !Util::has(instr, "iret"))
    {
        // pseudo ret
        line[0] = "pop" + getAddrMode(instr);
        line.push_back("r7");
        processInstruction(line, lineNum);
        return;
    }
    else if(!Util::is4bInstruction(line, lineNum))
    {
        Word16 code = Util::opcode[getAddrMode(instr)];
        code = code | Util::opcode[getInstr(instr)];
        if(line.size() > 1 ) code = code | Util::op1(line, lineNum);
        if(line.size() > 2 ) code = code | Util::op2(line, lineNum);
        writeSection(code);
        incOffset(2);
    } else {
                                                                        //
        Word16 opcode;                                                  //      I N S T R U K C I J E   4 B
        Word16 val;                                                     //
        opcode = Util::opcode[getAddrMode(instr)];
        opcode = opcode | Util::opcode[getInstr(instr)];
        for(unsigned int i=1; i< line.size(); i++){
            std::string op = line[i];
            Word16 opp;
            std::string cut = op.substr(1, op.length());
            if(!Util::isPureReg(op)){

                // PAZI SAMO ZA * (ZA TEST)
                //opp = Util::opcode["memdir"] << 3;
                // calculate addr mode:
                //  * memdir
                //  & immed
                if(op[0] == '*') opp = Util::opcode["memdir"] << 3;
                else if (op[0] == '&') opp = Util::opcode["immed"] << 3;
                else if (op[0] == '$' ) {
                    opp = Util::opcode["regindpom"] << 3;
                    opp = opp | 0x07; //koristi se pc
                }
                else if (Util::has(op, "[")) {
                    opp = Util::opcode["regindpom"] << 3;
                    opp = opp | Util::opcode[Util::getReg(op)]; // koji registar je u pitanju
                }
                else if(!Util::isInteger(op)){
                    // ako je labela, samo memdir, [memdir + dodatna 2B]
                    opp = Util::opcode["memdir"] << 3;
                }
                // ako je neposredno adresiranje, 0x00 [immed + dodatna 2B], ne mora nista da se radi

                val = reloc(op, _tOffsetUSekciji + 2, lineNum);
                if(i==1){
                    opp = opp << 5;
                }

            } else {
                if(i==1){ opp=Util::op1(line, lineNum); }
                else opp=Util::op2(line, lineNum);
            }

            //pisi opcode
            opcode = opcode | opp;
        } // for both ops

        //write opcode
        //write val LITTLE ENDIAN
        writeSection(opcode);
        writeSectionLE(val);

        incOffset(4);
    }

}

/*
    ===================================================================================================
    ====================================    R E L O C ( )   ===========================================
    ===================================================================================================
*/
Word16 DrugiProlaz::reloc(std::string op, int toffset, int lineNum){
    Word16 ret;

    if(Util::isInteger(op)){
        ret = (Word16)Util::intValue(op, lineNum);
        return ret;
    }
    std::string sym;

    if(op[0] == '*'){                                   /*================  *  *  *  *  *  *  *   ================================*/
        sym = op.substr(1, op.length());

        //check if sym is number
        if(Util::isInteger(sym)){
            return (Word16)Util::intValue(sym, -1);
        }

        Util::SymTablRecord *str = SymTab::get(sym, lineNum);
        if(str->local){
            //reloc sect
            int sekcija = str->section;
            std::string sec;
            switch(sekcija){
                case 0: sec = ".rodata"; break;
                case 1: sec = ".data"; break;
                case 2: sec = ".text"; break;
                default: sec = ".bss"; break;
            }
            Util::SymTablRecord *strSekcija = SymTab::get(sec, lineNum);
            Util::addRelocRecord(_tSekcija, toffset, STD_RELOK, strSekcija->rbr, '+');
            return str->offset;
        } else {
            //reloc symb
            Util::addRelocRecord(_tSekcija, toffset, STD_RELOK, str->rbr, '+');
            return 0x0000;
        }
    } // *

    else if (op[0]=='&'){                               /*================  &  &  &  &  &  &  &  ================================*/
        sym = op.substr(1, op.length());

        if(Util::isInteger(sym)){
            Util::error(lineNum, "Nedozvoljen nacin adresiranja.");
        }

        Util::SymTablRecord *str = SymTab::get(sym, lineNum);
        if(str->local){
            //reloc sect
            int sekcija = str->section;
            std::string sec;
            switch(sekcija){
                case 0: sec = ".rodata"; break;
                case 1: sec = ".data"; break;
                case 2: sec = ".text"; break;
                default: sec = ".bss"; break;
            }

            Util::SymTablRecord *strSekcija = SymTab::get(sec, lineNum);
            Util::addRelocRecord(_tSekcija, toffset, STD_RELOK, strSekcija->rbr, '+');
            return str->offset;
        } else {
            //reloc symb
            Util::addRelocRecord(_tSekcija, toffset, STD_RELOK, str->rbr, '+');
            return 0x0000;
        }

    } // &

    else if (op[0]=='$'){                               /*================  $  $  $  $  $  $  ================================*/
        //regindpom, val
        sym = op.substr(1, op.length());

        //check if sym is number
        if(Util::isInteger(sym)){
            return (Word16)Util::intValue(sym, -1);
        }

        Util::SymTablRecord *str = SymTab::get(sym, lineNum);
        if(str->local){
            //reloc sect
            int sekcija = str->section;
            std::string sec;
            switch(sekcija){
                case 0: sec = ".rodata"; break;
                case 1: sec = ".data"; break;
                case 2: sec = ".text"; break;
                default: sec = ".bss"; break;
            }
            Util::SymTablRecord *strSekcija = SymTab::get(sec, lineNum);
            Util::addRelocRecord(_tSekcija, toffset, STD_RELOK, strSekcija->rbr, '+');
            return str->offset;
        } else {
            //reloc symb
            Util::addRelocRecord(_tSekcija, toffset, STD_RELOK, str->rbr, '+');
            return 0x0000;
        }

    } // $
    else if(Util::has(op, "]")){                        /*================  reg[]  reg[]  reg[]   ================================*/
        // broj ili [labela] - immed / mem[labela]
        std::string pomeraj = Util::getPomeraj(op);
        std::string reg = Util::getReg(op);

        if(Util::isInteger(pomeraj)){
            return (Word16)Util::intValue(pomeraj, 0);  //nikad nece baciti gresku, broj linije nije bitan (0)
        } else {
            Util::SymTablRecord *str = SymTab::get(pomeraj, lineNum);

            if(str->local){
                //reloc sect
                int sekcija = str->section;
                std::string sec;
                switch(sekcija){
                    case 0: sec = ".rodata"; break;
                    case 1: sec = ".data"; break;
                    case 2: sec = ".text"; break;
                    default: sec = ".bss"; break;
                }
                Util::SymTablRecord *strSekcija = SymTab::get(sec, lineNum);
                Util::addRelocRecord(_tSekcija, toffset, STD_RELOK, strSekcija->rbr, '+');
                return str->offset;
            } else {
                //reloc symb
                Util::addRelocRecord(_tSekcija, toffset, STD_RELOK, str->rbr, '+');
                return 0x0000;
            }
        }
    }
    else {                                          /*================  simbol simbol simbol   ================================*/
        Util::SymTablRecord *str = SymTab::get(op, lineNum);
        if(str->local){
        //reloc sect
            int sekcija = str->section;
            std::string sec;
            switch(sekcija){
                case 0: sec = ".rodata"; break;
                case 1: sec = ".data"; break;
                case 2: sec = ".text"; break;
                default: sec = ".bss"; break;
            }
            Util::SymTablRecord *strSekcija = SymTab::get(sec, lineNum);
            Util::addRelocRecord(_tSekcija, toffset, STD_RELOK, strSekcija->rbr, '+');
            return str->offset;
        } else {
                //reloc symb
            Util::addRelocRecord(_tSekcija, toffset, STD_RELOK, str->rbr, '+');
            return 0x0000;
        }
    }

    return ret;
}


std::string DrugiProlaz::getAddrMode(std::string s){
    return s.substr(s.size()-2, s.size()-1);
}

std::string DrugiProlaz::getInstr(std::string s){
    return s.substr(0, s.size()-2);
}

void DrugiProlaz::validateInstruction(std::vector<std::string> line, int lineNum){
    checkSection(lineNum);
    for(std::string s : line){
        if(Util::has(s, "[") && !Util::has(s, "]"))Util::error(lineNum, "Sintaksna greska [");
    }
    /* TODO
      if(line.size() != (shouldHave + 1)) Util::error(lineNum, "Instrukcija \
                                                           nema odgovarajuci broj operanada"); */
}

void DrugiProlaz::processDirective(std::vector<std::string> line, int lineNum){
    if(Util::eq(line[0], ".global")){
        for(unsigned int i=1; i< line.size(); i++){
            if(!SymTab::contains(line[i])){
                Util::error(lineNum, "Simbol koji se izvozi nije definisan (" + line[i] + ")");
            }
            SymTab::setGlobal(line[i]);
        }
    } else if(Util::eq(line[0], ".rodata")){
        changeSection(0);
    } else if(Util::eq(line[0], ".data")){
        changeSection(1);
    } else if(Util::eq(line[0], ".text")){
        changeSection(2);
    } else if(Util::eq(line[0], ".bss")){
        changeSection(3);
    } else if(Util::eq(line[0], ".skip")){
        Byte8 skipVal = 0;
        int num = Util::intValue(line[1], lineNum);
        if(line.size() > 2) skipVal = Util::intValue(line[2], lineNum);
        for(int i=0;i<num;i++){
            writeSection((Byte8)skipVal);
        }
        incOffset(num);
    } else if(Util::eq(line[0], ".align")){
        int num = Util::alignBytes(_tOffset, Util::intValue(line[1], lineNum));
        for(int i=0; i<num; i++){
            writeSection((Byte8)0);
        }
        incOffset(num);
    } else if(Util::eq(line[0], ".char")){
        for(unsigned int i=1; i<line.size(); i++){
            if(line[i][0] == '\''){
                writeSection((Byte8)extractChar(line[i], lineNum));
            } else {
                int charVal = Util::intValue(line[i], lineNum);
                if(charVal > 255) Util::error(lineNum, "Vrednost se ne moze smestiti u 8 bita (.char " + line[i] + ")");
                writeSection((Byte8)(charVal & 0x0ff));
            }
        }
        incOffset(line.size() - 1);
    } else if(Util::eq(line[0], ".long")){                              // ----------- . L O N G
        for(unsigned int i=1; i< line.size(); i++){
            Long32 val = 0x00000000;
            if(Util::has(line[i], "+") || Util::has(line[i], "-")){
            // u pitanju je izraz
                std::vector<std::string> izraz = Util::splitIzraz(line[i], lineNum);
                int sign = 1;
                for(std::string s : izraz){
                    if(Util::isPureInteger(s)){
                        val += (sign*Util::intValue(s, -1));
                    } else if (Util::eq(s, "-")){
                        sign = -1;
                    } else if (Util::eq(s, "+")){
                        sign = +1;
                    } else {
                    //labela
                        Long32 toAdd;
                        Util::SymTablRecord *str = SymTab::get(s, lineNum);
                        if(str->local){
                        //reloc sect
                            int sekcija = str->section;
                            std::string sec;
                            switch(sekcija){
                                case 0: sec = ".rodata"; break;
                                case 1: sec = ".data"; break;
                                case 2: sec = ".text"; break;
                                default: sec = ".bss"; break;
                            }
                            Util::SymTablRecord *strSekcija = SymTab::get(sec, lineNum);
                            char signChar;
                            if(sign > 0) signChar = '+';
                            else signChar = '-';
                            Util::addRelocRecord(_tSekcija, _tOffsetUSekciji, STD_RELOK, strSekcija->rbr, signChar);
                            toAdd = 0x00000000 | str->offset;
                        } else {
                                //reloc symb
                            char signChar;
                            if(sign > 0) signChar = '+';
                            else signChar = '-';
                            Util::addRelocRecord(_tSekcija, _tOffsetUSekciji, STD_RELOK, str->rbr, signChar);
                            toAdd = 0x00000000;
                        }
                        val += sign*toAdd;
                    }
                } // izraz

            } else {
            // nije izraz -> .long 3, .long labela
                if(Util::isInteger(line[i])){
                    val = (Long32)Util::intValue(line[i], -1);
                } else if(line[i][0] == '&'){
                    // podrzano je .long labela, a ne .long &labela, u tom slucaju se uvek uzima adresa labele
                    Util::error(lineNum, "Za direktive su dozvoljena samo imena labela bez eksplicitnog nacina adresiranja.");
                } else {
                    Util::SymTablRecord *str = SymTab::get(line[i], lineNum);
                    if(str->local){
                    //reloc sect
                        int sekcija = str->section;
                        std::string sec;
                        switch(sekcija){
                            case 0: sec = ".rodata"; break;
                            case 1: sec = ".data"; break;
                            case 2: sec = ".text"; break;
                            default: sec = ".bss"; break;
                        }
                        Util::SymTablRecord *strSekcija = SymTab::get(sec, lineNum);
                        Util::addRelocRecord(_tSekcija, _tOffsetUSekciji, STD_RELOK, strSekcija->rbr, '+');
                        val = 0x00000000 | str->offset;
                    } else {
                            //reloc symb
                        Util::addRelocRecord(_tSekcija, _tOffsetUSekciji, STD_RELOK, str->rbr, '+');
                        val = 0x00000000;
                    }
                }
            }
            writeSectionLE(val);
            incOffset(4);
        }

    } // _end .long
    else if(Util::eq(line[0], ".word")){                                    // ----------- . W O R D
        for(unsigned int i=1; i< line.size(); i++){
            Word16 val = 0x0000;
            if(Util::has(line[i], "+") || Util::has(line[i], "-")){
            // u pitanju je izraz ------------------------------------------------------------------- izraz
                std::vector<std::string> izraz = Util::splitIzraz(line[i], lineNum);
                int sign = 1;
                for(std::string s : izraz){
                    if(Util::isPureInteger(s)){
                        val += (Word16)(sign*Util::intValue(s, -1));
                    } else if (Util::eq(s, "-")){
                        sign = -1;
                    } else if (Util::eq(s, "+")){
                        sign = +1;
                    } else {
                    //labela
                        Word16 toAdd;
                        Util::SymTablRecord *str = SymTab::get(s, lineNum);
                        if(str->local){
                        //reloc sect
                            int sekcija = str->section;
                            std::string sec;
                            switch(sekcija){
                                case 0: sec = ".rodata"; break;
                                case 1: sec = ".data"; break;
                                case 2: sec = ".text"; break;
                                default: sec = ".bss"; break;
                            }
                            Util::SymTablRecord *strSekcija = SymTab::get(sec, lineNum);
                            char signChar;
                            if(sign > 0) signChar = '+';
                            else signChar = '-';
                            Util::addRelocRecord(_tSekcija, _tOffsetUSekciji, STD_RELOK, strSekcija->rbr, signChar);
                            toAdd = 0x0000 | (Word16)str->offset;
                        } else {
                                //reloc symb
                            char signChar;
                            if(sign > 0) signChar = '+';
                            else signChar = '-';
                            Util::addRelocRecord(_tSekcija, _tOffsetUSekciji, STD_RELOK, str->rbr, signChar);
                            toAdd = 0x0000;
                        }
                        val += sign*toAdd;
                    }
                } // izraz

            } else { // ---------------------------------------------------------------- end izraz
            // nije izraz -> .word 3, .word labela
                if(Util::isInteger(line[i])){
                    val = (Word16)Util::intValue(line[i], -1);
                } else if(line[i][0] == '&'){
                    // podrzano je .word labela, a ne .long &labela, u tom slucaju se uvek uzima adresa labele
                    Util::error(lineNum, "Za direktive su dozvoljena samo imena labela bez eksplicitnog nacina adresiranja.");
                } else {
                    Util::SymTablRecord *str = SymTab::get(line[i], lineNum);
                    if(str->local){
                    //reloc sect
                        int sekcija = str->section;
                        std::string sec;
                        switch(sekcija){
                            case 0: sec = ".rodata"; break;
                            case 1: sec = ".data"; break;
                            case 2: sec = ".text"; break;
                            default: sec = ".bss"; break;
                        }
                        Util::SymTablRecord *strSekcija = SymTab::get(sec, lineNum);
                        Util::addRelocRecord(_tSekcija, _tOffsetUSekciji, STD_RELOK, strSekcija->rbr, '+');
                        val = 0x0000 | (Word16)str->offset;
                    } else {
                            //reloc symb
                        Util::addRelocRecord(_tSekcija, _tOffsetUSekciji, STD_RELOK, str->rbr, '+');
                        val = 0x0000;
                    }
                }
            }
            writeSectionLE(val);
            incOffset(2);
        }

    } // _end .word
    else {

    }
}

int DrugiProlaz::extractChar(std::string s, int lineNum){
    if(s.length()!=3)Util::error(lineNum, "Ne moze biti procesiran karakter.");
    return (int)s[1];
}

void DrugiProlaz::processLabel(std::vector<std::string> line, int lineNum){
    if(line.size() > 1){
        std::vector<std::string> afterLabel = line;
        afterLabel.erase(afterLabel.begin());
        processLine(afterLabel, lineNum);
    }
}

void DrugiProlaz::print(std::string s){
    std::cout << "[ DrugiProlaz]    " << s <<"\n";
}

// offset se povecava posle ove fje
void DrugiProlaz::writeSection(Word16 code){
    //addSectionRecord(int sekcija, int offset, int sadrzaj);
    Util::addSectionRecord(_tSekcija, _tOffsetUSekciji, (code & 0xff00) >> 8);
    Util::addSectionRecord(_tSekcija, _tOffsetUSekciji+1, (code & 0x00ff));
}

void DrugiProlaz::writeSectionLE(Word16 code){
    Util::addSectionRecord(_tSekcija, _tOffsetUSekciji, (code & 0x00ff));
    Util::addSectionRecord(_tSekcija, _tOffsetUSekciji+1, (code & 0xff00) >> 8);
}

void DrugiProlaz::writeSectionLE(Long32 code){
    Util::addSectionRecord(_tSekcija, _tOffsetUSekciji, (code & 0x000000ff));
    Util::addSectionRecord(_tSekcija, _tOffsetUSekciji+1, (code & 0x0000ff00) >> 8);
    Util::addSectionRecord(_tSekcija, _tOffsetUSekciji+2, (code & 0x00ff0000) >> 16);
    Util::addSectionRecord(_tSekcija, _tOffsetUSekciji+3, (code & 0xff000000) >> 24);
}

void DrugiProlaz::writeSection(Byte8 code){
    Util::addSectionRecord(_tSekcija, _tOffsetUSekciji, code);
}

void DrugiProlaz::checkSection(int lineNum){
    if(_tSekcija == -1) Util::error(lineNum, "Sekcija nije definisana.");
}

void DrugiProlaz::incOffset(int val){
    _tOffset += val;
    _tOffsetUSekciji +=val;
}

void DrugiProlaz::changeSection(int section){
    _tSekcija = section;
    _tOffsetUSekciji = 0;
}

