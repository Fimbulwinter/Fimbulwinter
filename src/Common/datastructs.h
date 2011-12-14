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

template< unsigned int mapid , string server_mapname , string client_mapname >
class RadixNode {

    

	private:
		int bitpos;
		mapid SMapid;
		server_mapname SMapname;
		client_mapname CMapname;
		RadixNode<mapid,server_mapname,client_mapname>** rightbranch;
		RadixNode<mapid,server_mapname,client_mapname>** leftbranch;

	public:
		
		// RadixTree Constructor
		RadixNode( mapid SMapid, server_mapname SMapname, client_mapname CMapName , int bitpos, RadixNode<mapid,server_mapname,client_mapname>* rightbranch, RadixNode<mapid,server_mapname,client_mapname>* leftbranch ){

			StartNode( this.SMapid , this.SMapname , this.CMapName , this.bitpos , this.rightbranch , this.leftbranch );

		}
			
		~RadixNode();
		
		// Node Starter
		void StartNode (mapid SMapid, server_mapname SMapname, client_mapname CMapname, int bitpos , RadixNode<mapid,server_mapname,client_mapname>* leftbranch, RadixNode<mapid,server_mapname,client_mapname>* rightbranch ){

				SMapid = this.SMapid;
				SMapname = this.SMapname;
				CMapName = this.CMapName;
				leftbranch = this.leftbranch;
				rightbranch = this.rightbranch;
				bitpos = this.bitpos;
		
		}

        
		// Getters and Setters
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
		
		RadixNode<mapid,server_mapname,client_mapname>* GetRightBranch(){

			return rightbranch;

		}

		void SetRightBranch( RadixNode<mapid,server_mapname,client_mapname>* SetBranch ){

			rightbranch = SetBranch;

		}
	    
		RadixNode<mapid,server_mapname,client_mapname>* GetLeftBranch(){

			return leftbranch;

		}

		void SetLeftBranch( RadixNode<mapid,server_mapname,client_mapname>* SetBranch ){

			leftbranch = SetBranch;

		}

		int GetBitPos(){

			return bitpos;

		}

};

template< unsigned int mapid , string server_mapname , string client_mapname >
class RadixTree {

private:
	   
	RadixTree<mapid,server_mapname,client_mapname>* treetop;

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

void RemoveNode(RadixNode<mapid,server_mapname,client_mapname>* treeroot){

	if (treeroot == NULL) 
		return;

	RadixNode<mapid,server_mapname,client_mapname>* LeftBranch_r = (RadixNode<mapid,server_mapname,client_mapname>*)treeroot->GetLeftBranch();
	RadixNode<mapid,server_mapname,client_mapname>* RightBranch_r = (RadixNode<mapid,server_mapname,client_mapname>*)treeroot->GetRightBranch();

	
	if((LeftBranch_r != treeroot) && (LeftBranch_r != treetop) && (LeftBranch_r->GetBitPos() >= treeroot->GetBitPos()){
		RemoveNode(LeftBranch_r);
    }else
    if((RightBranch_r != treeroot) && (RightBranch_r != treetop) && (RightBranch_r->GetBitPos() >= treeroot->GetBitPos()){
		RemoveNode(RightBranch_r);
	}

	DeleteNode treeroot;

}

void Copy_Mapid(RadixNode<mapid,server_mapname,client_mapname>* SourceNode, RadixNode<mapid,server_mapname,client_mapname>* DestinationNode) {
	DestinationNode->Get_Mapid() = SourceNode->Get_Mapid();
	DestinationNode->GetServer_Mapname() = SourceNode->GetServer_Mapname();
	DestinationNode->GetClient_Mapname() = SourceNode->GetClient_Mapname();
}
		
public:
RadixTree(){
	treetop = new RadixNode<mapid,server_mapname,client_mapname>();
	treetop->SMapid = mapid();
}

~RadixTree(){
	RemoveNode(treetop);
}

int FirstDiferentBit(mapid server_mapid1, mapid server_mapid2){

	int size=0,bytepos=0,DifBitPos=0;
	
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

bool CompareMapid(mapid mapid1, mapid mapid2) {
   
   if(mapid1 == mapid2)
	   return true;
   else
	   return false;

}
	
RadixNode<mapid,server_mapname,client_mapname>* InsertNode(mapid SMapid, server_mapname SMapname, client_mapname CMapname) {


	RadixNode<mapid,server_mapname,client_mapname> *TreeNode1, *TreeNode2, *TreeNode3;

	TreeNode1 = treetop;
	TreeNode2 = (RadixNode<mapid,server_mapname,client_mapname>*)TreeNode1->GetRightBranch();

	
	while (TreeNode1->bitpos < TreeNode2->bitpos) {
		TreeNode1 = TreeNode2;
		TreeNode2 = (RadixNode<mapid,server_mapname,client_mapname>*)(GetBit(SMapid, TreeNode2->bitpos) ? TreeNode2->GetRightBranch() : TreeNode2->GetLeftBranch());
	}

	if (CompareMapid(SMapid, TreeNode2->Get_Mapid()))
		return NULL; 

	int fdb = FirstDiferentBit(SMapid, TreeNode2->Get_Mapid());

	
	TreeNode1 = treetop;
	TreeNode3 = (RadixNode<mapid,server_mapname,client_mapname>*)(TreeNode1->GetRightBranch());
	
	while ((TreeNode1->GetBitPos() < TreeNode3->GetBitPos()) && (TreeNode3->GetBitPos() < fdb)){
		TreeNode1 = TreeNode3;
		TreeNode3 = (RadixNode<mapid,server_mapname,client_mapname>*)(GetBit(SMapid,TreeNode3->GetBitPos() ? TreeNode3->GetRightBranch() : TreeNode3->GetLeftBranch());
	}

	
	TreeNode2 = new RadixNode<mapid,server_mapname,client_mapname>();
	TreeNode2->StartNode(SMapid, SMapname, CMapName , fdb , GetBit(SMapid,fdb)?TreeNode3:TreeNode2, GetBit(SMapid,fdb)?TreeNode2:TreeNode3);
	
	if(GetBit(SMapid,TreeNode1->GetBitPos()))
		TreeNode1->SetRightBranch(TreeNode2);
	else
		TreeNode2->SetLeftBranch(TreeNode2);

	return TreeNode2;

}



};