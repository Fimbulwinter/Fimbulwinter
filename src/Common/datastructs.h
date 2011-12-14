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
		mapid server_mapid;
		server_mapname SMapname;
		client_mapname CMapname;
		RadixNode<mapid,server_mapname,client_mapname>* rightbranch;
		RadixNode<mapid,server_mapname,client_mapname>* leftbranch;

	public:
		
		// RadixTree Constructor
		RadixNode( mapid server_mapid, server_mapname SMapName, client_mapname CMapName , int bitpos, RadixNode<mapid,server_mapname,client_mapname>* rightbranch, RadixNode<mapid,server_mapname,client_mapname>* leftbranch ){

			StartNode( this.server_mapid , this.SMapName , this.CMapName , this.bitpos , this.rightbranch , this.leftbranch );

		}
			
		~RadixNode();
		
		// Node Starter
		void StartNode (mapid server_mapid, server_mapname SMapname, client_mapname CMapname, int bitpos , RadixNode<mapid,server_mapname,client_mapname>* leftbranch, RadixNode<mapid,server_mapname,client_mapname>* rightbranch ){

				server_mapid	= this.server_mapid;
				SMapName = this.SMapName;
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
				
			return server_mapid;
		
		}
		
		RadixNode<mapid,server_mapname,client_mapname>* GetRightBranch(){

			return rightbranch;

		}
	    
		RadixNode<mapid,server_mapname,client_mapname>* GetLeftBranch(){

			return leftbranch;

		}

};

template< unsigned int mapid , string server_mapname , string client_mapname >
class RadixTree {

private:

int GetBit(mapid server_mapid, int pos) {

	unsigned char* bytemap = (unsigned char*)&this.server_mapid;
	unsigned int byteva = pos/8;
	unsigned int bitva = 8-1-(pos % 8);
	unsigned char bitmask = 0x01 << bitva; 
	
	bytemap = bytemap + byteva;
	
	int BitReturn = ((*bytemap) & bitmask) >> bitva;
	assert(BitReturn == 0 || BitReturn == 1);
	
	return BitReturn;
}

int FirstDiferentBit(mapid server_mapid1, mapid server_mapid2){

	int size=0,bytepos=0,DifBitPos=0;
	
	unsigned char* pointer_servermapid1 = (unsigned char*)&server_mapid1;
	unsigned char* pointer_servermapid2 = (unsigned char*)&server_mapid2;

	while (size < sizeof(mapid) && *pointer_servermapid1 == *pointer_servermapid2){
		size++;
		pointer_servermapid1++;
		pointer_servermapid2++;
	}
		
	mapid* object_servermapid1 = (mapid*)pointer_servermapid1;
	mapid* object_servermapid2 = (mapid*)pointer_servermapid2;

	while (GetBit(*object_servermapid1, bytepos) == GetBit(*object_servermapid2, bytepos)){
		bytepos++;
	}
	
	DifBitPos = (size << 3)+bytepos;

	return DifBitPos;

}

void RemoveNode(RadixNode<mapid,server_mapname,client_mapname>* treeroot){

	if (treeroot == NULL) 
		return;

	RadixNode<mapid,server_mapname,client_mapname>* RLeftBranch = (RadixNode<mapid,server_mapname>*)treeroot->GetLeftBranch;
	RadixNode<mapid,server_mapname,client_mapname>* RRightBranch = (RadixNode<mapid,server_mapname>*)treeroot->GetRightBranch;

	
	if((RLeftBranch != treeroot) && (RLeftBranch != treetop) && (RLeftBranch->bitpos >= treeroot->bitpos))
		RemoveNode(RLeftBranch);

   if((RRightBranch != treeroot) && (RRightBranch != treetop) && (RRightBranch->bitpos >= treeroot->bitpos))
		RemoveNode(RRightBranch);

	DeleteNode treeroot;

}

void Copy_mapid(RadixNode<mapid,server_mapname,client_mapname>* SourceNode, RadixNode<mapid,server_mapname,client_mapname>* DestinationNode) {
	DestinationNode->server_mapid = SourceNode->server_mapid;
	DestinationNode->SMapName = SourceNode->SMapName;
}
		
public:


};