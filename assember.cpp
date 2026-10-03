#include <iostream>
#include <cstdlib>
#include <set>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <sstream>

namespace {
    // instruction set
    const std::set<std::string> ins_set = {"add", "nand", "lw", "sw", "beq", "jalr", "halt", "noop", ".fill"};
    std::map<std::string, int> label;

    // split string to vecter
    std::list<std::string> split(std::string str) {
        std::stringstream ss(str);
        std::string segment;
        std::list<std::string> seglist;

        while (ss >> segment) {
            seglist.push_back(segment);
        }
        return seglist;
    }

    class Analysis{
        private:    
            std::string line;
            int line_count = 0;

        public:
            Analysis() {}

            void setLine(std::string str) {
                this->line = str;
                line_count++;
            }

            std::map<std::string, std::string> analyze() {
                std::map<std::string, std::string> result;
                std::list<std::string> list = split(line);
                if(list.empty()) {return result;}
                std::vector<std::string> format = {"label", "instruction", "field0", "field1", "field2"};
                std::string op;
                int size = 0;
                if(ins_set.count(list.front())){
                    op = list.front();
                    result.insert({format[1], op});
                    list.pop_front();
                }
                else{
                    if(label.find(list.front()) != label.end()) {
                        std::cerr << "Error: Duplicate label " << list.front() << " in line " << line_count << std::endl;
                        exit(1);
                    }
                    result.insert({format[0], list.front()});
                    label.insert({list.front(), line_count});
                    list.pop_front();
                    if(list.empty()) {return result;}

                    if(ins_set.count(list.front())){
                        op = list.front();
                        result.insert({format[1], op});
                        list.pop_front();
                    }
                    else{
                        std::cerr << "Error: Unknown opcode " << list.front() << " in line " << line_count << std::endl;
                        exit(1);
                    }

                }

                if(op == "add" || op == "nand" || op == "lw" || op == "sw" || op == "beq"){size = 3;}
                else if(op == "jalr"){size = 2;}
                else if(op == ".fill"){size = 1;}
                
                for(int i=0; i<size; i++){
                    if(list.empty()) {
                        std::cerr << "Error: missing field for instruction " << op << " in line " << line_count << std::endl;
                        exit(1);
                    }
                    result.insert({format[i+2], list.front()});
                    list.pop_front();
                }
                return result;
            }
    };

    class Synthesis{};
}

class assembler{
    
};