/* 
 * File:   Node.cpp
 * Author: Daniel
 * 
 * Created on 30. září 2010, 22:48
 */

//#include <limits>

#include "Node.h"

Node::Node() {
    this->_current_value = 0;
    this->_current_volume = 0;
}


//Node::Node(const Node& orig) {}

Node::~Node() {
}

Node::Node(int count, int position, float value, float volume) {
    this->_current_content  = vector<int>(count,2);
    this->_current_position = position;
    this->_current_value = value;
    this->_current_volume = volume;

}

//nutno volat jako expand ( &sem1, &sem2, &vektor_values, &vektor_volumes);

void Node::expand(Node * n0, Node * n1, vector<float> * values, vector<float> * volumes) {
    //oriznout zleva
    vector<int> vect_0 = this->_current_content;
    vect_0[this->_current_position] = 0;

    (*n0)._current_content = vect_0;
    (*n0)._current_position = this->_current_position + 1;
    (*n0)._current_value = this->_current_value; // nothing added
    (*n0)._current_volume = this->_current_volume; // nothing added



    vector<int> vect_1 = this->_current_content;
    vect_1[this->_current_position] = 1;

    (*n1)._current_content = vect_1;
    (*n1)._current_position = this->_current_position + 1;
    (*n1)._current_value = this->_current_value + (*values)[this->_current_position]; // value added
    (*n1)._current_volume = this->_current_volume + (*volumes)[this->_current_position];
    ; // volume added


}

float Node::getCurrentValue() {
    return this->_current_value;
}

float Node::getCurrentVolume() {
    return this->_current_volume;
}

int Node::getCurrentPosition() {
    return this->_current_position;
}

vector<int> Node::getCurrentContent() {
    return (this->_current_content);
}

bool Node::isExpandable() {
    int vector_size = this->_current_content.size();

    return (vector_size > (this->_current_position)); // 2 items >  #1  ->>  expandable! bc. -> <x,0>,<x,1>..
}

void Node::print() {
    cout << "---Node Content:---" << endl;

    cout << "Vector content: <";
    for (int b = 0; b < this->_current_content.size(); b++)
        cout << (this->_current_content)[b] << " ";
    cout << ">" << endl;

    cout << "Value: " << this->_current_value << endl;
    cout << "Volume: " << this->_current_volume << endl;
    cout << "Depth: " << this->_current_position << endl;
    cout << "Expandable: " << ((this->isExpandable())?"YES":"NO") << endl;


}
