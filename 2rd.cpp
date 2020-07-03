#define export exports
extern "C" {
	#include "qbe/all.h"
}
#undef export

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

struct Block
{
	string name;
	set<string> var, Gen, Kill, In, Out;
	uint npred;
	Blk **pred;
};

static vector<Block> BuildVar(Fn *fn)
{
	vector<Block> blocks;
	for (Blk *blk = fn->start; blk; blk = blk->link)
	{
		Block block;
		block.name = blk->name;
		block.npred = blk->npred;
		block.pred = blk->pred;
		for(uint i = 0; i < blk->nins; ++i)
		{
			if(Tmp0 <= (blk->ins[i]).to.val)
			{
				block.var.insert(fn->tmp[(blk->ins[i]).to.val].name);
			}
		}
		blocks.push_back(block);
	}
	return blocks;
}

static void BuildGenKill(vector<Block>& blocks)
{
	for(uint i = 0; i < blocks.size(); ++i)
	{
		for(uint j = 0; j < blocks.size(); ++j)
		{
			if(i != j)
			{
				for(set<string>::const_iterator it = blocks[i].var.begin();
					it != blocks[i].var.end(); it++)
				{
					if(blocks[j].var.find(*it) != blocks[j].var.end())
					{
						blocks[i].Kill.insert("@"+blocks[j].name+"%"+(*it));
					}
				}
			}
		}
		for(set<string>::const_iterator it = blocks[i].var.begin();
			it != blocks[i].var.end(); it++)
		{
			blocks[i].Gen.insert("@"+blocks[i].name+"%"+(*it));
		}
	}
}

static void PrintReachDef(vector<Block> blocks)
{
	for(uint i = 0; i < blocks.size(); ++i)
	{
		cout << "@" << blocks[i].name;
		cout << "\n\trd_in = ";
		for(set<string>::const_iterator it = blocks[i].In.begin();
			it != blocks[i].In.end(); it++)
		{
			cout << (*it) << " ";
		}
		if(i != blocks.size() - 1)
		{
			cout << "\n\n";
		}
	}
}

static void BuildInOut(Fn *fn, vector<Block>& blocks)
{
	int changed = 1;
	while(changed)
	{
		changed = 0;
		for(uint i = 0; i < blocks.size(); ++i)
		{
			set<string> InWithoutKill;
			set_difference(
				blocks[i].In.begin(), blocks[i].In.end(),
				blocks[i].Kill.begin(), blocks[i].Kill.end(),
				inserter(InWithoutKill, InWithoutKill.begin()));
			set<string> NewOut;
			set_union(
				InWithoutKill.begin(), InWithoutKill.end(),
				blocks[i].Gen.begin(), blocks[i].Gen.end(),
				inserter(NewOut, NewOut.begin()));
			blocks[i].In.clear();
			for(uint j = 0; j < blocks[i].npred; ++j)
			{
				for(uint k = 0; k < blocks.size(); ++k)
				{
					if(!blocks[k].name.compare(blocks[i].pred[j]->name))
					{
						for(set<string>::const_iterator it = blocks[k].Out.begin();
							it != blocks[k].Out.end(); it++)
						{
							blocks[i].In.insert(*it);
						}
					}
				}
			}
			if(!(NewOut == blocks[i].Out))
			{
				blocks[i].Out.clear();
				blocks[i].Out = NewOut;
				changed = 1;
			}			
		}
	}
}

static void readfn (Fn *fn)
{
	vector<Block> blocks = BuildVar(fn);
	BuildGenKill(blocks);
	BuildInOut(fn, blocks);
	PrintReachDef(blocks);
}

static void readdat (Dat *dat) {
	(void) dat;
}

int main () {
	parse(stdin, "<stdin>", readdat, readfn);
	freeall();
}
