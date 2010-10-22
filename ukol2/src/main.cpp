/*
 * File:   main.cpp
 * Author: Daniel Kavan
 *
 * Created on 30. září 2010, 18:29
 *
 *
 *
 * **PERKS**
 *
 *  * the current value of a bag used to be calculated when expanding (just as volume is).
 *    This has been changed, bc. we can save the time for computing value for variations of contents that are cut by the BB algorithm
 *  * Old way, how we stored nodes in stack was implemented using int data type.
 *    We change it to bit representation -> we use char instead of int and use every bit of char to
 *    signify, if the item is presented or not
 *    We save a lot of memory ->    int:                4bytes/item
 *                                  char(with bit rep.) 1bit/item
 */

#include <cstdlib>
#include <stack>
#include <vector>

#include <iostream>  // I/O
#include <fstream>   // file I/O
#include <iomanip>   // format manipulation
#include <string>

#include <stdio.h>
#include <string.h>

#include "mpi.h"

#include "Node.h"

#define LENGTH 100

using namespace std;

/**
 * Print the content of a vecor
 * @param vect1 pointer to a vector
 * @param name optional title
 */
template<class T> void printVector(vector<T> * vect1, char * name = "") {

    if (name != "") {
        cout << "Vector \"" << name << "\" content:" << endl;
    } else {
        cout << "Vector content:" << endl;
    }
    for (int b = 0; b < (* vect1).size(); b++) {
        cout << (*vect1)[b] << " ";
    }
    cout << endl;
}

/**
 * Loads data from filename into vectors volume and value
 * @param filename name of the file loading
 * @param volume vector containing floats with volumes
 * @param value vector containing floats with values
 */
void loadDataFromFile(char * filename, int * items_cnt, vector<float> * volume, vector<float> * value, float * bagSize) {

    ifstream fp_in; // declarations of stream fp_in
    fp_in.open(filename, ios::in); // open the streams

    //cout << fp_in << endl;
    if (!fp_in) {
        cerr << "File could not be opened" << endl;
        exit(1);
    }

    int count;
    float float_val;

    // get number of intems
    fp_in >> count; // input from file pointer or standard input
    if (!fp_in.good()) {
        cerr << "Wrong format" << endl;
        exit(1);
    }

    (*items_cnt) = count;

    //get size of back
    fp_in >> float_val; // input from file pointer or standard input
    if (!fp_in.good()) {
        cerr << "Wrong format" << endl;
        exit(1);
    }

    (*bagSize) = float_val;



    (*volume).resize(count);
    (*value).resize(count);

    cout << "Expecting 2x" << count << " values." << endl;

    for (int i = 0; i < count; i++) {
        fp_in >> float_val;
        if (!fp_in.good()) {
            cerr << "Wrong format inside" << endl;
            exit(1);
        }
        (*volume)[i] = float_val;

        fp_in >> float_val;
        if (!fp_in.good()) {
            cerr << "Wrong format inside" << endl;
            exit(1);
        }
        (*value)[i] = float_val;

    }

    float dummy;
    fp_in >> dummy;
    if (!fp_in.eof()) {
        cerr << "WARNING: file should have ended, skipping the rest.." << endl;
        // exit(1);
    }

    fp_in.close(); // close the streams
}
    /**
     * 
     * @param bestValue pointer to value of best actual solution
     * @param root - node, which will be solved
     * @param best - best node we found
     * @param bagSize - pointer to bagSize
     * @param volumes - pointer to volumes vector
     * @param values - pointer to values vector
     * @param stack1 - pointer to stack 0 we will add new nodes using this
     * @param items_count - # of items
     */

void procedeNode(float * bestValue, Node root,Node * best, float * bagSize, vector<float> * volumes, vector<float> * values, stack<Node> * stack1, int items_count) {
     if (root.isExpandable()) {
            Node a;
            Node b;
            root.expand(&a, &b, volumes);

            /*debug - start*/
           // cout << "---inner node---" << endl;
           // root.print();
            /*debug - end*/

            // predmet nepridan, takze a je urcite perspektivni.
            (*stack1).push(a);

            //b se jeste vejde?
         
            if (b.getCurrentVolume() > (*bagSize) ){
             //   cout << "DEBUG: Volume of " << b.getCurrentVolume() << " would be too much, cutting BB-branch" << endl;
            } else {
                (*stack1).push(b);
            }

        } else {
            // tree leaf
          //  cout << "========leaf======" << endl;
          //  root.print();

            //je akt lepsim resenim?
            float aktValue = root.calculateValue(values);
            if ((*bestValue) <= aktValue) { //if equal, doent matter, just making sure, when e.g. the bag is too small and the solution si the bag with <0,0, .. ,0,0>
                (*best) = root;
                (*bestValue) = aktValue;
            }
            //cout << "DEBUG: Current best value = " << (*bestValue) << endl;
        }

}

int main(int argc, char** argv) {
    
    int process_rank = 0;
    int processes = 5;
    char message[LENGTH];

    /**/
    MPI_Status status;
    

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);


    sprintf(message, "There are %d processes.\n", processes); //==message
    printf("P%d:%s", process_rank, message); //everybody saying how many processes there are
   

   //check arguments
    if (argc != 2) {
        cerr << "# arguments found = " << (argc - 1) << endl;
        cerr << "USAGE : \"" << argv[0] << " <filename>" << endl;
        return 3;
    }
    
    //declare vectors for input data
    vector<float> volumes;
    vector<float> values;
    float bagSize;
    int items_count;
    stack<Node> stack1;
    

    //controls for paralel program
    //this falg will be used to hadle ADUV
    bool data_given_to_next = false;
    //PID of next donator of data. Need to be init after creating of proceses
    int donator_next;

    //load source date from file
    loadDataFromFile(argv[1], &items_count, &volumes, &values, &bagSize);

    //debug only
//    cout << "Loaded bag size = " << bagSize << endl;
//    cout << "Loaded items count = " << items_count << endl;
//    printVector(&volumes, "Volume");
//    printVector(&values);

    //we can prepare data for other proceses here
    
    Node thisProot(items_count);

    if (process_rank == 0) {
        //divide
        
        stack1.push(thisProot);
        
        while(stack1.size() < processes && !stack1.empty()) {
            Node akt = stack1.top();
            stack1.pop(); //remove top

            if (akt.isExpandable()) {
                Node a, b;
                akt.expand(&a,&b,&volumes);
                stack1.push(a);

                if (b.getCurrentVolume() > bagSize ){
                   cout << "DEBUG=pre0: Volume of " << b.getCurrentVolume() << " would be too much, cutting BB-branch" << endl;
                } else {
                    stack1.push(b);
                }

            } else {
                cerr << "expand p0 error! DATA too small for too many processors." << endl;
                return 225;
            }

        }
        
        cout << "P0:processes expanded to #" << stack1.size() << endl;
        
        for (int i=1;i<processes;i++) {
            Node akt = stack1.top();
            stack1.pop(); //remove top
            int bufS = 100;
            akt.serialize(message, bufS);
            MPI_Send (message, strlen(message)+1, MPI_CHAR, i, 100, MPI_COMM_WORLD);
        }
        
        cout << "P0:and sent " << endl;
            
    } else { 
        //procesor not 0
        MPI_Recv(message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        cout << "P" << process_rank << ":msg received" << endl;
        int BS = 100;
        thisProot.deserialize(message,BS);
        //thisProot.print(process_rank);
        //cout << "P" << process_rank << ":look this is what i got:" << endl;
        stack1.push(thisProot);

    }
    
    

    Node best; // containing the right-now best leaf node.
    float bestValue = 0;

    while (!stack1.empty()) {
        Node akt = stack1.top();
        stack1.pop(); //remove top

        procedeNode(&bestValue,akt,&best,&bagSize,&volumes,&values,&stack1,items_count);

    } //while stack !empty end
    // the stack is empty now, everything has been tested and the winner is known:

   /**/
    cout << "P" << process_rank << ":msg received" << endl << "OK, here it is, the solution seems to be:"
         << "with the best value of " << bestValue << "." << endl;
    best.print(process_rank);
   

    /*
    int bufSize = 100;
    //char * buffer;
    //buffer = new char(bufSize);
    char buffer[LENGTH];

    best.serialize(buffer, bufSize);

    cout << "akt best serializovano na: <" << buffer << "~" << bufSize << ">" <<endl;
    

    Node new2;
    
    new2.deserialize(buffer,bufSize);

    new2.print();
     cout << "with the best value of " << new2.calculateValue(&values) << "." << endl;
*/
    //delete buffer;
    MPI_Finalize();
    return 0;
}
