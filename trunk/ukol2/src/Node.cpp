/*
 * File:   Node.cpp
 * Author: Daniel
 *
 * Created on 30. září 2010, 22:48
 */

//#include <limits>

#include "Node.h"
#include <sstream>


Node::Node() {
    this->_current_volume = 0.0;
    this->_int_char_size = sizeof(char) * 8;
    
    this->_int_char_size = sizeof(char) * 8;
    //if count is fold of sizeof(char), simple dividing will determine no. chars we need
    this->_current_content = vector<char> (_int_char_size,0);
    this->_items_count = 0;

}

//Node::Node(const Node& orig) {}

Node::~Node() {
}

Node::Node(int count, int position, float volume) {

    this->_int_char_size = sizeof(char) * 8;
    this->_current_content = vector<char> (_int_char_size,0);


    //if ((count % this->_int_char_size)==0) {
        //this->_current_content = vector<char>((count/this->_int_char_size) + 1, 0);

    this->_current_content.resize((count/this->_int_char_size) + 1,0);

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
void Node::setCurrentVolume(float volume) {
    this->_current_volume = volume;
}

int Node::getCurrentPosition() {
    return this->_current_position;
}
void Node::setCurrentPosition(int position) {
    this->_current_position = position;
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

float Node::calculateValue(vector<float> * values) {
    float sumValue = 0;
    int allItemsCount = this->getItemsCount();

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

int Node::getItemsCount() {
    return this->_items_count;
}
void Node::setItemsCount(int count) {
    this->_items_count = count;
}

/**
 * Serialization of a node
 * expected format is <total_items_count|current_position|current_volume|vector>, e.g.: <5|10001|1|2.1>
 * @param buffer buffer to write into
 * @param bufferSize buffer size
 * @return
 */
bool Node::serialize(char * buffer, int &bufferSize) {

    /*
    string content;
    content.clear();
    char b[100]; //aux

    //items count|
    sprintf (b, "%d", this->_items_count);
    content.append( b );
    content.append("|");

    //vector|
    for (int i=0;i < this->_items_count;i++) {
        if (this->isItemAt(i)) {
            content.append("1");
        } else {
            content.append("0");
        }

    }
    content.append("|");

    //current position|
    sprintf (b, "%d", this->_current_position);
    content.append( b );
    content.append("|");

   //current volume|
    stringstream sstr;
    sstr << this->getCurrentVolume();
    //string str1 = sstr.str();
    //sprintf (b, "%f", this->_current_volume); //tady to pada!
    content.append( sstr.str() );
    content.append("|");



    
    //finding out size?
    bufferSize = content.size();

    //making a c-string
    strcpy(buffer, content.c_str());
*/
    stringstream sstr;
    sstr.clear();
    sstr.flush();

    sstr << this->getItemsCount();
    sstr << " " << this->getCurrentPosition();
    sstr << " " << this->getCurrentVolume() << " " ;
    for (int i=0;i < this->_items_count; i++) {
        if (this->isItemAt(i)) {
            sstr << '1' ;
        } else {
            sstr << '0' ;
        }
    }
    
    //bufferSize = sstr.str().size();
    string res = sstr.str();
    bufferSize = res.length();

    strcpy(buffer, res.c_str());

    //strncpy(buffer, sstr.str().c_str(), sstr.str().size());
    //debug
    //cout << buffer  << " .. size = " << bufferSize << endl;
    //cout << res  << " .. size = " << bufferSize << endl;
    
  //cout << sstr.str().c_str()  << " .. size = " << bufferSize << endl;

    return true;
}

bool Node::deserialize(char * buffer, int bufferSize) {
    /* string a;
    a = buffer;

    int pos1 = a.find("|",0);

    cout << pos1 << endl;

    int count = atoi(a.substr(0,pos1).c_str()); //the count
    cout << count << endl;

    this->_items_count = count;
    this->_current_content = vector<char>((count/this->_int_char_size) + 1, 0);

    string b = a.substr(2,count);
    cout << "-" << b  <<"-" << endl;


    for (int i=0; i<b.size();i++) {
        if (b.at(i) == '0') {
        //if (true) {
            this->setItemAt(i,false);
        }else {
            this->setItemAt(i,true);
        }
    }

    //dodelat current volume a pozici!
     */
    string serial;
    serial.clear();
    serial = buffer;

    //cout << "A=" << serial << "=" << endl;
    stringstream sstr;
    sstr.clear();
    sstr.flush();
    sstr << serial;

    int myint;
    float myfloat;
    string rest;

    sstr >> myint;
    this->setItemsCount(myint);

    sstr >> myint;
    this->setCurrentPosition(myint);

    sstr >> myfloat;
    this->setCurrentVolume(myfloat);

    
    sstr >> rest;
    this->_current_content.resize((rest.size()/this->_int_char_size) + 1,0);


    for (int i=0;i< rest.size();i++) {
        if(rest.at(i) == '1') {
            this->setItemAt(i,true);
        } else {
            this->setItemAt(i,false);
        }
    }
    

    //cout << myint << ", " <<  2*myint << endl;


    return true;
}
