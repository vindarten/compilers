#ifdef __cplusplus
#define export exports
extern "C" {
#include "qbe/all.h"
}
#undef export
#else
#include <qbe/all.h>
#endif

#include <stdio.h>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

struct Block
{
	Blk *blk;
	set<Blk*> dom, rdf;
};

static void PrintDom(vector<Block> blocks)
{
	for(uint i = 0; i < blocks.size(); ++i)
	{
		cout << "@" << blocks[i].blk->name << "\n\t Doms: ";
		for(set<Blk*>::const_iterator it = blocks[i].dom.begin();
			it != blocks[i].dom.end(); it++)
		{
			cout << "@" << (*it)->name << " ";
		}
		cout << "\n\t Rdf: ";
		for(set<Blk*>::const_iterator it = blocks[i].rdf.begin();
			it != blocks[i].rdf.end(); it++)
		{
			cout << "@" << (*it)->name << " ";
		}
		cout << "\n";
	}
}

static void BuildAllWaysRec(
	Blk *cur_blk,
	Blk *blk,
	vector<set<Blk*> >& AllWays,
	set<Blk*> cur_way
)
{
	if(cur_way.find(cur_blk) != cur_way.end())
	{
		return;
	}
	cur_way.insert(cur_blk);
	if(cur_blk->s1 != NULL)
	{
		BuildAllWaysRec(cur_blk->s1, blk, AllWays, cur_way);
	}
	if(cur_blk->s2 != NULL)
	{
		BuildAllWaysRec(cur_blk->s2, blk, AllWays, cur_way);
	}
	if(cur_blk->s1 == NULL && cur_blk->s2 == NULL)
	{
		AllWays.push_back(cur_way);
	}
}


static void BuildAllWays(Blk *blk, vector<set<Blk*> >& AllWays)
{
	set<Blk*> cur_way;
	BuildAllWaysRec(blk, blk, AllWays, cur_way);
}

static vector<Block> CreateBlocksWithDom(Fn *fn) 
{
	vector<Block> blocks;
	for(Blk *blk = fn->start; blk; blk = blk->link)
	{
		vector<set<Blk*> > AllWays;
		BuildAllWays(blk, AllWays);
		set<Blk*> dom = AllWays[0];
		for(uint i = 1; i < AllWays.size(); ++i)
		{
			set<Blk*> inter;
			set_intersection(
				dom.begin(), dom.end(),
				(AllWays[i]).begin(), (AllWays[i]).end(),
				inserter(inter, inter.begin()));
			dom = inter;
		}
		Block block;
		block.blk = blk;
		block.dom = dom;
		blocks.push_back(block);
	}
	return blocks;
}

static set<Blk*>& GetDom(Blk *blk, vector<Block> blocks)
{
	for(uint i = 0; i < blocks.size(); ++i)
	{
		if(blocks[i].blk == blk)
		{
			return blocks[i].dom;
		}
	}
}

static void AddBlkToRdf(vector<Block>& blocks, Blk* cur, Blk* pred)
{
	set<Blk*> cur_dom = GetDom(cur, blocks);
	cur_dom.erase(cur);
	set<Blk*> pred_dom = GetDom(pred, blocks);
	set<Blk*> differ;
	set_difference(
		pred_dom.begin(), pred_dom.end(),
		cur_dom.begin(), cur_dom.end(),
		inserter(differ, differ.begin()));
	for(set<Blk*>::const_iterator it = differ.begin();
		it != differ.end(); it++)
	{
		for(uint i = 0; i < blocks.size(); ++i)
		{
			if(blocks[i].blk == (*it))
			{
				blocks[i].rdf.insert(cur);
			}
		}
	}
}

static void FillRdf(Fn *fn, vector<Block>& blocks)
{
	for(Blk *blk = fn->start; blk; blk = blk->link)
	{
		if(blk->s1 != NULL)
		{
			AddBlkToRdf(blocks, blk, blk->s1);
		}
		if(blk->s2 != NULL)
		{
			AddBlkToRdf(blocks, blk, blk->s2);
		}
	}
}

static void PrintAllIns(Fn *fn)
{
	for(Blk *blk = fn->start; blk; blk = blk->link)
	{
		cout << "@" << blk->name << "\n";
		for(uint i = 0; i < blk->nins; ++i)
		{
			cout << "\t" << optab[blk->ins[i].op].name << "\n";
		}
		Phi *phi = blk->phi;
		while(phi != NULL)
		{
			cout << "\tPhi:\t" << fn->tmp[phi->to.val].name << "\n";
			phi = phi->link;
		}
		cout << "\tJmp:\t" << blk->jmp.type << "\t";
		cout << fn->tmp[blk->jmp.arg.val].name << "\t";
		if(blk->s1 != NULL)
		{
			cout << blk->s1->name << "\t";
		}
		if(blk->s2 != NULL)
		{
			cout << blk->s2->name << "\t";
		}
		cout << "\n";
	}
}

struct Marked
{
	enum Type { MIns, MPhi, MJmp};
	Type type;
	Ins *ins;
	Phi *phi;
	Blk *blk;
};

static bool operator<(const Marked& m1, const Marked& m2)
{
	if(m1.type != m2.type)
	{
		return m1.type < m2.type;
	}
	else
	{
		if(m1.type == Marked::MIns)
		{
			if(m1.ins->op < m2.ins->op)
			{
				return true;
			}
			if(m1.ins->op == m2.ins->op)
			{
				if(m1.ins->to.val < m2.ins->to.val)
				{
					return true;
				}
				if(m1.ins->to.val == m2.ins->to.val)
				{
					if(m1.ins->arg[0].val < m2.ins->arg[0].val)
					{
						return true;
					}
					if(m1.ins->arg[0].val == m2.ins->arg[0].val)
					{
						if(m1.ins->arg[1].val < m2.ins->arg[1].val)
						{
							return true;
						}
						if(m1.ins->arg[1].val == m2.ins->arg[1].val)
						{
							return strcmp(m1.blk->name, m2.blk->name) < 0;
						}
					}
				}
			}
			return false;
		}
		else if(m1.type == Marked::MPhi)
		{
			return m1.phi->to.val < m2.phi->to.val ||
			m1.phi->to.val == m2.phi->to.val &&
			strcmp(m1.blk->name, m2.blk->name) < 0;
		}
		else if(m1.type == Marked::MJmp)
		{
			return strcmp(m1.blk->name, m2.blk->name) < 0;
		}
		return false;
	}
}

static void PrintMarked(Fn *fn, set<Marked>& WorkList)
{
	for(set<Marked>::const_iterator it = WorkList.begin();
		it != WorkList.end(); it++)
	{
		cout << "@" << (*it).blk->name << "\t";
		if((*it).type == Marked::MIns)
		{
			cout << optab[(*it).ins->op].name << "\n";
		}
		else if((*it).type == Marked::MJmp)
		{
			cout << "Jmp" << "\t";
			cout << fn->tmp[(*it).blk->jmp.arg.val].name << "\t";
			if((*it).blk->s1 != NULL)
			{
				cout << (*it).blk->s1->name << "\t";
			}
			if((*it).blk->s2 != NULL)
			{
				cout << (*it).blk->s2->name << "\t";
			}
			cout << "\n";
		}
		Phi *phi = (*it).blk->phi;
		while(phi != NULL)
		{
			cout << "\tPhi " << fn->tmp[phi->to.val].name << "\n";
			phi = phi->link;
		}
	}
	cout << "\n";
}

static Marked CreateMarkedIns(Ins *ins, Blk *blk)
{
	Marked marked;
	marked.type = Marked::MIns;
	marked.ins = ins;
	marked.phi = NULL;
	marked.blk = blk;
	return marked;
}

static Marked CreateMarkedPhi(Phi *phi, Blk *blk)
{
	Marked marked;
	marked.type = Marked::MPhi;
	marked.ins = NULL;
	marked.phi = phi;
	marked.blk = blk;
	return marked;
}

static Marked CreateMarkedJmp(Blk *blk)
{
	Marked marked;
	marked.type = Marked::MJmp;
	marked.ins = NULL;
	marked.phi = NULL;
	marked.blk = blk;
	return marked;
}

static void InitWorkList(Fn *fn, set<Marked>& WorkList)
{
	for(Blk *blk = fn->start; blk; blk = blk->link)
	{
		for(uint i = 0; i < blk->nins; ++i)
		{
			if(isstore(blk->ins[i].op) ||
				blk->ins[i].op == Ovacall ||
				blk->ins[i].op == Ocall ||
				isarg(blk->ins[i].op))
			{
				WorkList.insert(CreateMarkedIns(&(blk->ins[i]), blk));
			}
		}
		if(isret(blk->jmp.type))
		{
			WorkList.insert(CreateMarkedJmp(blk));
		}
	}
}

static Marked FindDef(Fn *fn, uint val)
{
	for(Blk *blk = fn->start; blk; blk = blk->link)
	{
		for(uint i = 0; i < blk->nins; ++i)
		{
			if(blk->ins[i].to.val == val)
			{
				return CreateMarkedIns(&(blk->ins[i]), blk);
			}
		}
		Phi *phi = blk->phi;
		while(phi != NULL)
		{
			if(phi->to.val == val)
			{
				return CreateMarkedPhi(phi, blk);
			}
			phi = phi->link;
		}
	}
}

static set<Blk*> GetRdf(vector<Block> blocks, Blk *blk)
{
	for(uint i = 0; i < blocks.size(); ++i)
	{
		if(blocks[i].blk == blk)
		{
			return blocks[i].rdf;
		}
	}
}

static set<Marked> Mark(Fn *fn, vector<Block> blocks)
{
	set<Marked> WorkList;
	InitWorkList(fn, WorkList);
	set<Marked> AllMarked(WorkList);
	while(!WorkList.empty())
	{
		Marked old = *WorkList.begin();
		if(old.type == Marked::MIns)
		{
			if(fn->tmp[old.ins->arg[0].val].name != NULL &&
				strcmp(fn->tmp[old.ins->arg[0].val].name, ""))
			{
				Marked marked = FindDef(fn, old.ins->arg[0].val);
				if(AllMarked.find(marked) == AllMarked.end())
				{
					AllMarked.insert(marked);
					WorkList.insert(marked);
				}
			}
			if(fn->tmp[old.ins->arg[1].val].name != NULL &&
				strcmp(fn->tmp[old.ins->arg[1].val].name, ""))
			{
				Marked marked = FindDef(fn, old.ins->arg[1].val);
				if(AllMarked.find(marked) == AllMarked.end())
				{
					AllMarked.insert(marked);
					WorkList.insert(marked);
				}
			}
		}
		else if(old.type == Marked::MJmp)
		{
			if(fn->tmp[old.blk->jmp.arg.val].name != NULL &&
				strcmp(fn->tmp[old.blk->jmp.arg.val].name, ""))
			{
				Marked marked = FindDef(fn, old.blk->jmp.arg.val);
				if(AllMarked.find(marked) == AllMarked.end())
				{
					AllMarked.insert(marked);
					WorkList.insert(marked);
				}
			}
		}
		else if(old.type == Marked::MPhi)
		{
			for(uint i = 0; i < old.phi->narg; ++i)
			{
				if(fn->tmp[old.phi->arg[i].val].name != NULL &&
					strcmp(fn->tmp[old.phi->arg[i].val].name, ""))
				{
					Marked marked = FindDef(fn, old.phi->arg[i].val);
					if(AllMarked.find(marked) == AllMarked.end())
					{
						AllMarked.insert(marked);
						WorkList.insert(marked);
					}
					Marked marked_block = CreateMarkedJmp(old.phi->blk[i]);
					if(AllMarked.find(marked_block) == AllMarked.end())
					{
						AllMarked.insert(marked_block);
						WorkList.insert(marked_block);
					}
				}
			}

		}
		set<Blk*> rdf = GetRdf(blocks, old.blk);
		for(set<Blk*>::const_iterator it = rdf.begin();
			it != rdf.end(); it++)
		{
			Marked marked = CreateMarkedJmp((*it));
			if(AllMarked.find(marked) == AllMarked.end())
			{
				AllMarked.insert(marked);
				WorkList.insert(marked);
			}
		}
		WorkList.erase(old);
	}
	return AllMarked;
}

static set<Blk*> GetDom(vector<Block> blocks, Blk *blk)
{
	for(uint i = 0; i < blocks.size(); ++i)
	{
		if(blocks[i].blk == blk)
		{
			return blocks[i].dom;
		}
	}
}

static Blk* GetNearPostDom(set<Marked> AllMarked, vector<Block> blocks, Blk *blk)
{
	set<Blk*> blk_dom = GetDom(blocks, blk);
	set<Blk*> will_be_del;
	for(set<Blk*>::const_iterator it = blk_dom.begin();
		it != blk_dom.end(); it++)
	{
		Marked marked = CreateMarkedJmp((*it));
		if(AllMarked.find(marked) == AllMarked.end())
		{
			will_be_del.insert(*it);
		}
	}
	set<Blk*> only_marked;
	set_difference(
		blk_dom.begin(), blk_dom.end(),
		will_be_del.begin(), will_be_del.end(),
		inserter(only_marked, only_marked.begin()));
	for(set<Blk*>::const_iterator it = only_marked.begin();
		it != only_marked.end(); it++)
	{
		set<Blk*> strict_copy = only_marked;
		strict_copy.erase(*it);
		int is_find = 0;
		for(set<Blk*>::const_iterator it2 = strict_copy.begin();
			it2 != strict_copy.end(); it2++)
		{
			set<Blk*> it2_dom = GetDom(blocks, (*it2));
			if(it2_dom.find(*it) != it2_dom.end())
			{
				is_find = 1;
				break;
			}
		}
		if(!is_find)
		{
			return (*it);
		}
	}
}

static void Sweep(Fn *fn, set<Marked> AllMarked, vector<Block> blocks)
{
	for(Blk *blk = fn->start; blk; blk = blk->link)
	{
		for(uint i = 0; i < blk->nins; ++i)
		{
			Marked marked = CreateMarkedIns(&(blk->ins[i]), blk);
			if(AllMarked.find(marked) == AllMarked.end())
			{
				blk->ins[i].op = Onop;
				blk->ins[i].to = R;
				blk->ins[i].arg[0] = R;
				blk->ins[i].arg[1] = R;
			}
		}
		Phi **phi = &(blk->phi);
		while(*phi != NULL)
		{
			Marked marked = CreateMarkedPhi(*phi, blk);
			if(AllMarked.find(marked) == AllMarked.end())
			{
				Phi *will_be_del = *phi;
				*phi = (*phi)->link;
			}
			else
			{
				phi = &( (*phi)->link );
			}
		}
		if(blk->s1 != NULL && blk->s2 != NULL)
		{
			Marked marked = CreateMarkedJmp(blk);
			if(AllMarked.find(marked) == AllMarked.end())
			{
				blk->jmp.type = Jjmp;
				blk->jmp.arg = R;
				blk->s1 = GetNearPostDom(AllMarked, blocks, blk);
				blk->s2 = NULL;
			}
		}
	}

}

static void readfn (Fn *fn)
{
    fillrpo(fn); // Traverses the CFG in reverse post-order, filling blk->id.
    fillpreds(fn);
    filluse(fn);
    ssa(fn);
	vector<Block> blocks = CreateBlocksWithDom(fn);
	FillRdf(fn, blocks);
	cout << "DOMS AND RDF\n";
	PrintDom(blocks);
	cout << "ALL INSTRUCTIONS\n";
	PrintAllIns(fn);
	set<Marked> AllMarked = Mark(fn, blocks);
	cout << "MARKED INSTRUCTIONS\n";
	PrintMarked(fn, AllMarked);
	Sweep(fn, AllMarked, blocks);
	fillpreds(fn); // Either call this, or keep track of preds manually when rewriting branches.
    fillrpo(fn); // As a side effect, fillrpo removes any unreachable blocks.
    printfn(fn, stdout);
}

static void readdat (Dat *dat)
{
  (void) dat;
}

int main ()
{
  parse(stdin, "<stdin>", readdat, readfn);
  freeall();
}
