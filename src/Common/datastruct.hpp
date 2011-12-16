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
*						    Database Classes						*
* ================================================================= */


#include <cmath>
#include <list>
#include <cstdlib>
#include <cassert>
#include <iostream>

using namespace std;

class RadixNode {

private:
	
	/* Radix Tree Node Private Vars/Modules
	 * Explanation:
	 * - RadixTrie Class: The base of Radix Tree
	 * - mapid: The unique Map Identification will work as a Radix Key
	 * - smapname: The Map name to the server
	 * - cmapname: The Map name to the client
	 * - Left and Right Branches: The branches of a tree
	 * - bitpos: The index of tree bits
	 * Note: Why two map names? 
	 * - Well, with two mapnames will be possible to create a 'infinite'
	 * number of maps with the same .gat, will make the instanced dungeons
	 * easier to make
	 */
	friend class RadixTrie;
	static unsigned int mapid;
	static string smapname;
	static string cmapname;
	static RadixNode* leftbranch;
	static RadixNode* rightbranch;
	static int bitpos;
	
public:
	
// I will use this typedefs to make the code easier to read.
 
// RadixNode constructor,destructor and starter prototypes
// mapid, Server Mapname, Client Mapname, Bitpos, Right Branch, Left Branch
  void Startnode( unsigned int mapids, string smapnames, string cmapnames, int bitposs, RadixNode* rightbranchs, RadixNode* leftbranchs ){

	mapid = mapids;
	smapname = smapnames;
	cmapname = cmapnames;
	bitpos = bitposs;
	rightbranch = rightbranchs;
	leftbranch = leftbranchs;
 
  }

 RadixNode( unsigned int mapids, string smapnames, string cmapnames, int bitposs, RadixNode* rightbranchs, RadixNode* leftbranchs ){
 
    Startnode( mapids , smapnames , cmapnames , 
	           bitposs , rightbranchs , leftbranchs );
 
 }
 
 RadixNode(){
 }
 
 RadixNode::~RadixNode(){

 }
 
	
/* Radix Node Getters and Setters
 * Explanation: Do i really have to explain
 * what getters and setters are? :(
 */
	 
 static string Gsmapname(){
	return smapname;
 }
	 
 static string Gcmapname(){
	return cmapname;
 }
	
 static void Smapname(string smapnames, string cmapnames){
	smapname = smapnames;
	cmapname = cmapnames;
 }
	
 static unsigned int Gmapid(){
	return mapid;
 } 

 static void SMapid(unsigned int mapids){
	mapid = mapids;
  } 
	
 static RadixNode* Gleftbranch(){
	return leftbranch;
 }
	
 static RadixNode* Grightbranch(){
	return rightbranch;
 }
	
 static void Sleftbranch(RadixNode* branch){
	leftbranch = branch;
 }
	
 static void Srightbranch(RadixNode* branch){
	rightbranch = branch; 
 }
	
 static int Gbitpos(){
	return bitpos;
 }
	
static int Gbit( unsigned int smapid, int pos ){
	
	unsigned char* bytemap = (unsigned char*)&smapid;
	unsigned int byteva = pos/8;
	unsigned int bitva = 8-1-(pos % 8);
	unsigned char bitmask = 0x01 << bitva; 

	bytemap = bytemap + byteva;

	int BitReturn = ((*bytemap) & bitmask) >> bitva;
	assert(BitReturn == 0 || BitReturn == 1);

	return BitReturn;

 }

};

class RadixTrie{

private:
	
	static RadixNode* treetop;

public:
 
 // RadixTree constructor and destructor
 RadixTrie::RadixTrie(){
 
	treetop = new RadixNode();
	treetop->SMapid(unsigned int());
	
 }

 RadixTrie::~RadixTrie(){
	Removenode(Gtreetop());
 }


 /* RadixTree Manipulation
 * Explanation: Well, functions to copy,search
 * insert,compare blablabla. Usage explanation
 * above each function signature in .cpp file.
 */
 static int Firstdifbit( unsigned int, unsigned int );
 static bool Comparemapid( unsigned int, unsigned int );
 static void Copymapid( RadixNode*, RadixNode* );
 static string Searchmapid( unsigned int );
 static RadixNode* Bitpos2node( RadixNode*, int );
 static RadixNode* Searchnode_ack( unsigned int );
 static RadixNode* Findfather( unsigned int );
 static bool Existnode( unsigned int );
 static list<RadixNode*> Getallnodes( RadixNode* treeroot );
 static RadixNode* InsertNode( unsigned int, string, string );
 static void Removenode( RadixNode* treeroot );
 static bool Deletenode( unsigned int );
 
 static RadixNode* Gtreetop(){
	return treetop;
 }
 
 static RadixNode* Streetop( RadixNode* treetops ){
	treetop = treetops;
 }

 
 };