/*=========================================================*
*	______ _           _           _           _		   *
*	|  ___(_)         | |         | |         | |		   *
*	| |_   _ _ __ ___ | |__  _   _| |_   _____| |_ _ __    *
*	|  _| | | '_ ` _ \| '_ \| | | | \ \ / / _ \ __| '__|   *
*	| |   | | | | | | | |_) | |_| | |\ V /  __/ |_| |      *
*	\_|   |_|_| |_| |_|_.__/ \__,_|_| \_/ \___|\__|_|      *
* -------------------------------------------------------- *
*               An Ragnarok Online Emulator                *
* -------------------------------------------------------- *
*                Licenced under GNU GPL v3                 *
* -------------------------------------------------------- *
*				  Data Structures Classes			       *
* ======================================================== */


#include <show_message.hpp>
#include <core.hpp>
#include <timers.hpp>
#include <iostream>
#include <cassert>
#include <strfuncs.hpp>

using namespace std;

/*==============================================================*
* Class: RadixNode												*                                                     
* Author: Minos	                                                *
* Date: 14/12/11 												*
* Description: The node of a Radix Tree.                        *
* Note: This tree is based in a Java RadixTree Model that i     *
* found in google. Thanks to RKA for the base code.             *
**==============================================================*/
template< unsigned int mapid , string server_mapname , string client_mapname >
class RadixNode {


	// This typedef makes the code more legible. Because i used a lot of RadixNode<bla bla bla> in the code.
    typedef typename RadixNode<mapid,server_mapname,client_mapname> RTreeNode;

	private:
		friend class RadixTree; // The Main Tree Class.
		int bitpos;
		mapid SMapid; // The Map ID in the server
		// I Created 2 Map names to make the Instance Maps easier. Thanks to Vianna for this tip.
		server_mapname SMapname; // The Map Name to the server
		client_mapname CMapname; // The Map Name to the client
		// The Tree Branches.
		RTreeNode* rightbranch;
		RTreeNode* leftbranch;

	public:
		
		// RadixTree Constructor and Destructor
		RadixNode( mapid SMapid, server_mapname SMapname, client_mapname CMapName , int bitpos, RTreeNode* rightbranch, RTreeNode* leftbranch ){

			StartNode( this.SMapid , this.SMapname , this.CMapName , this.bitpos , this.rightbranch , this.leftbranch );

		}
			
		~RadixNode();
		
		// A Node Starter
		void StartNode (mapid SMapid, server_mapname SMapname, client_mapname CMapname, int bitpos , RTreeNode* leftbranch, RTreeNode* rightbranch ){

				SMapid = this.SMapid;
				SMapname = this.SMapname;
				CMapName = this.CMapName;
				leftbranch = this.leftbranch;
				rightbranch = this.rightbranch;
				bitpos = this.bitpos;
		
		}

        
		// Getters and Setters, the modules name tell what they do :P
		server_mapname GetServer_Mapname(){

			return SMapname;
		
		}

		client_mapname GetClient_Mapname(){

			return CMapname;

		}


		bool Set_mapname(server_mapname SMapname, client_mapname CMapname){

			SMapname = this.SMapname;
			CMapname = this.CMapname;
			return true;

        }
	    
		mapid Get_Mapid() {
				
			return SMapid;
		
		}
		
		RTreeNode* GetRightBranch(){

			return rightbranch;

		}

		void SetRightBranch( RTreeNode* SetBranch ){

			rightbranch = SetBranch;

		}
	    
		RTreeNode* GetLeftBranch(){

			return leftbranch;

		}

		void SetLeftBranch( RTreeNode* SetBranch ){

			leftbranch = SetBranch;

		}

		int GetBitPos(){

			return bitpos;

		}

};


/*==============================================================*
* Class: Radix Tree												*                                                     
* Author: Minos	                                                *
* Date: 14/12/11 												*
* Description: The Main Class of Radix Tree, this tree will be  *
* used to store the mapid with his names, will consume more     *
* memory than an array, but will be a lot more faster.			*
**==============================================================*/
template< unsigned int mapid , string server_mapname , string client_mapname >
class RadixTree {

	typedef typename RadixNode<mapid,server_mapname,client_mapname> RTreeNode;
	
private:
	
	// Well, we need a treetop for reference.
	RTreeNode* treetop;

/*
* This is a recursive function to remove a node from the tree *
*/
void RemoveNode(RTreeNode* treeroot){

	if (this.treeroot == NULL) 
		return;

	RTreeNode* LeftBranch_r = (RTreeNode*)this.treeroot->GetLeftBranch();
	RTreeNode* RightBranch_r = (RTreeNode*)this.treeroot->GetRightBranch();

	
	if((LeftBranch_r != this.treeroot) && (LeftBranch_r != treetop) && (LeftBranch_r->GetBitPos() >= this.treeroot->GetBitPos()){
		RemoveNode(LeftBranch_r);
    }else
    if((RightBranch_r != this.treeroot) && (RightBranch_r != treetop) && (RightBranch_r->GetBitPos() >= this.treeroot->GetBitPos()){
		RemoveNode(RightBranch_r);
	}

	DeleteNode treeroot;

}

public:

/*
* Constructor and Destructor *
*/
RadixTree(){
	treetop = new RTreeNode();
	treetop->SMapid = mapid();
}

~RadixTree(){
	RemoveNode(treetop);
}


/*
* GetBit Module: Used to get a bit from a bitstream, like SMapid *
*/
int GetBit(mapid SMapid, int pos) {

	unsigned char* bytemap = (unsigned char*)&this.SMapid;
	unsigned int byteva = pos/8;
	unsigned int bitva = 8-1-(pos % 8);
	unsigned char bitmask = 0x01 << bitva; 
	
	bytemap = bytemap + byteva;
	
	int BitReturn = ((*bytemap) & bitmask) >> bitva;
	assert(BitReturn == 0 || BitReturn == 1);
	
	return BitReturn;
}

/*
* First Diferent Bit Module: Well, like the name says, find the first diferent bit between 2 bitstreams *
*/
int FirstDiferentBit(mapid server_mapid1, mapid server_mapid2){

	int size=0,bytepos=0,DifBitPos=0;
	
	// Pointers to the bitstream
	unsigned char* Pointer_SMapid1 = (unsigned char*)&SMapid1;
	unsigned char* Pointer_SMapid2 = (unsigned char*)&SMapid2;

	while (size < sizeof(mapid) && *Pointer_SMapid1 == *Pointer_SMapid2){
		size++;
		Pointer_SMapid1++;
		Pointer_SMapid2++;
	}
	
	mapid* object_SMapid1 = (mapid*)Pointer_SMapid1;
	mapid* object_SMapid2 = (mapid*)Pointer_SMapid2;

	while (GetBit(*object_SMapid1, bytepos) == GetBit(*object_SMapid2, bytepos)){
		bytepos++;
	}
	
	DifBitPos = (size << 3)+bytepos;

	return DifBitPos;

}

/*
* Search Map ID Module: This Module will search for the mapid in the tree, then it will return the names of the map *
*/
string SearchMapid(mapid SMapid){

	RTreeNode* NodeAux = SearchNode_ack(this.SMapid);

	if(!NodeAux)
		return server_mapname(),client_mapname();
	else
		return  NodeAux->GetServer_Mapname(),NodeAux->GetClient_Mapname();
}

/*
* Compare Map ID Module: Check if 2 Bitstreams are equal *
*/
bool CompareMapid(mapid mapid1, mapid mapid2) {
   
   if(mapid1 == mapid2)
	   return true;
   else
	   return false;

}

/*
* Copy Map ID Module: It copies all informations from a node to another one *
*/
void CopyMapid(RTreeNode* SourceNode, RTreeNode* DestinationNode) {
	DestinationNode->Get_Mapid() = SourceNode->Get_Mapid();
	DestinationNode->GetServer_Mapname() = SourceNode->GetServer_Mapname();
	DestinationNode->GetClient_Mapname() = SourceNode->GetClient_Mapname();
}


/*
* Search Node ACK Module: With the mapid the module will return the Node that contains the chosen mapid *
*/
RTreeNode* SearchNode_ack( mapid SMapid ){

	RTreeNode* NodeAux1;
	RTreeNode* NodeAux2;

	NodeAux1 = treetop;
	NodeAux2 = (RTreeNode*)(treetop->GetRightBranch());

	while (NodeAux1->GetBitPos() < NodeAux2->GetBitPos()) {
		NodeAux1 = NodeAux2;
		NodeAux2 = (RTreeNode*)(GetBit(this.SMapid,NodeAux2->GetBitPos())? NodeAux2->GetRightBranch():NodeAux2->GetLeftBranch());

    }

	if(CompareMapid(SMapid, NodeAux2->Get_Mapid()) != 0)
		return NULL; // There is not a Node with this mapid.
	else
		return NodeAux2;

}


/*
* Exist Node Module: With the mapid the module will return the Node that contains the chosen mapid *
*/
bool ExistNode( mapid SMapid ){

	RTreeNode* node = SearchNode_ack(this.SMapid);
	
	if( node != NULL )
		return true;
	else
		return false;

}

/*
* Get All Tree Module: Get The elements of a tree and save them in a list. Top/Root -> Left -> Right *
*/
list<RTreeNode*> GetAllTree ( RTreeNode* treeroot ){


		list<RTreeNode*> ReturnTree;
	    list<RTreeNode*> BranchLeft,BranchRight;
	    RTreeNode* AuxNode = (this.treeroot == NULL ? treetop : treeroot);

		if(AuxNode->bitpos() >= 0)	
			ReturnTree.insert(ReturnTree.end(), AuxNode);

		// Down to the Left Branch.
		if (AuxNode->GetLeftBranch() != NULL && AuxNode->GetLeftBranch()->GetBitPos() > AuxNode->GetBitPos() ) 
			BranchLeft = GetAllTree(AuxNode->GetLeftBranch());

		ReturnTree.insert(ReturnTree.end(),BranchLeft.begin(),BranchLeft.end());

		// Down to the Right Branch.
		if (AuxNode->GetRightBranch() != NULL && AuxNode->GetRightBranch()->GetBitPos() > AuxNode->GetBitPos() ) 
			BranchRight = GetAllTree(AuxNode->GetRightBranch());

		ReturnTree.insert(ReturnTree.end(),BranchRight.begin(),BranchRight.end());

		return ReturnTree;

}

/*
* Find Father Module: Find the potential fathernode to a new element *
*/
RTreeNode* FindFather( mapid SMapid ){

	RTreeNode *AuxNode1, *AuxNode2;

	AuxNode1 = treetop;
	AuxNode2 = (RTreeNode*)(AuxNode1->GetRightBranch());

	while(AuxNode1->GetBitPos() < AuxNode2->GetBitPos()) {
		AuxNode1 = AuxNode2;
		AuxNode2 = (RTreeNode*)(GetBit(this.SMapid, AuxNode2->GetBitPos())?AuxNode2->GetRightBranch(): AuxNode2->GetLeftBranch());
	}

	return AuxNode2;

}

/*
* Bit Position to Node: With the Bit Position, the module will search and return the node localized in this position*
*/
RTreeNode* Bitpos2node( RTreenode* treeroot, int bitpos ){

	if (this.bitpos == -1)
		return treetop;

	list<RTreeNode*> allnodes = GetAllTree(this.treeroot);
	list<RTreeNode*>::iterator i = allnodes.begin();
	list<RTreeNode*>::iterator j = allnodes.end();

	for ( ; i != j; i++) {
		RTreeNode* node = *i;
		if (node->GetBitPos() == this.bitpos) return node;
	}

	return NULL;
}

/*
* Insert Node: Insert a node into the tree*
*/
RTreeNode* InsertNode(mapid SMapid, server_mapname SMapname, client_mapname CMapname) {


	RTreeNode *TreeNode1, *TreeNode2, *TreeNode3;

	TreeNode1 = treetop;
	TreeNode2 = (RTreeNode*)TreeNode1->GetRightBranch();

	
	// See if the MAPID is already used, return NULL if it is.
	while (TreeNode1->bitpos < TreeNode2->bitpos) {
		TreeNode1 = TreeNode2;
		TreeNode2 = (RTreeNode*)(GetBit(this.SMapid, TreeNode2->GetBitPos() ) ? TreeNode2->GetRightBranch() : TreeNode2->GetLeftBranch());
	}

	if (CompareMapid(SMapid, TreeNode2->Get_Mapid()))
		return NULL; 

	
	int fdb = FirstDiferentBit(this.SMapid, TreeNode2->Get_Mapid());

	// Search were to insert the node.
	TreeNode1 = treetop;
	TreeNode3 = (RTreeNode*)(TreeNode1->GetRightBranch());
	
	while ((TreeNode1->GetBitPos() < TreeNode3->GetBitPos()) && (TreeNode3->GetBitPos() < fdb)){
		TreeNode1 = TreeNode3;
		TreeNode3 = (RTreeNode*)(GetBit(this.SMapid,TreeNode3->GetBitPos() ? TreeNode3->GetRightBranch() : TreeNode3->GetLeftBranch());
	}

	
	TreeNode2 = new RTreeNode(); // Alocate memory and set the node to active.
	TreeNode2->StartNode(this.SMapid, this.SMapname, this.CMapName , fdb , GetBit(this.SMapid,fdb)?TreeNode3:TreeNode2, GetBit(this.SMapid,fdb)?TreeNode2:TreeNode3);
	
	if(GetBit(this.SMapid,TreeNode1->GetBitPos()))
		TreeNode1->SetRightBranch(TreeNode2);
	else
		TreeNode2->SetLeftBranch(TreeNode2);

	return TreeNode2;

}

/*
* Delete Node: Remove a node and all his informations from the tree, then rebalance the tree*
*/
bool DeleteNode(mapid SMapid)  {


	// A LOT of auxiliary nodes
	(RTreeNode*) *AuxNode1, *AuxNode2, *AuxNode3, *AuxNode4, *AuxNode5; 
	int NodeIndex, NodeIndexRight, NodeIndexLeft;
	mapid LMapid;

	AuxNode1 = treetop;
	AuxNode2 = (RTreeNode*)(AuxNode1->GetRightBranch());


	// Search for the mapid to delete. If not found, return false
	while (AuxNode1->GetBitPos() < AuxNode2->GetBitPos()) {
		AuxNode3 = AuxNode1;
		AuxNode1  = Auxnode2;
		AuxNode2  = (RTreeNode*)(GetBit(this.SMapid,AuxNode2->GetBitPos())? AuxNode2->GetRightBranch() : AuxNode2->GetLeftBranch());
	}

	if (CompareMapid(this.SMapid,AuxNode2->GetMapid()) != 0)
		return false; 

	if (AuxNode2 != AuxNode1)
		CopyMapid(AuxNode1, AuxNode2);

	// See if the AuxNode1 is a leaf
	NodeIndex = AuxNode1->GetBitPos();
	NodeIndexLeft = (RTreeNode*)AuthNode1->GetLeftBranch()->GetBitPos();   
	NodeIndexRight = (RTreeNode*)AuthNode1->GetRightBranch()->GetBitPos();

	if (NodeIndexRight > NodeIndex || NodeIndexLeft > NodeIndex) {
		
      if (AuxNode1 != AuxNode2) {
			
			LMapid = mapid(AuxNode1->GetMapid());

			AuxNode5 = AuxNode1;
			AuxNode4 = (RTreeNode*)(GetBit(LMapid, AuxNode1->GetBitPos()) ? AuxNode1->GetRightBranch() : AuxNode1->GetLeftBranch());
      
			while(AuxNode5->GetBitPos() < AuxNode5->GetBitPos()) {
				AuxNode5 = AuxNode4;
				AuxNode4 = (RTreeNode*)(GetBit(LMapid, AuxNode4->GetBitPos()) ? AuxNode4->GetRightBranch() : AuxNode4->GetLeftBranch());
			}

	        if(CompareMapid(LMapid,AuxNode4->GetMapid()) != 0)
				return false; 

			if(GetBit(LMapid,AuxNode5->(GetBitPos())
				AuxNode5->SetRightBranch(AuxNode2);	
			else
				AuxNode5->SetLeftBranch(AuxNode2);	

		}

		if (AuxNode3 != AuxNode1) {
			(RTreeNode*) x = (RTreeNode*)(GetBit(LMapid,AuxNode1->GetBitPos())?AuxNode1->GetRightBranch():AuxNode1->GetLeftBranch());
			if(GetBit(LMapid, AuxNode3->GetBitPos()))   
				AuxNode3->SetRightBranch(x);
			else
				AuxNode3->SetLeftBranch(x);

		}

	} else {

		// Rebalance the tree.
		if (AuxNode3 != AuxNode1) {
			(RTreeNode*)AuxNode6 = (RTreeNode*)AuxNode1->GetLeftBranch();
			(RTreeNode*)AuxNode7 = (RTreeNode*)AuxNode1->GetRightBranch();
			(RTreeNode*)AuxNode8 = (((AuxNode6 == AuxNode7 && (AuxNode6 == AuxNode1))?AuxNode3 : ((AuxNode6==AuxNode1)?AuxNode7 : AuxNode6)));
			
			if(GetBit(LMapid, AuxNode3->GetBitPos()))
				AuxNode3->SetRightBranch(AuxNode8);
			else
				AuxNode3->SetLeftBranch(AuxNode8);
		
		}

	}

	delete AuxNode1;
	return true;
}


};