#include<iostream>
#include<fstream>
#include<string>
#include<iomanip>
#include <ctype.h>
#include <bits/stdc++.h>

using namespace std;

class Scheduling{
public:

    class Node{
    public:
        int jobId, jobTime, dependentCount;
        Node* next;

        Node(){}

        Node(int jobId, int jobTime, int dependentCount){
            this->jobId = jobId;
            this->jobTime = jobTime;
            this->dependentCount = dependentCount;
        }

        void printNode(Node* node, ofstream& outFile){
            outFile<< "(" <<node->jobId <<", " <<node->jobTime <<", " <<node->dependentCount <<")";
        }
    };

    class JOBS{
    public:
        int jobTime, onWhichProc, onOpen, parentCount, dependentCount;

        JOBS(){}
    };

    class Proc{
    public:
        int doWhichJob = -1, timeRemain;
    };

    int numNodes, numProcs , procUsed, totalJobTimes, currentTime;
    JOBS* jobAry;
    Proc* procAry;
    Node* OPEN;
    int** adjMatrix;
    int** scheduleTable;
    int* parentCountAry, *dependentCountAry, *onGraphAry;

    void initialization(ifstream& inFile1, ifstream& inFile2){
        procUsed = 0;
        currentTime = 0;
        OPEN = new Node();
        inFile1>> numNodes;
        adjMatrix = new int*[numNodes+1];
        for(int i = 0; i < numNodes+1; i++){
            adjMatrix[i] = new int[numNodes+1];
            for(int j = 0; j<numNodes+1; j++){
                adjMatrix[i][j]=0;
            }
            parentCountAry[i] = 0;
            dependentCountAry[i] = 0;
            onGraphAry[i] = 0;
        }
        jobAry = new JOBS[numNodes+1];
        procAry = new Proc[numProcs+1];
        scheduleTable = new int*[numProcs+1];
        for(int i = 0; i<numProcs+1; i++){
            scheduleTable[i] = new int[totalJobTimes+1];    //Possibly initialize all to zero
        }
        loadMatrix(inFile1);
        computeParentCount(adjMatrix, parentCountAry);
        computeDependentCount(adjMatrix, dependentCountAry);
        totalJobTimes = constructJobAry(inFile2, adjMatrix);
    }

    void loadMatrix(ifstream& inFile){
        int r = 0, c = 0;
        while(!inFile.eof()){
            inFile>>r >>c;
            adjMatrix[r][c] = 1;
        }
    }

    void loadProcAry(){
        int availProc, jobId, jobTime;
        Node* newJob = new Node();
        while(true){
            availProc = findProcessor();
            if(availProc>0 && OPEN->next!=NULL && procUsed<numProcs){
                procUsed++;
                newJob = OPEN->next;
                OPEN->next = OPEN->next->next;
                jobId = newJob->jobId;
                jobTime = newJob->jobTime;
                procAry[availProc].doWhichJob = jobId;
                procAry[availProc].timeRemain = jobTime;
                putJobOnTable(availProc, currentTime, jobId, jobTime);
            }
            else break;
        }
    }

    int constructJobAry(ifstream& inFile, int **adjMatrix){
        int totalTime = 0;
        int nodeId, jobTime;
        while(!inFile.eof()){
            inFile>>nodeId >>jobTime;
            totalTime += jobTime;
            jobAry[nodeId].jobTime = jobTime;
            jobAry[nodeId].onWhichProc = -1;
            jobAry[nodeId].onOpen = 0;
            jobAry[nodeId].parentCount = parentCountAry[nodeId];
            jobAry[nodeId].dependentCount = dependentCountAry[nodeId];
        }
        return totalTime;
    }

    void computeParentCount(int **adjMatrix, int *parentCountAry){
        for(int i=1; i<=numNodes; i++){
            for (int j=1; j<=numNodes; j++){
				parentCountAry[j] += adjMatrix[i][j];
				jobAry[j].parentCount = parentCountAry[j];
            }
        }
    }

    void computeDependentCount(int **adjMatrix, int *dependentCountAry){
        for(int i=1; i<=numNodes; i++){
            for (int j=1; j<=numNodes; j++)
				dependentCountAry[i] += adjMatrix[i][j];
				jobAry[i].dependentCount = dependentCountAry[i];
        }
    }

    void loadOpen(){
         int orphanNode, jobId, jobTime;
        while(true){
            orphanNode = findOrphan();
            if(orphanNode > 0){
                jobId = orphanNode;
                jobTime = jobAry[jobId].jobTime;
                Node* newNode = new Node(jobId, jobTime, dependentCountAry[jobId]);
                listInsert(OPEN, newNode);
                jobAry[jobId].onOpen = 1;
            }
            else break;
        }
    }

    int findOrphan(){
        for(int i = 1; i<=numNodes; i++) {
			if(jobAry[i].parentCount<=0 && jobAry[i].onOpen==0 && jobAry[i].onWhichProc==0)
				return i;
		}
		return -1;
    }

    void listInsert(Node* listHead, Node* newNode){
        Node* spot = findSpot(listHead, newNode);
        newNode->next = spot->next;
        spot->next = newNode;
    }

    Node* findSpot(Node* listHead, Node* newNode){
        Node* spot = listHead;
        while(spot->next!=NULL && spot->next->dependentCount > newNode->dependentCount)
            spot = spot->next;
        return spot;
    }

    void printList(Node* listHead, ofstream& outFile){
        Node* current = listHead;
        outFile<<"listHead -> ";
        while(current->next!=NULL){
            current->printNode(current, outFile);
            current = current->next;
            outFile<<" -> ";
        }
        outFile<<" NULL\n";
    }

    void printScheduleTable(ofstream& outFile){
        outFile<<"       -";
			for (int i=0; i<=totalJobTimes; i++)
				outFile<<i <<"---";
			outFile<<endl;
			for (int i=1; i<=numProcs; i++) {
				outFile<<"P (" <<i <<") ";
				for (int j=1; j<=totalJobTimes; j++)
					outFile<<"| " <<scheduleTable[i][j] <<" ";
				outFile<<"\n      ";
				for (int j=1; j<=totalJobTimes; j++)
					outFile<<"----";
				outFile<<endl;
			}
    }

    int findProcessor(){
        for(int i=1; i<=numProcs; i++)
			if(procAry[i].timeRemain<=0)
				return i;
		return -1;
    }

    void putJobOnTable(int availProc, int currentTime, int jobId, int jobTime){
        int time = currentTime;
        int endTime = time + jobTime;
        while(time < endTime){
            scheduleTable[availProc][time] = jobId;
            time++;
        }
    }

    bool checkCycle(){
        if(OPEN->next==NULL && !graphIsEmpty() && findProcessor()==-1)
			return true;
		return false;
    }

    bool graphIsEmpty(){
        bool isEmpty = true;
        for (int i=1; i<=numNodes; i++)
            if (onGraphAry[i] != 0)
                isEmpty = false;
        return isEmpty;
    }

    void updateProcTime(){
        for (int i=1; i<=numProcs; i++)
			if (procAry[i].timeRemain>0)
				procAry[i].timeRemain--;
    }

    int findDoneProc(){
        int jobId;
        for (int i=1; i<=numProcs; i++) {
			if (procAry[i].doWhichJob>0 && procAry[i].timeRemain<=0) {
				jobId = procAry[i].doWhichJob;
				procAry[i].doWhichJob = -1;
				break;
			}
		}
		return jobId;
    }

    void deleteFinishedNodes(){
        int jobId;
        while(true){
            jobId = findDoneProc();
            if(jobId > 0){
                onGraphAry[jobId] = 0;
                deleteEdge(jobId);
            }
            else break;
        }
    }

    void deleteEdge(int jobId){
        for (int i=1; i<=numNodes; i++)
			if (adjMatrix[jobId][i]>0)
				parentCountAry[i]--;
    }
};

   int main(int argc, char* argv[]){

        ifstream inFile1;
        ifstream inFile2;
        ofstream outFile1;
        ofstream outFile2;

        string inFile1Name = argv[1];
        string inFile2Name = argv[2];
        string outFile1Name = argv[4];
        string outFile2Name = argv[5];

        bool hasCycle;
        int currentTime = 0;

        inFile1.open(inFile1Name.c_str());
        inFile2.open(inFile2Name.c_str());
        outFile1.open(outFile1Name.c_str());
        outFile2.open(outFile2Name.c_str());

        Scheduling* mySchedule = new Scheduling();
        std::stringstream convert{argv[3]};
        if (!(convert >> mySchedule->numProcs))
           mySchedule->numProcs = 0;
        mySchedule->initialization(inFile1, inFile2);
        if (mySchedule->numProcs > mySchedule->numNodes)
            mySchedule->numProcs = mySchedule->numNodes;
        while(!mySchedule->graphIsEmpty()){
            mySchedule->loadOpen();
            mySchedule->printList(mySchedule->OPEN, outFile2);
            mySchedule->loadProcAry();
            hasCycle = mySchedule->checkCycle();
            if(hasCycle){
                cout<<"There is a cycle in the graph!!!" <<endl;
                return 1;
            }
            mySchedule->printScheduleTable(outFile1);
            currentTime++;
            mySchedule->updateProcTime();
            mySchedule->deleteFinishedNodes();
        }
        mySchedule->printScheduleTable(outFile1);

        inFile1.close();
        inFile2.close();
        outFile1.close();
        outFile2.close();
        return 0;
    }
