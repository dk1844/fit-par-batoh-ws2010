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

    /**
     * Creates a node
     * @param count vector size (# items in the cave)
     * @param position where are we at the vector, starting with 0 => next expanding will be done in the first (index 0) element (default:0)
     * @param value current value of the content (default:0)
     * @param volume current volume of the content (default:0)
     */
    Node(int count, int position = 0, float value = 0.0, float volume = 0.0);

    virtual ~Node();

    /**
     * Expanding a node into 2 child nodes.
     * Call it as expand ( &sem1, &sem2, &vektor_values, &vektor_volumes)
     * @param a first child node
     * @param b second child node
     * @param volumes vector of items' volumes
     */
    void expand(Node * a, Node * b, vector<float> * volumes);

    /**
     * Let's you know what the current value of the bag content is.
     * @return current value of the bag content
     */
    //float getCurrentValue();

    /**
     * Let's you know what the current volume of the bag content is.
     * @return current volume of the bag content
     */
    float getCurrentVolume();
    
    /**
     * calculates bag content's value
     * @param allItemsCount how many item there are in general (not only in the bag)
     * @param values vector of items' values
     * @return total value of items
     */
    float calculateValue(int allItemsCount, vector<float> * values);

    /**
     * Inspects whether a given item is inside the bag (leaf)
     * @param index index of the item
     * @return true if the item is inside of the bag, false otherwise
     */
    bool isItemInside(int index);

    /**
     * Let's you know which index is gonna be expanded next time.
     * @return position in the vector = depth of the tree.
     */
    int getCurrentPosition();

    /**
     * Gives you the "binary" vector representing the content of the bag
     * @return vector what's-inside
     */
    vector<int> getCurrentContent();

    /**
     * Describes whether this node can be further expanded
     * @return false on a leaf (no further expansion possible), false otherwise
     */
    bool isExpandable();

    /**
     * Prints info about a node.
     * Namely: content, value & volume, depth and expansion capability.
     */
    void print();

private:
    float _current_volume; //volume taken
    int _current_position; // 0..n-1
    vector<int> _current_content;


};

#endif	/* NODE_H */

