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
}

//Node::Node(const Node& orig) {}

Node::~Node() {
}

Node::Node(int count, int position, float value, float volume) {
    this->_current_content = vector<int>(count, 2);
    this->_current_position = position;
    this->_current_volume = volume;

}

void Node::expand(Node * n0, Node * n1, vector<float> * volumes) {

    // no change part
    (*n0)._current_content = this->_current_content;
    (*n0).setItemAt(this->_current_position, false);
    (*n0)._current_position = this->_current_position + 1;
    (*n0)._current_volume = this->_current_volume; // nothing added

    
    //adding an item part
    (*n1)._current_content = this->_current_content; //same content
    (*n1).setItemAt(this->_current_position, true); //plus add upcoming item

    (*n1)._current_position = this->_current_position + 1;
    (*n1)._current_volume = this->_current_volume + (*volumes)[this->_current_position]; //volume added
     
}

float Node::getCurrentVolume() {
    return this->_current_volume;
}

int Node::getCurrentPosition() {
    return this->_current_position;
}

vector<int> Node::getCurrentContent() {
    return this->_current_content;
}

bool Node::isExpandable() {
    int vector_size = this->_current_content.size();

    return (vector_size > (this->_current_position)); // 2 items >  #1  ->>  expandable! bc. -> <x,0>,<x,1>..
}


bool Node::isItemAt(int index) {
    // the reason this is implemeted is to encapsulate the way items are represented.
    // changing binary vector <-> bitwise field would only require this method to change while the rest stays the same :)
    
    return ((this->getCurrentContent())[index] == 1);
}

void Node::setItemAt(int index, bool itemPresent) {
    // same reasoning as above
    if(itemPresent) { //1
        (this->_current_content)[index] = 1;
    } else {
        (this->_current_content)[index] = 0;
    }

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
    for (int b = 0; b < this->_current_content.size(); b++)
        cout << (this->_current_content)[b] << " ";
    cout << ">" << endl;

    cout << "Volume: " << this->_current_volume << endl;
    cout << "Depth: " << this->_current_position << endl;
    cout << "Expandable: " << ((this->isExpandable()) ? "YES" : "NO") << endl;
}
