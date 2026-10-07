#include <iostream>
#include <cstdlib>
#include <set>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <regex>
#include <fstream>
#include <cmath>

namespace {
    // instruction set
    const std::set<std::string> ins_set = {"add", "nand", "lw", "sw", "beq", "jalr", "halt", "noop", ".fill"};
    // label table
    std::map<std::string, int> label;
    // split string to List
    std::list<std::string> split(std::string str) {
        std::stringstream ss(str);
        std::string segment;
        // list of segment
        std::list<std::string> seglist;

        while (ss >> segment) {
            seglist.push_back(segment);
        }
        return seglist;
    }

    class Pass1{
        private:
            // string in each line
            std::string line;
            int line_count = -1;

        public:
            Pass1(){}

            void setLine(std::string str) {
                this->line = str;
            }

            // Convert a string into assembly format
            std::map<std::string, std::string> compute() {
                std::map<std::string, std::string> result;
                std::list<std::string> list = split(line);
                if(list.empty()) {return result;}
                line_count++;
                // set address of code in this line
                result.insert({"address", std::to_string(line_count)});
                std::vector<std::string> formats = {"label", "instruction", "field0", "field1", "field2"};
                // variable of field
                std::string f;
                // size of field in each format
                int size = 0;
                // check if The very front of field is instruction?
                if(ins_set.count(list.front())){
                    f = list.front();
                    result.insert({formats[1], f});
                    list.pop_front();
                }
                else{
                    // if not will assume that is lable
                    if(label.find(list.front()) != label.end()) {
                        std::cerr << "Error: Duplicate label " << list.front() << " in line " << line_count << std::endl;
                        exit(1);
                    }
                    result.insert({formats[0], list.front()});
                    label.insert({list.front(), line_count});
                    list.pop_front();
                    if(list.empty()) {return result;}
                    
                    // check if next is instruction?
                    if(ins_set.count(list.front())){
                        f = list.front();
                        result.insert({formats[1], f});
                        list.pop_front();
                    }
                    else{
                        std::cerr << "Error: Unknown opcode " << list.front() << " in line " << line_count << std::endl;
                        exit(1);
                    }

                }

                // set size of field according to the instruction
                if(f == "add" || f == "nand" || f == "lw" || f == "sw" || f == "beq"){size = 3;}
                else if(f == "jalr"){size = 2;}
                else if(f == ".fill"){size = 1;}
                // if instruction is noop or halt use default
                //The excess will be ignored and turned into a comment
                
                // check if in the line has all the field and put it in result map
                for(int i=0; i<size; i++){
                    if(list.empty()) {
                        std::cerr << "Error: missing field for instruction " << f << " in line " << line_count << std::endl;
                        exit(1);
                    }
                    result.insert({formats[i+2], list.front()});
                    list.pop_front();
                }
                return result;
            }
    };

    class Pass2{
        private:
            std::map<std::string, std::string> code;

            // check is string has farmat of number?
            bool isNumber(std::string str){
                std::regex pattern("^[+-]?[0-9]*$");
                return std::regex_match(str, pattern);
            }

            // transform string to int
            int stringToInt(std::string str){
                int num = 0;
                std::stringstream ss(str);
                ss >> num;
                return num;
            }

            // transform int to vector of binary
            std::vector<int> intToBinaryVec(int num, int size){
                if(num<0) {
                    num += (1 << size);
                }
                std::vector<int> binaryVec;
                for(int i=0; i<size; i++){
                    binaryVec.insert(binaryVec.begin(), num%2);
                    num /= 2;
                }
                return binaryVec;
            }

            // transform int to string of binary
            std::vector<int> strToBinaryVec(std::string str, int size){
                int num = stringToInt(str);
                return intToBinaryVec(num, size);
            }

            // check Undefined Label
            void checkLebel(std::string str, int line){
                if(label.count(str) == 0) {
                    std::cerr << "Error: Undefined Label at line " << line << std::endl;
                    exit(1);
                }
            }

            // check OffsetField Out of Range
            void checkOffset16(int i, int line){
                if(i<-32768|| i>32767){
                    std::cerr << "Error: OffsetField Out of Range at line " << line << std::endl;
                    exit(1);
                }
            }

        public:
            Pass2(){}

            void setCode(const std::map<std::string, std::string>& newCode) {
                code = newCode;
            }

            int compute() {
                // vector of binary
                std::vector<int> bi;
                // variable of instruction field
                std::string op = code.at("instruction");
                // check if instruction field is R-type
                if(op == "add" || op == "nand"){
                    if(op == "add") bi = {0, 0, 0};
                    else bi = {0, 0, 1};
                    // assign the field to a variable
                    std::vector<int> field0 = strToBinaryVec(code.at("field0"), 3);
                    // concatenate vectors
                    bi.insert(bi.end(), field0.begin(), field0.end());
                    std::vector<int> field1 = strToBinaryVec(code.at("field1"), 3);
                    bi.insert(bi.end(), field1.begin(), field1.end());
                    Padding a vector with zeros
                    for(int i=3; i<16; i++){
                        bi.push_back(0);
                    }
                    std::vector<int> field2 = strToBinaryVec(code.at("field2"), 3);
                    bi.insert(bi.end(), field2.begin(), field2.end());
                }
                // check if instruction field is I-type
                else if(op == "lw" || op == "sw" || op == "beq"){
                    if(op == "lw") bi = {0, 1, 0};
                    else if(op == "sw") bi = {0, 1, 1};
                    else bi = {1, 0, 0};
                    std::vector<int> field0 = strToBinaryVec(code.at("field0"), 3);
                    bi.insert(bi.end(), field0.begin(), field0.end());
                    std::vector<int> field1 = strToBinaryVec(code.at("field1"), 3);
                    bi.insert(bi.end(), field1.begin(), field1.end());
                    std::vector<int> field2;
                    // check if field2 is offsetField?
                    if(isNumber(code.at("field2"))) {
                        checkOffset16(stringToInt(code.at("field2")), stringToInt(code.at("address")));
                        field2 = strToBinaryVec(code.at("field2"), 16);
                    }
                    // check if instruction field is lw or sw?
                    else if(op != "beq"){
                        checkLebel(code.at("field2"), stringToInt(code.at("address")));
                        checkOffset16(label.at(code.at("field2")), stringToInt(code.at("address")));
                        field2 = intToBinaryVec(label.at(code.at("field2")), 16);
                    }
                    else {
                        checkLebel(code.at("field2"), stringToInt(code.at("address")));
                        int field2Demo = label.at(code.at("field2")) - stringToInt(code.at("address")) - 1;
                        checkOffset16(field2Demo, stringToInt(code.at("address")));
                        field2 = intToBinaryVec(field2Demo, 16);
                    }
                    bi.insert(bi.end(), field2.begin(), field2.end());
                }
                // check if instruction field is jalr?
                else if (op == "jalr"){
                    bi = {1, 0, 1};
                    std::vector<int> field0 = strToBinaryVec(code.at("field0"), 3);
                    bi.insert(bi.end(), field0.begin(), field0.end());
                    std::vector<int> field1 = strToBinaryVec(code.at("field1"), 3);
                    bi.insert(bi.end(), field1.begin(), field1.end());
                    for(int i=0; i<16; i++){
                        bi.push_back(0);
                    }
                }
                // check if instruction field is halt
                else if (code.at("instruction") == "halt") return 25165824; 
                // check if instruction field is noop
                else if (code.at("instruction") == "noop") return 29360128;
                // instruction field is .fill
                else {
                    if(isNumber(code.at("field0"))){
                        return stringToInt(code.at("field0"));
                    }
                    checkLebel(code.at("field0"), stringToInt(code.at("address")));
                    return label.at(code.at("field0"));
                }

                int result = 0;
                int b = 1;
                // transform vector of binary in int
                for(int i=bi.size(); i>0; i--){
                    result += bi[i-1]*b;
                    b *= 2;
                }
                
                return result;
            }

    };
}

class Assembler{
    private:
        Pass1 pass1;
        Pass2 pass2;
        // variable of string in every line
        std::vector<std::string> str_file;
        // variable of assembly's format in every line
        std::vector<std::map<std::string, std::string>> pass1_result;

        // transform str_file to pass1_result by using pass1
        void pass1Tranform(){
            for(int i=0; i<str_file.size(); i++){
                pass1.setLine(str_file[i]);
                pass1_result.push_back(pass1.compute());
            }
        }

        // transform pass1_result of int by using pass2 and write it in to file
        void pass2AndWrite(){
            std::ofstream file("machine.txt");
            if (!file.is_open()) {
                std::cerr << "Error: Can't Open File" << std::endl;
                exit(1);
            }
            for(int i=0; i<pass1_result.size(); i++){
                if(pass1_result[i].empty()) continue;
                pass2.setCode(pass1_result[i]);
                file << pass2.compute() << std::endl;
            }
            file.close();
        }
    
    public:
        Assembler(std::string filename){
            std::ifstream file(filename);
            // check if file can open?
            if (!file.is_open()){
                std::cerr << "Error: File not found" << std::endl;
                exit(1);
            }
            std::string str_line;
            // push string of each line in str_file
            while (std::getline(file, str_line)) {
                str_file.push_back(str_line);
            }
            file.close();
            pass1Tranform();
            pass2AndWrite();
            exit(0);
        }
};