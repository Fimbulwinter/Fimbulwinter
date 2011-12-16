/*==================================================================*
*     ___ _           _           _          _       _				*
*    / __(_)_ __ ___ | |__  _   _| |_      _(_)_ __ | |_ ___ _ __	*
*   / _\ | | '_ ` _ \| '_ \| | | | \ \ /\ / / | '_ \| __/ _ \ '__|	*
*  / /   | | | | | | | |_) | |_| | |\ V  V /| | | | | ||  __/ |		*
*  \/    |_|_| |_| |_|_.__/ \__,_|_| \_/\_/ |_|_| |_|\__\___|_|		*
*																	*
* ------------------------------------------------------------------*
*							    Emulator			                *
* ------------------------------------------------------------------*
*						Licenced under GNU GPL v3					*
* ------------------------------------------------------------------*
*						    Database Modules						*
* ================================================================= */

#include <cmath>
#include <list>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <datastruct.hpp>


using namespace std;

/*
 * Get All Tree Module: Get The elements of a tree and save them in a list. Top/Root -> Left -> Right *
 */
list<RadixNode*> RadixTrie::Getallnodes( RadixNode* treeroot ){


		list<RadixNode*> ReturnTree;
	    list<RadixNode*> BranchLeft,BranchRight;
	    RadixNode* AuxNode = (treeroot == NULL ? RadixTrie::Gtreetop() : treeroot );

		if(AuxNode->Gbitpos() >= 0)	
			ReturnTree.insert(ReturnTree.end(), AuxNode);

		// Down to the Left Branch.
		if (AuxNode->Gleftbranch() != NULL && AuxNode->Gleftbranch()->Gbitpos() > AuxNode->Gbitpos() ) 
			BranchLeft = Getallnodes(AuxNode->Gleftbranch());

		ReturnTree.insert(ReturnTree.end(),BranchLeft.begin(),BranchLeft.end());

		// Down to the Right Branch.
		if (AuxNode->Grightbranch() != NULL && AuxNode->Grightbranch()->Gbitpos() > AuxNode->Gbitpos() ) 
			BranchRight = Getallnodes(AuxNode->Grightbranch());

		ReturnTree.insert(ReturnTree.end(),BranchRight.begin(),BranchRight.end());

		return ReturnTree;

}
 
 /*
  * Bit Position to Node: With the Bit Position, the module will search and return the node localized in this position
  */
 RadixNode* RadixTrie::Bitpos2node( RadixNode* treeroot, int bitpos ){

	if (bitpos == -1)
		return treetop;

	list<RadixNode*> allnodes = RadixTrie::Getallnodes(treeroot);
	list<RadixNode*>::iterator i = allnodes.begin();
	list<RadixNode*>::iterator j = allnodes.end();

	for ( ; i != j; i++) {
		RadixNode* node = *i;
		if (node->Gbitpos() == bitpos ) return node;
	}

	return NULL;
}  
   
 /*
  * Compare Map ID Module: Check if 2 Bitstreams are equal *
  */ 
 bool RadixTrie::Comparemapid( unsigned int mapid1, unsigned int mapid2 ) {
   
   if(mapid1 == mapid2)
	   return true;
   else
	   return false;

 }

/*
 * Copy Map ID Module: It copies all informations from a node to another one *
 */
 void RadixTrie::Copymapid( RadixNode* Sourcenode, RadixNode* Destinationnode ){

	Destinationnode->SMapid(Sourcenode->Gmapid());
	Destinationnode->Smapname(Sourcenode->Gsmapname(),Sourcenode->Gcmapname()); 
	
}

/*
 * Search Node ACK Module: With the mapid the module will return the Node that contains the chosen mapid *
 */
RadixNode* RadixTrie::Searchnode_ack( unsigned int smapid ){

	RadixNode* Naux1;
	RadixNode* Naux2;

	Naux1 = RadixTrie::Gtreetop();
	Naux2 = (RadixNode*)(RadixTrie::Streetop(RadixTrie::Gtreetop()->Grightbranch()));

	while(Naux1->Gbitpos() < Naux2->Gbitpos()) {
		Naux1 = Naux2;
		Naux2 = (RadixNode*)(RadixNode::Gbit(smapid,Naux2->Gbitpos())? Naux2->Grightbranch():Naux2->Gleftbranch());

    }

	if(RadixTrie::Comparemapid(smapid, Naux2->Gmapid()) != 0)
		return NULL; // There is not a Node with this mapid.
	else
		return Naux2;

}

/*
 * Exist Node Module: With the mapid the module will return the Node that contains the chosen mapid *
 */
bool RadixTrie::Existnode( unsigned int smapid ){

	RadixNode* node = RadixTrie::Searchnode_ack(smapid);
	
	if( node != NULL )
		return true;
	else
		return false;

}

/*
* Find Father Module: Find the potential fathernode to a new element *
*/
RadixNode* RadixTrie::Findfather( unsigned int smapid ){

	RadixNode *AuxNode1, *AuxNode2;

	AuxNode1 = RadixTrie::Gtreetop();
	AuxNode2 = (RadixNode*)(AuxNode1->Grightbranch());

	while(AuxNode1->Gbitpos() < AuxNode2->Gbitpos()) {
		AuxNode1 = AuxNode2;
		AuxNode2 = (RadixNode*)(RadixNode::Gbit(smapid, AuxNode2->Gbitpos())?AuxNode2->Grightbranch(): AuxNode2->Gleftbranch());
	}

	return AuxNode2;

}

/*
 * Search Map ID Module: This Module will search for the mapid in the tree, then it will return the names of the map *
 */
string RadixTrie::Searchmapid(unsigned int smapid){

	RadixNode* Naux = RadixTrie::Searchnode_ack(smapid);

	if(!Naux)
		return RadixNode::Gsmapname(),RadixNode::Gcmapname();
	else
		return  Naux->Gsmapname(),Naux->Gcmapname();
}

/*
 * First Diferent Bit Module: Well, like the name says, find the first diferent bit between 2 bitstreams *
 */
 int RadixTrie::Firstdifbit( unsigned int mapid1, unsigned int mapid2 ){

	 int size = 0;
	 int bytepos = 0;

	 unsigned char* pmapid = (unsigned char*)&mapid1;
	 unsigned char* pmapid2 = (unsigned char*)&mapid2;

	 while((size < sizeof(unsigned int)) && (*pmapid == *pmapid2)){
		 size++;
		 pmapid++;
		 pmapid2++;
	 }

	 unsigned int* omapid1 = (unsigned int*)pmapid;
	 unsigned int* omapid2 = (unsigned int*)pmapid;

	 while(RadixNode::Gbit(*omapid1,bytepos) == RadixNode::Gbit(*omapid2,bytepos)){
		bytepos++;
	 }

	 return (size << 3) + bytepos;
 }
 
 /*
  * This is a recursive function to remove a node from the tree *
  */
void RemoveNode(RadixNode* treeroot){

    // The node can't be null
	if (treeroot == NULL) 
		return;

	RadixNode* LeftBranch_r = (RadixNode*)treeroot->Gleftbranch();
	RadixNode* RightBranch_r = (RadixNode*)treeroot->Grightbranch();

	
	if((LeftBranch_r != treeroot) && (LeftBranch_r != RadixTrie::Gtreetop()) && (LeftBranch_r->Gbitpos() >= treeroot->Gbitpos())){
		RemoveNode(LeftBranch_r);
    }else
    if((RightBranch_r != treeroot) && (RightBranch_r != RadixTrie::Gtreetop()) && (RightBranch_r->Gbitpos() >= treeroot->Gbitpos())){
		RemoveNode(RightBranch_r);
	}

	//DeleteNode treeroot;

}

/*
* Delete Node: Remove a node and all his informations from the tree, then rebalance the tree*
*/
bool RadixTrie::Deletenode(unsigned int smapid)  {


	// A LOT of auxiliary nodes
	RadixNode *AuxNode1, *AuxNode2, *AuxNode3, *AuxNode4, *AuxNode5; 
	int NodeIndex, NodeIndexRight, NodeIndexLeft;
	unsigned int LMapid;

	AuxNode1 = RadixTrie::Gtreetop();
	AuxNode2 = (RadixNode*)(AuxNode1->Grightbranch());


	// Search for the mapid to delete. If not found, return false
	while (AuxNode1->Gbitpos() < AuxNode2->Gbitpos()) {
		AuxNode3 = AuxNode1;
		AuxNode1  = AuxNode2;
		AuxNode2  = (RadixNode*)(RadixNode::Gbit(smapid,AuxNode2->Gbitpos())? AuxNode2->Grightbranch() : AuxNode2->Gleftbranch());
	}

	if (RadixTrie::Comparemapid(smapid,AuxNode2->Gmapid()) != 0)
		return false; // Don't exist
    
	// Copy node2 to node1
	if (AuxNode2 != AuxNode1)
		RadixTrie::Copymapid(AuxNode1, AuxNode2);

	// See if the AuxNode1 is a leaf
	NodeIndex = AuxNode1->Gbitpos();
	NodeIndexLeft = (int)AuxNode1->Gleftbranch()->Gbitpos();   
	NodeIndexRight = (int)AuxNode1->Grightbranch()->Gbitpos();

	if (NodeIndexRight > NodeIndex || NodeIndexLeft > NodeIndex) {
		
      if (AuxNode1 != AuxNode2) {
			
			LMapid = (unsigned int)AuxNode1->Gmapid();

			AuxNode5 = AuxNode1;
			AuxNode4 = (RadixNode*)(RadixNode::Gbit(LMapid, AuxNode1->Gbitpos()) ? AuxNode1->Grightbranch() : AuxNode1->Gleftbranch());
      
			while(AuxNode5->Gbitpos() < AuxNode5->Gbitpos()) {
				AuxNode5 = AuxNode4;
				AuxNode4 = (RadixNode*)(RadixNode::Gbit(LMapid, AuxNode4->Gbitpos()) ? AuxNode4->Grightbranch() : AuxNode4->Gleftbranch());
			}

	        if(RadixTrie::Comparemapid(LMapid,AuxNode4->Gmapid()) != 0)
				return false;  // The Map ID don't match, should not occur.

			if(RadixNode::Gbit(LMapid,AuxNode5->Gbitpos()))
				AuxNode5->Srightbranch(AuxNode2);	
			else
				AuxNode5->Sleftbranch(AuxNode2);	

		}

		if (AuxNode3 != AuxNode1) {
			RadixNode* x = (RadixNode*)(RadixNode::Gbit(LMapid,AuxNode1->Gbitpos())?AuxNode1->Grightbranch():AuxNode1->Gleftbranch());
			if(RadixNode::Gbit(LMapid, AuxNode3->Gbitpos()))   
				AuxNode3->Srightbranch(x);
			else
				AuxNode3->Sleftbranch(x);

		}

	} else {

		// Rebalance the tree.
		if (AuxNode3 != AuxNode1) {
			 RadixNode* AuxNode6 = (RadixNode*)AuxNode1->Gleftbranch();
			 RadixNode* AuxNode7 = (RadixNode*)AuxNode1->Grightbranch();
			 RadixNode* AuxNode8 = (((AuxNode6 == AuxNode7 && (AuxNode6 == AuxNode1))?AuxNode3 : ((AuxNode6==AuxNode1)?AuxNode7 : AuxNode6)));
			
			if(RadixNode::Gbit(LMapid, AuxNode3->Gbitpos()))
				AuxNode3->Srightbranch(AuxNode8);
			else
				AuxNode3->Sleftbranch(AuxNode8);
		
		}

	}

	// Free the node memory and return success :D
	delete AuxNode1;
	return true;
}

/*
* Insert Node: Insert a node into the tree*
*/
/*
* Insert Node: Insert a node into the tree*
*/
RadixNode* RadixTrie::InsertNode(unsigned int smapid, string smapnamess, string cmapnames) {


	RadixNode *TreeNode1, *TreeNode2, *TreeNode3;

	TreeNode1 = RadixTrie::Gtreetop();
	TreeNode2 = (RadixNode*)TreeNode1->Grightbranch();

	
	// See if the MAPID is already used, return NULL if it is.
	while (TreeNode1->Gbitpos() < TreeNode2->Gbitpos()) {
		TreeNode1 = TreeNode2;
		TreeNode2 = (RadixNode*)(RadixNode::Gbit(smapid, TreeNode2->Gbitpos() ) ? TreeNode2->Grightbranch() : TreeNode2->Gleftbranch());
	}

	if (RadixTrie::Comparemapid(smapid, TreeNode2->Gmapid()))
		return NULL; 

	
	int fdb = RadixTrie::Firstdifbit(smapid, TreeNode2->Gmapid());

	// Search were to insert the node.
	TreeNode1 = RadixTrie::Gtreetop();
	TreeNode3 = (RadixNode*)(TreeNode1->Grightbranch());
	
	while ((TreeNode1->Gbitpos() < TreeNode3->Gbitpos()) && (TreeNode3->Gbitpos() < fdb)){
		TreeNode1 = TreeNode3;
		TreeNode3 = (RadixNode*)(RadixNode::Gbit(smapid,TreeNode3->Gbitpos()) ? TreeNode3->Grightbranch() : TreeNode3->Gleftbranch());
	}

	
	TreeNode2 = new RadixNode(); // Alocate memory and set the node to active.
	TreeNode2->Startnode(smapid, smapnamess, cmapnames , fdb , RadixNode::Gbit(smapid,fdb)?TreeNode3:TreeNode2, RadixNode::Gbit(smapid,fdb)?TreeNode2:TreeNode3);
	
	if(RadixNode::Gbit(smapid,TreeNode1->Gbitpos()))
		TreeNode1->Srightbranch(TreeNode2);
	else
		TreeNode2->Sleftbranch(TreeNode2);

	return TreeNode2;

}
 
