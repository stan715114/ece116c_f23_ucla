#include "CPU.h"

instruction::instruction(bitset<32> fetch)
{
	//cout << fetch << endl;
	instr = fetch;
	//cout << instr << endl;
}

CPU::CPU()
{
	PC = 0; //set PC to 0
	for (int i = 0; i < 4096; i++) //copy instrMEM
	{
		dmemory[i] = (0);
	}
}

bitset<32> CPU::Fetch(bitset<8> *instmem) {
	bitset<32> instr = ((((instmem[PC + 3].to_ulong()) << 24)) + ((instmem[PC + 2].to_ulong()) << 16) + ((instmem[PC + 1].to_ulong()) << 8) + (instmem[PC + 0].to_ulong()));  //get 32 bit instruction
	PC += 4;//increment PC
	instr_count++;
	return instr;
}


bool CPU::Decode(instruction* curr)
{
	bitset<32> opCode = curr->instr & bitset<32> (0x7f);
	bitset<32> func7 = (curr->instr >> 25) & bitset<32> (0x7f);
	bitset<32> func3 = (curr->instr >> 12) & bitset<32> (0x7);
	bitset<32> rs1 = (curr->instr >> 15) & bitset<32> (0x1f);
	bitset<32> rs2 = (curr->instr >> 20) & bitset<32> (0x1f);
	bitset<32> rd = (curr->instr >> 7) & bitset<32> (0x1f);
	bitset<32> i_imm = (curr->instr >> 20) | bitset<32>(-((curr->instr & bitset<32>(0x80000000)) >> 20).to_ulong());
	bitset<32> lw_imm = i_imm;
	bitset<32> sw_imm = ((i_imm >>5) << 5).to_ulong() + rd.to_ulong();
	bitset<32> b_imm = (((curr->instr >> 31) & bitset<32> (0x1)).to_ulong() << 12) + (((curr->instr >> 7) & bitset<32> (0x1)).to_ulong() << 11) + (((curr->instr >> 25) & bitset<32> (0x3f)).to_ulong() << 10) + (((curr->instr >> 8) & bitset<32> (0xf)).to_ulong() << 1);
	b_imm = b_imm | bitset<32>(-(b_imm & bitset<32>(0x00001000)).to_ulong()); 
	bitset<32> j_imm = i_imm;

	 
	if (opCode == rType)
	{
		r_type_count++;
		deck.rs1 = reg[(int)(rs1.to_ulong())];
		deck.rs2 = reg[(int)(rs2.to_ulong())]; 
		deck.rd = rd;

		if (func7 == bitset<32> (0x0))
		{
			if (func3 == bitset<32> (0x0)) {deck.type = TYPE::ADD;}
			else if (func3 == bitset<32> (0x4)) {deck.type = TYPE::XOR;}
			else {cout << "invalid R-type func3" << endl;}
		}
		else if (func7 == bitset<32> (0x20)) 
		{
			if (func3 == bitset<32> (0x0)) {deck.type = TYPE::SUB;}
			else if (func3 == bitset<32> (0x5)) {deck.type = TYPE::SRA;}
			else {cout << "invalid R-type func3" << endl;}
		}
		else 
		{
			cout << "invalid func7" << endl;
		}
	}
	else if (opCode == iType)
	{
		deck.rs1 = reg[(int)(rs1.to_ulong())]; 
		deck.rd = rd;
		deck.imm = i_imm;

		if (func3 == bitset<32> (0x0)) {deck.type = TYPE::ADDI;}
		else if (func3 == bitset<32> (0x7)) {deck.type = TYPE::ANDI;}
		else {cout << "invalid I-type func3" << endl;}
	}
	else if (opCode == loadW)
	{
		deck.rs1 = reg[(int)(rs1.to_ulong())]; 
		deck.rd = rd;
		deck.imm = lw_imm;
		deck.type = TYPE::LW;
	}
	else if (opCode == storeW)
	{
		deck.rs1 = reg[(int)(rs1.to_ulong())];  
		deck.rs2 = reg[(int)(rs2.to_ulong())]; 
		deck.imm = sw_imm;
		deck.type = TYPE::SW;
	}
	else if (opCode == blT)
	{
		deck.rs1 = reg[(int)(rs1.to_ulong())]; 
		deck.rs2 = reg[(int)(rs2.to_ulong())]; 
		deck.imm = b_imm;
		deck.type = TYPE::BLT;
	}
	else if (opCode == jalR)
	{
		deck.rs1 = reg[(int)(rs1.to_ulong())]; 
		deck.rd = rd;
		deck.imm = j_imm;
		deck.type = TYPE::JALR;
	}
	else
	{
		return false;
	}

	return true;
}

unsigned long CPU::readPC()
{
	return PC;
}

void CPU::Execute()
{
	switch (deck.type)
	{
	case TYPE::ADD: // R-type
		alu.aluResult = (deck.rs1).to_ulong() + (deck.rs2).to_ulong();
		alu.lessThan = (int)(((deck.rs1).to_ulong() - (deck.rs2).to_ulong()) >> 31);
		alu.ZERO = ((deck.rs1 ^ deck.rs2) == bitset<32>(0x0)) ? 1 : 0;
		break;
	case TYPE::SUB: // R-type
		alu.aluResult = (deck.rs1).to_ulong() - (deck.rs2).to_ulong();
		alu.lessThan = (int)(((deck.rs1).to_ulong() - (deck.rs2).to_ulong()) >> 31);
		alu.ZERO = ((deck.rs1 ^ deck.rs2) == bitset<32>(0x0)) ? 1 : 0;
		break;
	case TYPE::XOR: // R-type
		alu.aluResult = deck.rs1 ^ deck.rs2;
		alu.lessThan = (int)(((deck.rs1).to_ulong() - (deck.rs2).to_ulong()) >> 31);
		alu.ZERO = ((deck.rs1 ^ deck.rs2) == bitset<32>(0x0)) ? 1 : 0;
		break;
	case TYPE::SRA: // R-type
		alu.aluResult = (deck.rs1 >> (deck.rs2).to_ulong()) |  bitset<32> ( - ((deck.rs1 & bitset<32>(0x80000000)) >> (deck.rs2).to_ulong()).to_ulong());
		alu.lessThan = (int)(((deck.rs1).to_ulong() - (deck.rs2).to_ulong()) >> 31);
		alu.ZERO = ((deck.rs1 ^ deck.rs2) == bitset<32>(0x0)) ? 1 : 0;
		break;
	case TYPE::ADDI: // I-type
		alu.aluResult = (deck.rs1).to_ulong() + (deck.imm).to_ulong();
		break;
	case TYPE::ANDI: // I-type
		alu.aluResult = deck.rs1 & deck.imm;
		break;
	case TYPE::LW: // LoadWord
		alu.aluResult = (deck.rs1).to_ulong() + (deck.imm).to_ulong();
		break;
	case TYPE::SW: // StoreWord
		alu.aluResult = (deck.rs1).to_ulong() + (deck.imm).to_ulong();
		break;
	case TYPE::BLT: // Branch
		alu.lessThan = (int)((((deck.rs1).to_ulong() - (deck.rs2).to_ulong()) >> 31) & (1));
		alu.ZERO = ((deck.rs1 ^ deck.rs2) == bitset<32>(0x0)) ? 1 : 0;
		break;
	case TYPE::JALR: // Jump
		alu.aluResult = (deck.rs1).to_ulong() + (deck.imm).to_ulong(); 
		break;
	case TYPE::ZERO:
		break;
	}

}

void CPU::Memory()
{
	bitset<8> byte1, byte2, byte3, byte4;

	switch (deck.type)
	{
	case TYPE::LW:
		byte4 = dmemory[(int)(alu.aluResult).to_ulong()];  
		byte3 = dmemory[(int)(alu.aluResult).to_ulong() + 1]; 
		byte2 = dmemory[(int)(alu.aluResult).to_ulong() + 2]; 
		byte1 = dmemory[(int)(alu.aluResult).to_ulong() + 3];  

		lw_dum = (byte1.to_ulong() << 24) + (byte2.to_ulong() << 16) + (byte3.to_ulong() << 8) + (byte4).to_ulong();
		
		break;
	case TYPE::SW:
		byte1 = bitset<8> (((deck.rs2 >> 24) & bitset<32>(0xff)).to_ulong());
		byte2 = bitset<8>(((deck.rs2 >> 16) & bitset<32>(0xff)).to_ulong());
		byte3 = bitset<8>(((deck.rs2 >> 8) & bitset<32>(0xff)).to_ulong());
		byte4 = bitset<8>(((deck.rs2) & bitset<32>(0xff)).to_ulong());

		dmemory[(int)(alu.aluResult).to_ulong()] = byte4;
		dmemory[(int)(alu.aluResult).to_ulong() + 1] = byte3;
		dmemory[(int)(alu.aluResult).to_ulong() + 2] = byte2;
		dmemory[(int)(alu.aluResult).to_ulong() + 3] = byte1;
		break;
	default:
		break;
	}
}

void CPU::WriteBack()
{

	if (deck.type == TYPE::SW) {return;}
	else if (deck.type == TYPE::JALR) {
		reg[(int)(deck.rd).to_ulong()] = PC;
		PC = (int)(alu.aluResult).to_ulong();
	}
	else if (deck.type == TYPE::BLT) {
		if (alu.lessThan == 1) {PC = PC + (int)(deck.imm).to_ulong() - 4;}
		else { return; }
	}
	else if (deck.type == TYPE::LW)
		reg[(int)(deck.rd).to_ulong()] = lw_dum;
	else if ((deck.type != TYPE::ZERO))
		reg[(int)(deck.rd).to_ulong()] = alu.aluResult;

	cycle_count++;
}

void CPU::PrintStage()
{
	cout << "(" << (int)reg[10].to_ulong() << "," << (int)reg[11].to_ulong() << ")" << endl;
	//cout << "Total number of cycle: " << instr_count << endl;
	//cout << "Number of R-type instructions: " << r_type_count << endl;
	//cout << "IPC: " << (instr_count / cycle_count) << endl;
}
