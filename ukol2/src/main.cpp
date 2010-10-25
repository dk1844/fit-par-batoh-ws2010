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

    cout << "Loading data: (2x" << count << " values)." << endl;

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

void procedeNode(float * bestValue, Node root, Node * best, float * bagSize, vector<float> * volumes, vector<float> * values, stack<Node> * stack1, int items_count) {
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

        if (b.getCurrentVolume() > (*bagSize)) {
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

/**
 * Stack division for all processes
 * @param volumes vector of volumes
 * @param stack1 the stack
 * @param bagSize size of the bag
 * @param processes !important # processes to divide into
 * @return true if succesfully divided (ready for parallel), false for too little data for this many processors => serial job.
 */
bool initDivide(vector<float> * volumes, stack<Node> * stack1, float bagSize, int processes) {

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
    if (process_rank == 0) {
        printf("P%d:%s", process_rank, message); //P0 saying how many processes there are
    }

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

    //load source date from file
    loadDataFromFile(argv[1], &items_count, &volumes, &values, &bagSize);

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
            //strcpy("shutdown this thread, not parallel :)", message);
            for (int i = 1; i < processes; i++) {
                MPI_Send(message, strlen(message) + 1, MPI_CHAR, i, MSG_INIT_NO_WORK, MPI_COMM_WORLD);
            }
        }

    } else {
        //procesor not 0
        MPI_Recv(message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        cout << "P" << process_rank << ": received: " << message << endl;

        if (status.MPI_TAG == MSG_INIT_WORK) {
            //everything ok.
            int BS = 100;
            thisProot.deserialize(message, BS);
            //thisProot.print(process_rank);
            //cout << "P" << process_rank << ":look this is what i got:" << endl;
            stack1.push(thisProot);
        } else if (status.MPI_TAG == MSG_INIT_NO_WORK) {
            isParallel = false;
            cout << "P" << process_rank << ": serial job = i'm done." << endl;

            MPI_Finalize();
            return 0;
        } else {
            cerr << "P" << process_rank << ": init msg error" << endl;
        }
    }

    //init done.
    cout << "P" << process_rank << ": Init done. Everyone knows what to do." << endl;

    //now they have to "work" and check for messages (MSG_WANT_WORK,MSG_TERM)
    //and eventually send some (MSG_SENDING_WORK, MSG_NO_WORK, MSG_MY_BEST
    //tzn. pouzivat neblokujici I fce.



    Node best; // containing the right-now best leaf node.
    float bestValue = 0;
    //TODO load balancing

    //controls for paralel program
    int citac = 0;
    int flag;
    bool finished = false;
    int bufS = 100;
    bool im_done_too;
    int locals_received_count = 0;
    //this falg will be used to hadle ADUV
    bool data_given_to_next = false;
    //PID of next donator of data. Need to be init after creating of proceses
    int donator_next;

    if (isParallel) { /**/
        while (!stack1.empty() || !finished) //ukoncime jen, kdyz bude stack prazdny a finished true.
        {
            citac++;
            if ((citac % CHECK_MSG_AMOUNT) == 0) {
                MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
                if (flag) {
                    //prisla zprava, je treba ji obslouzit
                    //v promenne status je tag (status.MPI_TAG), cislo odesilatele (status.MPI_SOURCE)
                    //a pripadne cislo chyby (status.MPI_ERROR)
                    switch (status.MPI_TAG) {
                        case MSG_WORK_REQUEST: // zadost o praci, prijmout a dopovedet
                            // zaslat rozdeleny zasobnik a nebo odmitnuti MSG_WORK_NOWORK
                            break;
                        case MSG_WORK_SENT: // prisel rozdeleny zasobnik, prijmout
                            // deserializovat a spustit vypocet
                            break;
                        case MSG_WORK_NOWORK: // odmitnuti zadosti o praci
                            // zkusit jiny proces
                            // a nebo se prepnout do pasivniho stavu a cekat na token
                            break;

                        case MSG_TOKEN_WHITE:
                        case MSG_TOKEN_BLACK: //ukoncovaci token, prijmout a nasledne preposlat
                            // - bily nebo cerny v zavislosti na stavu procesu
                            im_done_too = stack1.empty(); // somehow to figure out later..

                            int msgToSend;
                            if (status.MPI_TAG == MSG_TOKEN_WHITE && im_done_too) {
                                msgToSend = MSG_TOKEN_WHITE;
                            } else {
                                msgToSend = MSG_TOKEN_BLACK;
                            }

                            if (process_rank != 0) {
                                MPI_Recv(&message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                                MPI_Send(message, strlen(message) + 1, MPI_CHAR, (process_rank + 1) % processes, msgToSend, MPI_COMM_WORLD);
                            } else {
                                //this is zero, received token and..
                                MPI_Recv(&message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                                //is it over?
                                if (status.MPI_TAG == MSG_TOKEN_WHITE) {
                                    //its over.
                                    for (int i = 1; i < processes; i++) {
                                        MPI_Send(message, strlen(message) + 1, MPI_CHAR, i, MSG_FINISH, MPI_COMM_WORLD);
                                    }
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
                            MPI_Send(message, strlen(message) + 1, MPI_CHAR, 0, MSG_MY_BEST, MPI_COMM_WORLD);

                            MPI_Finalize();
                            exit(0);
                            break;

                        case MSG_MY_BEST:
                            if (process_rank == 0) {
                                // receive and try to figure out, who's the best..

                                MPI_Recv(message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                                cout << "P" << process_rank << ":FINALE received: " << message << endl;

                                Node akt;
                                akt.deserialize(message, bufS);

                                //is akt better than my best?
                                if (akt.calculateValue(&values) > bestValue) {
                                    best = akt;
                                    //best.print(process_rank);
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
                }
            }


            //work!
            if (!stack1.empty()) {
                Node akt = stack1.top();
                stack1.pop(); //remove top
                procedeNode(&bestValue, akt, &best, &bagSize, &volumes, &values, &stack1, items_count);
            }

            //if (stack1.empty()) finished = true; //zatim
            if (process_rank == 0 && stack1.empty() && !data_given_to_next) { // to send this: 0, empty, not send before.
                data_given_to_next = true;
                MPI_Send(message, strlen(message) + 1, MPI_CHAR, (process_rank + 1) % processes, MSG_TOKEN_WHITE, MPI_COMM_WORLD);

                cout << "P" << process_rank << ":" << "LOCAL! solution with the value of " << best.calculateValue(&values) << " is:" << endl;
                best.print(process_rank);

            }

        }
        //isParallel part done
    } else {
        //it a serial job
        cout << "Performing a serial job, cuz the data is too small to bother :)" << endl;
        //work!
        while (!stack1.empty()) {
            Node akt = stack1.top();
            stack1.pop(); //remove top
            procedeNode(&bestValue, akt, &best, &bagSize, &volumes, &values, &stack1, items_count);
        }
    }



    //all threads are finished with work and the winner is known:
    cout << "P" << process_rank << ":" << "solution with the value of " << best.calculateValue(&values) << " is:" << endl;
    best.print(process_rank);




    //no one (expcept P0) should get here!
    if (process_rank != 0) {
        cerr << "P" << process_rank << ":" << "noone should reach the endpoint!" << endl;
    }

    MPI_Finalize();
    return 0;
}
