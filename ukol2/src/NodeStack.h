/* 
 * File:   NodeStack.h
 * Author: Daniel
 *
 * Created on 4. listopad 2010, 13:41
 */

#include "Node.h"
#include <stack>


#ifndef NODESTACK_H
#define	NODESTACK_H

using namespace std;

class NodeStack {
public:
    static const int BEHAVIOR_STACK = 321;
    static const int BEHAVIOR_QUEUE = 123;

    /**
     * Constructor, sets default behaviour
     */
    NodeStack();
    NodeStack(const NodeStack& orig);
    virtual ~NodeStack();
    
    /**
     * empty() wrapper around stack/queue methods
     * @return true if empty, false otherwise
     */
    bool empty();
    /**
     * size() wrapper around stack/queue methods
     * @return number of Nodes inside
     */
    int size();
    /**
     * top/front() wrapper around stack/queue methods. 
     * @return Node on the top or at the front.
     */
    Node top();
   /**
    * pop/pop_front wrapper around stack/queue methods. removes the first/top Node
    */
    void pop();

    /**
     * wrapper around stack/queue methods. Behaving as a stack the new node is iserted on the top (front), ohterwise at the end/bottom when queue-like
     * @param
     */
    void push(const Node& );

    /**
     * define, wheter to act as stack or queue.
     * @param beh BEHAVIOR_STACK | BEHAVIOR_QUEUE
     */
    void setBehavior(int beh = BEHAVIOR_STACK);
    /**
     * learn, wheter acting as stack or queue.
     * @return BEHAVIOUR_STACK | BEHAVIOUR_QUEUE
     */
    int  getBehavior();

    /**
     * auxiliary method to getBehavior.
     * @return true if stack-like, false otherwise
     */
    bool isBehaviorStack();
    /**
     * auxiliary method to getBehavior.
     * @return true if queue-like, false otherwise
     */
    bool isBehaviorQueue();

    
    
private:
    int _behavior;  //DFS or BFS; DFS is default
    deque<Node> mystack;
};

#endif	/* NODESTACK_H */

