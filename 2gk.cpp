#define export exports
extern "C" {
	#include "qbe/all.h"
}
#undef export

#include <iostream>
#include <vector>

using namespace std;

struct Defin
{
	string block_name, var_name;
};

struct Block
{
	string name;
	vector<string> gen;
	vector<Defin> kill;
};

static vector<Block> BuildGen(Fn *fn)
{
	vector<Block> blocks;
	for (Blk *blk = fn->start; blk; blk = blk->link)
	{
		Block block;
		block.name = blk->name;
		uint repeat[fn->ntmp];
		for(int i = 0; i < fn->ntmp; ++i)
		{
			repeat[i] = 0;
		}
		for(uint i = 0; i < blk->nins; ++i)
		{
			if(!repeat[(blk->ins[i]).to.val] && Tmp0 <= (blk->ins[i]).to.val)
			{
				block.gen.push_back(fn->tmp[(blk->ins[i]).to.val].name);
				repeat[(blk->ins[i]).to.val] = 1;
			}
		}
		blocks.push_back(block);
	}
	return blocks;
}

static void BuildKill(vector<Block>& blocks)
{
	for(uint i = 0; i < blocks.size(); ++i)
	{
		for(uint j = 0; j < blocks.size(); ++j)
		{
			if(i != j)
			{
				for(uint k = 0; k < blocks[i].gen.size(); ++k)
				{
					for(uint l = 0; l < blocks[j].gen.size(); ++l)
					{
						if(!blocks[i].gen[k].compare(blocks[j].gen[l]))
						{
							Defin defin;
							defin.block_name = blocks[j].name;
							defin.var_name = blocks[j].gen[l];
							blocks[i].kill.push_back(defin);
						}
					}
				}
			}
		}
	}
}

static void PrintGenKill(vector<Block> blocks)
{
	for(uint i = 0; i < blocks.size(); ++i)
	{
		cout << "@" << blocks[i].name;
		cout << "\n\tgen = ";
		for(uint j = 0; j < blocks[i].gen.size(); ++j)
		{
			cout << "@" << blocks[i].name << "%" << blocks[i].gen[j];
			if(j != blocks[i].gen.size() - 1)
			{
				cout << " ";
			}
		}
		cout << "\n\tkill = ";
		for(uint j = 0; j < blocks[i].kill.size(); ++j)
		{
			cout << "@" << blocks[i].kill[j].block_name << "%" << blocks[i].kill[j].var_name;
			if(j != blocks[i].kill.size() - 1)
			{
				cout << " ";
			}
		}

		if(i != blocks.size() - 1)
		{
			cout << "\n\n";
		}
	}
}

static void readfn (Fn *fn)
{
	vector<Block> blocks = BuildGen(fn);
	BuildKill(blocks);
	PrintGenKill(blocks);
}

static void readdat (Dat *dat) {
	(void) dat;
}

int main () {
	parse(stdin, "<stdin>", readdat, readfn);
	freeall();
}
