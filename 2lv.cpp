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
	set<string> Def, Use, In, Out;
	Blk *s1, *s2;
};

static vector<Block> BuildDefUse(Fn *fn)
{
	vector<Block> blocks;
	for (Blk *blk = fn->start; blk; blk = blk->link)
	{
		Block block;
		block.name = blk->name;
		block.s1 = blk->s1;
		block.s2 = blk->s2;
		for(uint i = 0; i < blk->nins; ++i)
		{
			for(uint j = 0; j < 2; ++j)
			{
				Ref arg = (blk->ins[i]).arg[j];
				if(Tmp0 <= (arg.val))
				{
					if(block.Def.find(fn->tmp[arg.val].name) == block.Def.end())
					{
						block.Use.insert(fn->tmp[arg.val].name);
					}
				}
			}
			Ref to = (blk->ins[i]).to;
			if(Tmp0 <= (to.val))
			{
				block.Def.insert(fn->tmp[to.val].name);
			}
		}
		if(blk->s1 == NULL && blk->s2 == NULL)
		{
			if(Tmp0 <= blk->jmp.arg.val)
			{
				if(block.Def.find(fn->tmp[blk->jmp.arg.val].name) == block.Def.end())
				{
					block.Use.insert(fn->tmp[blk->jmp.arg.val].name);
				}
			}
		}
		blocks.push_back(block);
	}
	return blocks;
}

static void BuildInOut(Fn *fn, vector<Block>& blocks)
{
	int changed = 1;
	while(changed)
	{
		changed = 0;
		for(uint i = 0; i < blocks.size(); ++i)
		{
			blocks[i].Out.clear();
			if(blocks[i].s1 != NULL)
			{
				for(uint k = 0; k < blocks.size(); ++k)
				{
					if(!blocks[k].name.compare(blocks[i].s1->name))
					{
						for(set<string>::const_iterator it = blocks[k].In.begin();
							it != blocks[k].In.end(); it++)
						{
							blocks[i].Out.insert(*it);
						}
					}
				}
			}
			if(blocks[i].s2 != NULL)
			{
				for(uint k = 0; k < blocks.size(); ++k)
				{
					if(!blocks[k].name.compare(blocks[i].s2->name))
					{
						for(set<string>::const_iterator it = blocks[k].In.begin();
							it != blocks[k].In.end(); it++)
						{
							blocks[i].Out.insert(*it);
						}
					}
				}
			}
			set<string> OutWithoutDef;
			set_difference(
				blocks[i].Out.begin(), blocks[i].Out.end(),
				blocks[i].Def.begin(), blocks[i].Def.end(),
				inserter(OutWithoutDef, OutWithoutDef.begin()));
			set<string> NewIn;
			set_union(
				OutWithoutDef.begin(), OutWithoutDef.end(),
				blocks[i].Use.begin(), blocks[i].Use.end(),
				inserter(NewIn, NewIn.begin()));
			if(!(NewIn == blocks[i].In))
			{
				blocks[i].In.clear();
				blocks[i].In = NewIn;
				changed = 1;
			}			
		}
	}
}

static void PrintLiveVar(vector<Block> blocks)
{
	for(uint i = 0; i < blocks.size(); ++i)
	{
		cout << "@" << blocks[i].name;
		cout << "\n\tlv_out = ";
		for(set<string>::const_iterator it = blocks[i].Out.begin();
			it != blocks[i].Out.end(); it++)
		{
			cout << "%" << (*it) << " ";
		}
		if(i != blocks.size() - 1)
		{
			cout << "\n\n";
		}
	}
}

static void readfn (Fn *fn)
{
	vector<Block> blocks = BuildDefUse(fn);
	BuildInOut(fn, blocks);
	PrintLiveVar(blocks);
}

static void readdat (Dat *dat) {
	(void) dat;
}

int main () {
	parse(stdin, "<stdin>", readdat, readfn);
	freeall();
}
