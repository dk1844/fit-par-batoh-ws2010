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

#include "NodeStack.h"
#include <vector>

#include <iostream>  // I/O
#include <fstream>   // file I/O
#include <iomanip>   // format manipulation
#include <string>

#include <stdio.h>
#include <string.h>
#include <sstream>

#include "mpi.h"

#include "Node.h"
#include "NodeStack.h"

#define LENGTH 100
#define CHECK_MSG_AMOUNT 100

//tags
#define MSG_INIT_WORK    100 //INIT: init prace
#define MSG_INIT_NO_WORK 103 //INIT: Neni prace, malo vstupnich dat


#define MSG_WORK_REQUEST 200 //REQ: zadost o praci
#define MSG_WORK_SENT 202 //ANS: posilam cast zasobniku (praci)
#define MSG_WORK_NOWORK 203 //ANS: neni prace

#define MSG_TOKEN_WHITE 300 //: pesek a dal TBD
#define MSG_TOKEN_BLACK 301 

#define MSG_FINISH 400 //: Ukoncuji
#define MSG_MY_BEST 401 //: "Send me your best"
#define MSG_TERMINATE 401 //





using namespace std;
// global variables
NodeStack stack1;
int process_rank = 0;
int processes = 5;
char message[LENGTH];
int donator_next;
MPI_Status status;
Node best; // containing the right-now best leaf node.
float bestValue = 0;
int locals_received_count = 0;
int bufS = 100;
bool im_done_too;
 //declare vectors for input data
vector<float> volumes;
vector<float> values;
float bagSize;
int items_count;
int citac = 0; //
bool finished = false;
bool p0_token_already_sent = false;
//this falg will be used to hadle ADUV
bool data_given_to_next = false;
bool waiting_for_data = false;
bool token_received = false;
int max_data_requests = 0;
int no_failed_data_req = 0;

    //stack<Node> stack1;
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

void generateMessage(char * buffer, int &bufferSize){
    char message[100];
    stringstream sstr;
    Node akt = stack1.top();
    //get maximum number of nodes to send
    //this value will be between number of items and half of the nodes in stack
    int noItems = akt.getItemsCount();
    int halfOfStack = stack1.size() / 2;
    int itemsAdded = 0;
    int itemsToBeAdded = 0;
    int msgLenght = 0;
    int msgBuffer = 0;
    // generally, we will send same number of nodes as items count
    //if there is not enough nodes in stack, we will send on hlaf of stack
    if(halfOfStack < noItems) {
        itemsToBeAdded = halfOfStack;
    } else {
        itemsToBeAdded = noItems;
    }

    sstr.clear();
    sstr.flush();

    //insert number of Nodes in message
    //sstr << "&&" << stack1.size() << "&";

    for(int i = 0; i <= itemsToBeAdded; i++ ) {
        akt = stack1.top();
        akt.serialize(message,msgBuffer);

        //if my msg is smaller than free space in buffer, add it
        // -1 is saving space for last &
        if( msgBuffer + 1 < bufferSize - msgLenght - 1){
            sstr << "&" << message;
            stack1.pop();
            msgLenght+= strlen(message) + 1;
        } else {
            break;
        }
        sstr << "&";
    }
    string res = sstr.str();
    bufferSize = res.length();
    strcpy(buffer, res.c_str());
 }

/**
 * Loads data from filename into vectors volume and value
 * @param filename name of the file loading
 * @param volume vector containing floats with volumes
 * @param value vector containing floats with values
 */
void loadDataFromFile(char * filename, int * items_cnt, vector<float> * volume, vector<float> * value, float * bagSize, int pid) {

    ifstream fp_in; // declarations of stream fp_in
    fp_in.open(filename, ios::in); // open the streams

    //cout << fp_in << endl;
    if (!fp_in) {
        cerr << "P" << pid << ":File could not be opened" << endl;
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

    cout << "P" << pid << ": Loading data: (2x" << count << " values)." << endl;

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
void procedeNode(float * bestValue, Node root, Node * best, float * bagSize, vector<float> * volumes, vector<float> * values, NodeStack * stack1, int items_count) {
    if (root.isExpandable()) {
        Node a;
        Node b;
        root.expand(&a, &b, volumes);

        // predmet nepridan, takze a je urcite perspektivni.
        (*stack1).push(a);

        //b se jeste vejde?
        if (b.getCurrentVolume() > (*bagSize)) {

        } else {
            (*stack1).push(b);
        }

    } else {
        //je akt lepsim resenim?
        float aktValue = root.calculateValue(values);
        if ((*bestValue) <= aktValue) { //if equal, doent matter, just making sure, when e.g. the bag is too small and the solution si the bag with <0,0, .. ,0,0>
            (*best) = root;
            (*bestValue) = aktValue;
        }
    }
}

/**
 * Stack division for all processes
 * @param volumes vector of volumes
 * @param stack1 the stack
 * @param bagSize size of the bag
 * @param processes !important # processes to divide into
 * @return true if succesfully divided (ready for parallel), false for too little data for this many processors => serial job.
 */
bool initDivide(vector<float> * volumes, NodeStack * stack1, float bagSize, int processes) {
//bool initDivide(vector<float> * volumes, stack<Node> * stack1, float bagSize, int processes) {

    while ((*stack1).size() < processes && !(*stack1).empty()) {
        Node akt = (*stack1).top();
        (*stack1).pop(); //remove top

        if (akt.isExpandable()) {
            Node a, b;
            akt.expand(&a, &b, volumes);
            (*stack1).push(a);

            if (b.getCurrentVolume() > bagSize) {
                cout << "DEBUG=pre0: Volume of " << b.getCurrentVolume() << " would be too much, cutting BB-branch" << endl;
            } else {
                (*stack1).push(b);
            }

        } else {
            cout << "P0:Data too small -> serial job." << endl;
            return false; //only p0 solving!
        }
    }
    return true; //parallel ok
}



void receiveMessage () {
    //blocking receive
    bool check_next_message = true;
    Node thisProot(items_count);
    
    while(check_next_message){
        MPI_Recv(message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        switch (status.MPI_TAG) {
                case MSG_WORK_REQUEST: // zadost o praci, prijmout a dopovedet
                    // zaslat rozdeleny zasobnik a nebo odmitnuti MSG_WORK_NOWORK
                    cout << "P" << process_rank << ": WORK_REQUEST_RECEIVED from P" << status.MPI_SOURCE << endl;

                    //if my stack is empty, send no_work_request
                    if(stack1.empty()==true){
                        MPI_Send(message, strlen(message) + 1, MPI_CHAR, status.MPI_SOURCE, MSG_WORK_NOWORK, MPI_COMM_WORLD);
                    } else {

                        Node akt = stack1.top();
                        stack1.pop(); //remove top
                        akt.serialize(message, bufS);
                        cout << "P" << process_rank << ": data has been sent to P" << status.MPI_SOURCE << endl; 
                        MPI_Send(message, strlen(message) + 1, MPI_CHAR, status.MPI_SOURCE, MSG_WORK_SENT, MPI_COMM_WORLD);

                        if (process_rank > status.MPI_SOURCE) {
                            data_given_to_next = true;
                        }
                    }
                    break;
                case MSG_WORK_SENT: // prisel rozdeleny zasobnik, prijmout
                    // deserializovat a spustit vypocet

                    cout << "P" << process_rank << ":Data received from P" << status.MPI_SOURCE << ": message:" << message << endl;

                    thisProot.deserialize(message, bufS);
                    //put my root to stack
                    stack1.push(thisProot);
                    //I have new data so I can start calculation again
                    waiting_for_data = false;
                    //erase counter of unseccessful tries for data
                    no_failed_data_req = 0;
                    break;
                case MSG_WORK_NOWORK: // odmitnuti zadosti o praci
                    // zkusit jiny proces
                    // a nebo se prepnout do pasivniho stavu a cekat na token
                    cout << "P" << process_rank << ":NO WORK Received from from P" << status.MPI_SOURCE << ":" << " Request for work sent to P" << donator_next << endl;
                    
                    //there is only limited unsuccessful requests for work
                    if(max_data_requests >= no_failed_data_req ){
                        MPI_Send(message, strlen(message) + 1, MPI_CHAR, donator_next, MSG_WORK_REQUEST, MPI_COMM_WORLD);
                        waiting_for_data=true;
                        no_failed_data_req++;

                        //calculation of next donator
                        donator_next = (donator_next +1) % processes;

                        //prohibit to ask myself for work
                        if (donator_next==process_rank) {
                            donator_next = (donator_next +1) % processes;
                        }
                     } else {
                        cout << "P" << process_rank << ": I cannot ask for other work... waiting" << endl;
                     }

                    break;

                case MSG_TOKEN_WHITE:
                case MSG_TOKEN_BLACK:  //ukoncovaci token, prijmout a nasledne preposlat
                // - bily nebo cerny v zavislosti na stavu procesu

                int msgToSend;
                if (process_rank != 0) {
                    //MPI_Recv(&message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    //if i sent data to another proces, I need to send black token and reset data_given flG
                    if(data_given_to_next) {
                        msgToSend = MSG_TOKEN_BLACK;
                        data_given_to_next = false;
                    } else {
                        //If iam white I dont care about token value and send it to next P
                        msgToSend = status.MPI_TAG;
                    }
                    cout << "P" << process_rank << ": TOKEN SENT:" << msgToSend << endl;
                    MPI_Send(message, strlen(message) + 1, MPI_CHAR, (process_rank + 1) % processes, msgToSend, MPI_COMM_WORLD);
                } else {
                    //this is zero, received token and..
                    //MPI_Recv(&message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                    //is it over?
                    if (status.MPI_TAG == MSG_TOKEN_WHITE) {
                        //its over.
                        // I can quit calculation loop
                        finished=true;
                    } else {
                        //someone was black. do yet another round.
                        MPI_Send(message, strlen(message) + 1, MPI_CHAR, (process_rank + 1) % processes, MSG_TOKEN_WHITE, MPI_COMM_WORLD);
                    }
                }
                    break;

                case MSG_FINISH: //konec vypoctu - proces 0 pomoci tokenu zjistil, ze jiz nikdo nema praci
                    //a rozeslal zpravu ukoncujici vypocet
                    //mam-li reseni, odeslu procesu 0
                    //nasledne ukoncim spoji cinnost
                    //jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
                    if (process_rank != 0) {
                        cout << "P" << process_rank << ":" << "OK, here it is, the LOCAL! solution seems to be"
                                << "with the best value of :" << bestValue << "." << endl;
                        //best.print(process_rank);
                    } else {
                        cerr << "P" << process_rank << ":" << "P0 should never receive MSG_FINISH!" << endl;
                    }

                    //best mam.
                    best.serialize(message, bufS);
                    cout << "P" << process_rank << ":" << " sending my best to P0!" << endl;
                    MPI_Send(message, strlen(message) + 1, MPI_CHAR, 0, MSG_MY_BEST, MPI_COMM_WORLD);
                    MPI_Barrier(MPI_COMM_WORLD);
                    MPI_Finalize();
                    exit(0);
                    break;

                case MSG_MY_BEST:
                    if (process_rank == 0) {
                        // receive and try to figure out, who's the best..
                        cout << "P" << process_rank << ":FINALE received: " << message  << "  from P" << status.MPI_SOURCE << endl;

                        Node akt;
                        akt.deserialize(message, bufS);

                        //is akt better than my best?
                        if (akt.calculateValue(&values) > bestValue) {

                            best = akt;
                            bestValue = best.calculateValue(&values);
                            //cout << "P0: best changed to " << bestValue << endl;
                            best.print(process_rank);
                        }
                    } else {
                        cerr << "P" << process_rank << ":" << "Pnot0 should never receive MSG_MY_BEST!" << endl;
                    }

                    locals_received_count++;

                    //did everything call home?
                    if (locals_received_count == processes - 1) {
                        
                        finished = true;
                    }
                    
                    break;
                default:
                    cerr << "unrecognized message type" << endl;
                    break;
            }
        //if I dont need to wait for the data, I can return to counting
        if(waiting_for_data==false){
            check_next_message=false;
        }

        //if im P0 and calculation is over, so ia dont care about waiting for data
        if(process_rank==0 && finished==true){
            check_next_message = false;
        }
    }
  }

int main(int argc, char** argv) {

    /**/
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);

    
    double t1 = 0;
    sprintf(message, "There are %d processes.\n", processes); //==message
    if (process_rank == 0) {
        printf("P%d:%s", process_rank, message); //P0 saying how many processes there are
        MPI_Barrier (MPI_COMM_WORLD);
        t1 = MPI_Wtime();
    }

    //check arguments
    if (argc != 2) {
        cerr << "# arguments found = " << (argc - 1) << endl;
        cerr << "USAGE : \"" << argv[0] << " <filename>" << endl;
        return 3;
    }

    donator_next = (process_rank + 1) % processes;
    
    max_data_requests = processes / 2;
    stack1.setBehavior(NodeStack::BEHAVIOR_STACK);


    //load source date from file
    loadDataFromFile(argv[1], &items_count, &volumes, &values, &bagSize, process_rank);

    //whether to run parallel
    bool isParallel;
  
    Node thisProot(items_count);

    if (process_rank == 0) {
        //  try to divide
        stack1.push(thisProot);
        isParallel = initDivide(&volumes, &stack1, bagSize, processes);
    }


    if (process_rank == 0) {

        if (isParallel) {
            cout << "P0:processes expanded to #" << stack1.size() << endl;

            for (int i = 1; i < processes; i++) {
                Node akt = stack1.top();
                stack1.pop(); //remove top
                int bufS = 100;
                akt.serialize(message, bufS);
                MPI_Send(message, strlen(message) + 1, MPI_CHAR, i, MSG_INIT_WORK, MPI_COMM_WORLD);
            }

        } else {
            //not parallel
            for (int i = 1; i < processes; i++) {
                MPI_Send(message, strlen(message) + 1, MPI_CHAR, i, MSG_INIT_NO_WORK, MPI_COMM_WORLD);
            }
        }

    } else {
        //procesor not 0 - Pi is waiting for work from P0
        MPI_Barrier (MPI_COMM_WORLD);
        MPI_Recv(message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        

        if (status.MPI_TAG == MSG_INIT_WORK) {
            cout << "P" << process_rank << ": received: " << message << endl;
            cout << "P" << process_rank << ": Starting calculation" << endl;
            //data recieved from P0, I can start with calculation
            isParallel = true;
            int BS = 100;
            //take data from message
            thisProot.deserialize(message, BS);

            //put my root to stack
            stack1.push(thisProot);

        } else if (status.MPI_TAG == MSG_INIT_NO_WORK) {
            //there is no work for me, i can go to the pub and take some beers...
            isParallel = false;
            cout << "P" << process_rank << ": serial job = i'm done." << endl;

            MPI_Finalize();
            return 0;
        } else {
            //unexpected message
            cerr << "P" << process_rank << ": init msg error" << endl;
        }
    }
    //init done.
    //controls for paralel program
    int flag;

    //PID of next donator of data. Need to be init after creating of proceses
    
    cout << "P" << process_rank << ": My next donator:" << donator_next << endl;
    if (isParallel) { /**/
        while (!finished) //ukoncime jen, kdyz bude stack prazdny a finished true.
        {
            //work...
            if (!stack1.empty()) {
                Node akt = stack1.top();
                stack1.pop(); //remove top
                procedeNode(&bestValue, akt, &best, &bagSize, &volumes, &values, &stack1, items_count);
            } else {
                // if iam p0, and i havent sent token yet, do it now.
                if (process_rank == 0 && p0_token_already_sent==false) {

                    MPI_Send(message, strlen(message) + 1, MPI_CHAR, (process_rank +1) % processes, MSG_TOKEN_WHITE, MPI_COMM_WORLD);
                    p0_token_already_sent=true;
                    cout << "P" << process_rank << ":" << " Token sent to  " << donator_next << endl;
                }
                    //iam out of work... so I send request to another process
                    cout << "P" << process_rank << ":" << " Request for work sent to P" << donator_next << endl;
                    MPI_Send(message, strlen(message) + 1, MPI_CHAR, donator_next, MSG_WORK_REQUEST, MPI_COMM_WORLD);
                    waiting_for_data=true;
                    no_failed_data_req++;
                    //calculation of next donator
                    donator_next = (donator_next +1) % processes;

                    //prohibit to ask myself for work
                    if (donator_next==process_rank) {
                        donator_next = (donator_next +1) % processes;
                    }
                    receiveMessage();
                    citac = 0;
            }
            citac++; //inc. couter when we reach check_msg_amount, we will check for messages

            if ((citac % CHECK_MSG_AMOUNT) == 0) {
                //if there is message in queue, receive it
                //cout << "P" << process_rank << ": Couter full, im going to check messages" << endl;
                MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
                if (flag) {
                    receiveMessage();
                }
                //protection against overflow of int during long calculations
                citac=0;
            }
        }
        //finished flag is true. I send finish message to all procesess
         for (int i = 1; i < processes; i++) {
             MPI_Send(message, strlen(message) + 1, MPI_CHAR, i, MSG_FINISH, MPI_COMM_WORLD);
             cout << "P" << process_rank << ":" << " FINISH MESSAGE TO: " << "P" << i << endl;
         }
        //isParallel part done
    } else {
        //it a serial job
        cout << "P" << process_rank << ":" << "Performing a serial job, cuz the data is too small to bother :)" << endl;
        //work!
        while (!stack1.empty()) {
            Node akt = stack1.top();
            stack1.pop(); //remove top
            procedeNode(&bestValue, akt, &best, &bagSize, &volumes, &values, &stack1, items_count);
        }
    }

    //all threads are finished with work and the winner is known:
    //I will return with P0 into messageReceive cycle and keep it here using waiting_for_data and finished
    //until all data are collected
    waiting_for_data = true;
    finished = false;
    receiveMessage();
    
    cout << "P" << process_rank << ":" << "solution with the value of " << best.calculateValue(&values) << " is:" << endl;
    best.print(process_rank);

    //no one (expcept P0) should get here!
    if (process_rank != 0) {
        cerr << "P" << process_rank << ":" << "noone should reach the endpoint!" << endl;
    }

    MPI_Barrier (MPI_COMM_WORLD);
    double t2 = MPI_Wtime();
    cout << "Spotrebovany cas je: " << t2 - t1 << endl;

    MPI_Finalize();
    return 0;
}
