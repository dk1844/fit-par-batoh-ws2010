/*
 * File:   Node.cpp
 * Author: Daniel
 *
 * Created on 30. září 2010, 22:48
 */

//#include <limits>

#include "Node.h"

Node::Node() {
    this->_current_volume = 0;
    this->_int_char_size = sizeof(char) * 8;

}

//Node::Node(const Node& orig) {}

Node::~Node() {
}

Node::Node(int count, int position, float volume) {
    this->_int_char_size = sizeof(char) * 8;
    //if count is fold of sizeof(char), simple dividing will determine no. chars we need

    //if ((count % this->_int_char_size)==0) {
        this->_current_content = vector<char>((count/this->_int_char_size) + 1, 0);
    //} else {
        //if there are some items remain after dividing, add 1 more char
      //  this->_current_content = vector<char>((count/this->_int_char_size) + 1, 0);
    //}

    this->_current_position = position;
    this->_current_volume = volume;
    this->_items_count = count;
}

void Node::expand(Node * n0, Node * n1, vector<float> * volumes) {

    // no change part
    (*n0)._current_content = this->_current_content;
    (*n0).setItemAt(this->_current_position, false);
    (*n0)._current_position = this->_current_position + 1;
    (*n0)._current_volume = this->_current_volume; // nothing added
    (*n0)._items_count = this->_items_count;

    //adding an item part
    (*n1)._current_content = this->_current_content; //same content
    (*n1).setItemAt(this->_current_position, true); //plus add upcoming item

    (*n1)._current_position = this->_current_position + 1;
    (*n1)._items_count = this->_items_count;
    (*n1)._current_volume = this->_current_volume + (*volumes)[this->_current_position]; //volume added

}

float Node::getCurrentVolume() {
    return this->_current_volume;
}

int Node::getCurrentPosition() {
    return this->_current_position;
}

vector<char> Node::getCurrentContent() {
    return this->_current_content;
}

bool Node::isExpandable() { //CHANGE: I added parametr to this function, because we cannot use .size() function
    
    //int vector_size = this->_current_content.size();

    //return (vector_size > (this->_current_position)); // 2 items >  #1  ->>  expandable! bc. -> <x,0>,<x,1>..
    return (this->_items_count > (this->_current_position)); // 2 items >  #1  ->>  expandable! bc. -> <x,0>,<x,1>..
}


bool Node::isItemAt(int index) {
    // the reason this is implemeted is to encapsulate the way items are represented.
    // changing binary vector <-> bitwise field would only require this method to change while the rest stays the same :)

    //first, we need to find, in which bit group (=one char member of vector) is our index
    int int_group= index/this->_int_char_size;

    //TODO: maybe, i can return  this->_current_content[int_group] & mask)==0 directly
    if ((this->_current_content[int_group] & this->getMask(index))==0) {
         return 0;

    }else{
         return 1;
    }

 }

void Node::setItemAt(int index, bool itemPresent) {
    // same reasoning as above
    //first, we need to find, in which bit group (=one char member of vector) is our index
    int int_group= index/this->_int_char_size;
    
    if(itemPresent) { //1
       this->_current_content[int_group] = this->_current_content[int_group] | this->getMask(index);
    } else {
        this->_current_content[int_group] = this->_current_content[int_group] & ~this->getMask(index);
    }

}

char Node::getMask(int index){
    //get index of our item in selected char
    int int_bit_positon = index%this->_int_char_size;

    //set mask to 1 -> 1 will be shifted to left to find proper mask
    char mask=1;

    //rotate mask. First item in char is in MSB, last one in LSB
    for (int i = 0; i < this->_int_char_size - int_bit_positon - 1; i++) {
            mask = mask << 1;
            }
    return mask;
}

float Node::calculateValue(int allItemsCount, vector<float> * values) {
    float sumValue = 0;

    for (int i=0; i < allItemsCount; i++) {
        if (this->isItemAt(i)) {
            sumValue = sumValue + (*values)[i];
        }
    }

    return sumValue;
}

void Node::print() {

    cout << "---Node Content:---" << endl;

    cout << "Vector content: <";
    for (int b = 0; b < this->_items_count; b++)
        cout << (this->isItemAt(b)) << " ";
    cout << ">" << endl;

    cout << "Volume: " << this->_current_volume << endl;
    cout << "Depth: " << this->_current_position << endl;
    cout << "Expandable: " << ((this->isExpandable()) ? "YES" : "NO") << endl;
}
