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
*				  Data Structures Classes				   *
* ======================================================== */


#include <show_message.hpp>
#include <core.hpp>
#include <timers.hpp>
#include <iostream>
#include <strfuncs.hpp>

using namespace std;

template< class radixkey , class value >
class RadixNode {

   
	private:
		int index;
		radixkey key;
		value data;
		RadixNode<radixkey,value>*	rightbranch;
		RadixNode<radixkey,value>*	leftbranch;

	public:
		
		// RadixTree Constructor
		RadixNode( radixkey key, value data, int index, RadixNode<radixkey,value>* rightbranch, RadixNode<radixkey,value>* leftbranch ){

			StartNode( this.key , this.data , this.index , this.rightbranch , this.leftbranch );

		}
			
		~RadixNode();
		
		// Node Starter
		void StartNode (radixkey key, value data, int index , RadixNode<radixkey,value>* leftbranch, RadixNode<radixkey,value>* rightbranch ){

				key	= this.key;
				data = this.data;
				leftbranch = this.leftbranch;
				rightbranch = this.rightbranch;
				index = this.index;
		
		}

        
		// Getters and Setters
		value GetValue(){

			return data;

		}

		bool SetValue(values data){

			data = this.data;
			return true;

        }
	    
		radixkey GetKey() {
				
			return key;
		
		}
		
		RadixNode<radixkey,value>* getRightBranch(){

			return rightbranch;

		}
	    
		RadixNode<radixkey,value>* getLeftBranch(){

			return leftbranch;

		}

};

// Fuck this shit... i'm sleepy. Tomorrow i will finish it.