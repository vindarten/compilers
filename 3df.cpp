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
	set<string> dom, dom_front;
};

static void PrintDom(vector<Block> blocks)
{
	for(uint i = 0; i < blocks.size(); ++i)
	{
		cout << "@" << blocks[i].name << ":\t";
		for(set<string>::const_iterator it = blocks[i].dom_front.begin();
			it != blocks[i].dom_front.end(); it++)
		{
			cout << "@" << (*it) << " ";
		}
		cout << "\n";
	}
}

static void BuildAllWaysRec(
	Blk *cur_blk,
	Blk *blk,
	vector<set<string> >& AllWays,
	set<string> cur_way
)
{
	if(cur_way.find(cur_blk->name) != cur_way.end())
	{
		return;
	}
	cur_way.insert(cur_blk->name);
	if(cur_blk == blk)
	{
		AllWays.push_back(cur_way);
	}
	else
	{
		if(cur_blk->s1 != NULL)
		{
			BuildAllWaysRec(cur_blk->s1, blk, AllWays, cur_way);
		}
		if(cur_blk->s2 != NULL)
		{
			BuildAllWaysRec(cur_blk->s2, blk, AllWays, cur_way);
		}
	}
}

static void BuildAllWays(Fn *fn, Blk *blk, vector<set<string> >& AllWays)
{
	set<string> cur_way;
	BuildAllWaysRec(fn->start, blk, AllWays, cur_way);
}

static set<string>& GetDom(Blk *blk, vector<Block> Doms)
{
	for(uint i = 0; i < Doms.size(); ++i)
	{
		if(!Doms[i].name.compare(blk->name))
		{
			return Doms[i].dom;
		}
	}
}

static void readfn (Fn *fn)
{
	vector<Block> blocks;
	for (Blk *blk = fn->start; blk; blk = blk->link)
	{
		vector<set<string> > AllWays;
		BuildAllWays(fn, blk, AllWays);
		set<string> dom = AllWays[0];
		for(uint i = 1; i < AllWays.size(); ++i)
		{
			set<string> inter;
			set_intersection(
				dom.begin(), dom.end(),
				(AllWays[i]).begin(), (AllWays[i]).end(),
				inserter(inter, inter.begin()));
			dom = inter;
		}
		Block block;
		block.name = blk->name;
		block.dom = dom;
		blocks.push_back(block);
	}
	for (Blk *blk = fn->start; blk; blk = blk->link)
	{
		set<string> cur_dom = GetDom(blk, blocks);
		cur_dom.erase(blk->name);
		for(uint i = 0; i < blk->npred; ++i)
		{
			set<string> pred_dom = GetDom(blk->pred[i], blocks);
			set<string> differ;
			set_difference(
				pred_dom.begin(), pred_dom.end(),
				cur_dom.begin(), cur_dom.end(),
				inserter(differ, differ.begin()));
			for(set<string>::const_iterator it = differ.begin(); it != differ.end(); it++)
			{
				for(uint i = 0; i < blocks.size(); ++i)
				{
					if(!blocks[i].name.compare((*it)))
					{
						blocks[i].dom_front.insert(blk->name);
					}
				}
			}
		}
	}
	PrintDom(blocks);
}

static void readdat (Dat *dat) {
	(void) dat;
}

int main () {
	parse(stdin, "<stdin>", readdat, readfn);
	freeall();
}
