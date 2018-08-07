#include "Util.h"

Util::Util(){}
Util::~Util(){}



std::vector<std::vector<Util::SectionRecord*>> Util::sekcije;
std::vector<std::vector<Util::RelocRecord*>> Util::relSekcije;
std::vector<int> Util::lineNums;
std::map<std::string, Word16> Util::opcode;


Word16 Util::op1(std::vector<std::string> line, int lineNum){
    Word16 ret = 0;
    // return (opcode[addrmode] << 8) | (opcode[reg] << 5);
    // regdir
    if(Util::isPureReg(line[1])){
        //regdir + reg
        if(Util::eq(line[1], "psw")){
            ret = Util::opcode["psw_immed"] << 5;
        } else {
            ret = Util::opcode["regdir"] << 8;
            ret = ret | (Util::opcode[line[1]] << 5);
        }
    }
    return ret;
}

Word16 Util::op2(std::vector<std::string> line, int lineNum){
    Word16 ret = 0;
    // return (opcode[addrmode] << 8) | (opcode[reg] << 5);
    // regdir
    if(Util::isPureReg(line[2])){
        if(Util::eq(line[2], "psw")){
            ret = Util::opcode["psw_immed"];
        } else {
            //regdir + reg
            ret = Util::opcode["regdir"] << 3;
            ret = ret | Util::opcode[line[2]];
        }
    }
    std::cout<< "NIJE PURE REG";
    return ret;
}

void Util::addSectionRecord(int sekcija, int offset, int sadrzaj){
    SectionRecord *sr = new SectionRecord(offset, sadrzaj);
    Util::sekcije[sekcija].push_back(sr);
}

void Util::addRelocRecord(int sekcija, int offset, int tipR, int rbrSimb, char sign){
    RelocRecord *sr = new RelocRecord(offset, tipR, rbrSimb, sign);
    Util::relSekcije[sekcija].push_back(sr);
}

void Util::printSection(int sekcija, std::ostream& stream){
    unsigned int size = sekcije[sekcija].size();//sizeof(SectionRecord*);
    if(size == 0)return;
    stream << "\n# " << sectionName(sekcija) << "                 \n";
    int cr = 0;
    for(unsigned int i= 0; i < size; i++){
        stream << std::hex << std::setfill('0') << std::setw(2) << sekcije[sekcija][i]->sadrzaj << " ";
        if(++cr == 16) {
            stream<<"\n";
            cr = 0;
        }
    }
    stream<<"\n\n";
}

void Util::printRelSection(int sekcija, std::ostream& stream){
    unsigned int size = relSekcije[sekcija].size();
    if(size == 0)return;
    stream<<"\n# Reloc." << sectionName(sekcija) << "\n";
    stream << "#      Offset   Tip   Rb.Simb.  Sgn \n";
    for(unsigned int i= 0; i < size; i++){
       stream << i << "\t"
            << relSekcije[sekcija][i]->offset << "\t"
            << relSekcije[sekcija][i]->tipRelokacije << "\t"
            << relSekcije[sekcija][i]->rbSimbola << "\t"
            << relSekcije[sekcija][i]->sign << "\n";

    }
}

std::string Util::sectionName(int num){
    switch(num){
        case 0: return "RODATA";
        case 1: return "DATA";
        case 2: return "TEXT";
        default: return "BSS";
    }
}

bool Util::is4bInstruction(std::vector<std::string> line, int lineNum)
{
	bool prvi_pure = true;
	bool ima_drugi = false;
	bool drugi_pure = true;
	if (line.size() == 1) return false; // u pitanju iret
	if (!isPureReg(line[1])) prvi_pure = false;; // return true
	if (line.size() > 2)
	{
		ima_drugi = true;
		if (!isPureReg(line[2])) drugi_pure = false; // return false
	}
	if (ima_drugi && !prvi_pure && !drugi_pure)
	{
	    Util::error(lineNum, "Oba operanda instrukcije zahtevaju dodatni podatak.");
	}
	if (!ima_drugi && !prvi_pure) return true;
	if (ima_drugi && (!prvi_pure || !drugi_pure)) return true;
	return false;
}

bool Util::isPureReg(std::string s)
{
	if ( Util::eq(s, "r0") || Util::eq(s, "r1") || Util::eq(s, "r2") || Util::eq(s, "r3")
			|| Util::eq(s, "r4") || Util::eq(s, "r5") || Util::eq(s, "r6") || Util::eq(s, "r7")
			|| Util::eq(s, "pc") || Util::eq(s, "sp") || Util::eq(s, "psw") || Util::eq(s, "PSW") )
        return true;
	else return false;
}

bool Util::isPureInteger(std::string s){
    for(unsigned int i=0; i<s.length(); i++){
        if(s[i] < '0' || s[i]>'9') return false;
    }
    return true;
}

bool Util::isInteger(std::string s){
    for(unsigned int i = ((s[0] == '-')?1:0);i<s.length(); i++){
        if(s[i] < '0' || s[i]>'9') return false;
    }
    return true;
}

bool Util::has(std::string s, std::string sub){
    if(s.find(sub) != std::string::npos) return true;
    else return false;
}

std::vector<std::string> Util::split_string(std::string s){
	std::replace(s.begin(), s.end(), '\t', ' ');
	std::string delimiter = " ";
	std::vector<std::string> vect;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos)
	{
		token = s.substr(0, pos);

		if (!eq(token, ""))
		{
			if (token.at(token.length() - 1) == ',')
			{
				token.erase(token.length() - 1, token.length());
			}

			if (!eq(token, "") || !token.empty() || !eq(token, "\n"))
			{
				vect.push_back(token);
			}
		}
		s.erase(0, pos + delimiter.length());
	}
	token = s;
	if (!eq(token, "") || !token.empty() || !eq(token, "\n")) {
		vect.push_back(token);
	}

	return vect;
}

std::string Util::getPomeraj(std::string op){
    int poss = op.find('[');
    int pose = op.find(']');
    int sz = pose - poss -1;
    std::string pom = op.substr((poss+1), sz);
    return pom;
}

std::string Util::getReg(std::string op){
    int poss = op.find('[');
    return op.substr(0, poss);
}

bool Util::eq(std::string s1, std::string s2){

    if(s1.size() != s2.size()) return false;
	for (unsigned int i=0; i< s1.size(); i++)
	{
		if(s1[i] != s2[i]) return false;
	}
	return true;
}

void Util::error(int lineNum, std::string greska){
    std::cout<< red <<"[ " << "GRESKA @linija " << lineNum << "] " reset
                << greska << red"\n[ EXIT]" << reset << "\n";
    std::exit(1);
}

void Util::error(std::string greska){
    std::cout<< red <<"[ GRESKA] "
                << greska << "\n[ EXIT]" << reset << "\n";
    std::exit(1);
}

int Util::alignBytes(int tOffset, int val){
    return val-(tOffset%val);
}

int Util::intValue(std::string s, int lineNum){
    bool jesteInt = (!s.empty() && s.find_first_not_of("0123456789") == std::string::npos);
    if(!jesteInt){
        error(lineNum, "Parametar \033[0;31m" + s + "\033[0m nije ceo broj.");
    }
    return std::stoi(s);
}

std::vector<std::string> Util::splitIzraz(std::string s, int lineNum){
    bool signOK = false;
    std::vector<std::string> retVector;
    std::string op = "";
    for(unsigned int i=0; i<s.length(); i++){

        if(s[i] != '+' && s[i] != '-'){
            op+=s[i];
            signOK = true;
        } else {
            if(!signOK)error(lineNum, "Neispravan izraz.");
            retVector.push_back(op);
            op = (s[i]=='-'?"-":"+");
            retVector.push_back(op);
            op = "";
            signOK = false;
        }
    }
    retVector.push_back(op);
    return retVector;
}

void Util::init(){
    std::vector<SectionRecord*> v1;
    std::vector<SectionRecord*> v2;
    std::vector<SectionRecord*> v3;
    std::vector<SectionRecord*> v4;
    sekcije.push_back(v1);
    sekcije.push_back(v2);
    sekcije.push_back(v3);
    sekcije.push_back(v4);

    std::vector<RelocRecord*> r1;
    std::vector<RelocRecord*> r2;
    std::vector<RelocRecord*> r3;
    std::vector<RelocRecord*> r4;
    relSekcije.push_back(r1);
    relSekcije.push_back(r2);
    relSekcije.push_back(r3);
    relSekcije.push_back(r4);
    /* insert into opcodes */
	//adr modes
	opcode.insert(std::make_pair("immed", 0x0000));
	opcode.insert(std::make_pair("regdir", 0x0001));
	opcode.insert(std::make_pair("memdir", 0x0002));
	opcode.insert(std::make_pair("regindpom", 0x0003));

	//conditions
	opcode.insert(std::make_pair("eq", 0x0000));
	opcode.insert(std::make_pair("ne", 0x4000));
	opcode.insert(std::make_pair("gt", 0x8000));
	opcode.insert(std::make_pair("al", 0xc000));

	// opcodes
	opcode.insert(std::make_pair("add", 0x0000));
	opcode.insert(std::make_pair("sub", 0x0400));
	opcode.insert(std::make_pair("mul", 0x0800));
	opcode.insert(std::make_pair("div", 0x0c00));
	opcode.insert(std::make_pair("cmp", 0x1000));
	opcode.insert(std::make_pair("and", 0x1400));
	opcode.insert(std::make_pair("or", 0x1800));
	opcode.insert(std::make_pair("not", 0x1c00));
	opcode.insert(std::make_pair("test", 0x2000));
	opcode.insert(std::make_pair("push", 0x2400));
	opcode.insert(std::make_pair("pop", 0x2800));
	opcode.insert(std::make_pair("call", 0x2c00));
	opcode.insert(std::make_pair("iret", 0x3000));
	opcode.insert(std::make_pair("mov", 0x3400));
	opcode.insert(std::make_pair("shl", 0x3800));
	opcode.insert(std::make_pair("shr", 0x3c00));

	//registers
	opcode.insert(std::make_pair("r0", 0x00));
	opcode.insert(std::make_pair("r1", 0x01));
	opcode.insert(std::make_pair("r2", 0x02));
	opcode.insert(std::make_pair("r3", 0x03));
	opcode.insert(std::make_pair("r4", 0x04));
	opcode.insert(std::make_pair("r5", 0x05));
	opcode.insert(std::make_pair("r6", 0x06));
	opcode.insert(std::make_pair("r7", 0x07));
	opcode.insert(std::make_pair("sp", 0x06));
	opcode.insert(std::make_pair("pc", 0x07));
	opcode.insert(std::make_pair("psw_immed", 0x07));
}
