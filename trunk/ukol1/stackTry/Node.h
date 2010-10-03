/* 
 * File:   Node.h
 * Author: Daniel
 *
 * Created on 30. září 2010, 22:48
 */


#include <cstdlib>
#include <vector>
#include <iostream>


using namespace std;

#ifndef NODE_H
#define	NODE_H

class Node {
public:
    Node();
  //  Node(const Node& orig);
    Node( int count, int position = 0, float value = 0.0, float volume = 0.0);

    virtual ~Node();

    void expand( Node * a, Node * b, vector<float> * values, vector<float> * volume);

    float getCurrentValue();
    float getCurrentVolume();
    int getCurrentPosition();
    vector<int> getCurrentContent();
    bool isExpandable();

    void print();

private:
    float _current_value;
    float _current_volume; //taken
    int _current_position; // 0..n-1
    vector<int> _current_content;
    

};

#endif	/* NODE_H */

