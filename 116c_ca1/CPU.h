#include <iostream>
#include <bitset>
#include <stdio.h>
#include<stdlib.h>
#include <string>
using namespace std;

#define rType bitset<32> (0x33)
#define iType bitset<32> (0x13)
#define loadW bitset<32> (0x3)
#define storeW bitset<32> (0x23)
#define blT bitset<32> (0x63)
#define jalR bitset<32> (0x67)

enum class TYPE
{
	ADD, SUB, XOR, SRA, ADDI, ANDI, LW, SW, BLT, JALR, ZERO,
};

class instruction {
public:
	bitset<32> instr;//instruction
	instruction(bitset<32> fetch); // constructor

};

class CPU {
private:
	bitset<8> dmemory[4096]; //data memory byte addressable in little endian fashion;
	unsigned long PC; //pc 
	bitset<32> reg[32]; // 32 register
	bitset<32> lw_dum;

	int r_type_count = 0;
	double instr_count = 0;
	double cycle_count;


	struct decisionModule {
		bitset<32> rs1 = 0;
		bitset<32> rs2 = 0;
		bitset<32> rd = 0;
		bitset<32> imm = 0;
		TYPE type = TYPE::ZERO;
	}deck;

	struct aluModule {
		bitset<32> aluResult = 0; 
		int lessThan = 0; // rs1 < rs2, lessThan = MSB = 1;
		int ZERO = 0; // rs1 = rs2, ZERO = 1, otherwise = 0;

	}alu;

public:
	CPU();
	unsigned long readPC();
	bitset<32> Fetch(bitset<8> *instmem);
	bool Decode(instruction* instr);
	void Execute();
	void Memory();
	void WriteBack();
	void PrintStage();
};

// add other functions and objects here
