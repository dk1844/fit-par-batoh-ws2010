/* 
 * File:   NodeStack.cpp
 * Author: Daniel
 * 
 * Created on 4. listopad 2010, 13:41
 */

#include <stack>
#include "NodeStack.h"

NodeStack::NodeStack() {
    _behavior == BEHAVIOR_STACK;
}

NodeStack::NodeStack(const NodeStack& orig) {
    _behavior == BEHAVIOR_STACK;
}

NodeStack::~NodeStack() {
}

bool NodeStack::empty() {
    return mystack.empty();
}

void NodeStack::pop() {
    mystack.pop_front(); 
}

Node NodeStack::top(){
    return mystack.front(); 
}

void NodeStack::push(const Node& node) {
    if (_behavior == BEHAVIOR_STACK ) {
        mystack.push_front(node);   //stack like
    } else {
        mystack.push_back(node);    //queue like
    }
}

int NodeStack::size() {
    return mystack.size();
}

void NodeStack::setBehavior(int beh) {
    if(beh == BEHAVIOR_QUEUE) {
        _behavior = BEHAVIOR_QUEUE;
    } else {
        _behavior = BEHAVIOR_STACK;
    }
}

int NodeStack::getBehavior() {
    return _behavior;
}

bool NodeStack::isBehaviorQueue() {
    return (_behavior == BEHAVIOR_QUEUE);
}

bool NodeStack::isBehaviorStack() {
    return (_behavior == BEHAVIOR_STACK);
}