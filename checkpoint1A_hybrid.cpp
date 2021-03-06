//
// Example Checkpoint - can be compiled anywhere
//
// Must have PIN_HOME environment variable set
// 


#include "pin.H"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <unistd.h> // for pid
#include <stdlib.h>

using namespace std;


/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
static INT32 Usage()
{
    cerr << "This pin tool collects a profile of jump/branch/call instructions for an application\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;
    return -1;
}
/* ===================================================================== */

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,         "pintool",
                            "o", "output.out", "specify profile file name");

KNOB<BOOL>   KnobPid(KNOB_MODE_WRITEONCE,                "pintool",
                            "i", "0", "append pid to output");

KNOB<UINT64> KnobBranchLimit(KNOB_MODE_WRITEONCE,        "pintool",
                            "l", "0", "set limit of dynamic branches simulated/analyzed before quit");

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */
UINT64 CountSeen = 0;
UINT64 CountTaken = 0;
UINT64 CountCorrect = 0;
UINT64 CountReplaced = 0;
UINT64 BTBHit = 0;
UINT64 BTBMiss = 0;

unsigned int HistoryReg_Gshare;
unsigned int BTB_History;
unsigned int BTB_HistoryLength;
unsigned int PHT_Table[4096];
//NEW
unsigned int PHT_Table_gshare[1024];


/* ===================================================================== */
/* Branch predictor                                                      */
/* ===================================================================== */
UINT64 mask = 0x03FF;
UINT64 mask_pag = 0xFFF;
#define BTB_SIZE 1024

struct entry_one_bit
{
    bool valid;
    bool prediction;
    UINT64 tag;
    UINT64 ReplaceCount;
    UINT64 HistoryReg;
    UINT8 counter;
} BTB[BTB_SIZE];

/* initialize the BTB */
VOID BTB_init()
{
    int i;

    for(i = 0; i < BTB_SIZE; i++)
    {
        BTB[i].valid = false;
        BTB[i].prediction = false;
        BTB[i].tag = 0;
        BTB[i].ReplaceCount = 0;
	BTB[i].HistoryReg = 0;
        BTB[i].counter = 0;
    }

  // HINT: in the case of the two-bit predictor, you would just need to update the prediction
  //  field to implement the two-bit automata

  // HINT: in the case of GAg, you will need to add a history register and history table
  //  something like:

    //BTB_History = 0;
    BTB_HistoryLength = 10;
    HistoryReg_Gshare = 0;
  

    
    int j;

    for (j = 0; j < 1024; j++){
          
   
      PHT_Table_gshare[j] = 2;
  
    }

    int k;

    for (k = 0; k < 4096; k++){
          
      PHT_Table[j] = 2;
     
  
    }

  // the table would be initialized to 2 for weak taken, and
  // you would access the table using:
  //
  //    prediction = BTB_Table[BTB_History & 0xFF];
  //    if (prediction > 1)
  //      prediction is taken
  //    else 
  //      prediction is not taken
  //
  // note: the code above only handles an 8-bit history
 
  // HINT: in the case of PAg, you will need to add a field to the BTB structure
  // to maintain the history for the branch of that entry
}

/* see if the given address is in the BTB */
bool BTB_lookup(ADDRINT ins_ptr)
{
    UINT64 index;

    index = mask & ins_ptr;

    if(BTB[index].valid){
        if(BTB[index].tag == ins_ptr){
	    BTBHit++;
            return true;
	}
    }
    BTBMiss++;
    return false;
}

/* return the prediction for the given address */
bool BTB_prediction(ADDRINT ins_ptr)
{

    UINT64 index;

    index = mask & ins_ptr; 

    unsigned int predict;
    unsigned int predict_pag;
    unsigned int predict_gshare;
    unsigned int counter_value;

    predict_pag = PHT_Table[(BTB[index].HistoryReg & 0xFFF)]; 
    predict_gshare = PHT_Table_gshare[(index ^ (HistoryReg_Gshare & 0x3FF))];  	

    counter_value = BTB[index].counter;
    if (counter_value > 1){
	predict = predict_pag;
       
    }
    else
  	predict = predict_gshare;  
      
    if (predict > 1)
        return true;
    else 
        return false;
  
}

/* update the BTB entry with the last result */
VOID BTB_update(ADDRINT ins_ptr, bool taken)
{
    UINT64 index;
    index = mask & ins_ptr;

    int counter_pag;
    int counter_gshare;
    
    counter_pag =  PHT_Table[BTB[index].HistoryReg & 0xFFF];  
    counter_gshare = PHT_Table_gshare[(index ^ (HistoryReg_Gshare & 0x3FF))];  	
    

    if(taken){
  
      counter_pag++; 
      if(counter_pag > 3)
        counter_pag = 3;
    
     }
     else{
      counter_pag--; 
       if(counter_pag < 0)
         counter_pag = 0;  
             
     }

    if(taken){
  
     
      counter_gshare++;
      if(counter_gshare > 3)
        counter_gshare = 3;
    
     }
     else{
    
      counter_gshare--;
       if(counter_gshare < 0)
         counter_gshare = 0;  
             
     }
      
     PHT_Table[BTB[index].HistoryReg & 0xFFF] = counter_pag;
     BTB[index].HistoryReg = (BTB[index].HistoryReg << 1) | taken;      
 
     PHT_Table_gshare[(index ^ (HistoryReg_Gshare & 0x3FF))] = counter_gshare;
     
     HistoryReg_Gshare = (HistoryReg_Gshare << 1) | taken;

     //Meta Predictor update logic
     unsigned int pag_prediction;
     unsigned int gshare_prediction;

     bool pag_bool;
     bool gshare_bool;

     pag_prediction=PHT_Table[BTB[index].HistoryReg & 0xFFF];
     gshare_prediction=PHT_Table_gshare[(index ^ (HistoryReg_Gshare & 0x3FF))];
    
     if(pag_prediction > 1)
     	pag_bool = true;

     if(gshare_prediction > 1)
	gshare_bool = true;

     unsigned int counter_meta; 
     counter_meta=BTB[index].counter;
     if(pag_prediction != gshare_prediction){
	if(gshare_bool == taken){
		counter_meta++;
			if(counter_meta > 3)
				counter_meta=3;
	}
	else if(pag_bool == taken)
		counter_meta--;
			if(counter_meta < 0)
			    counter_meta=0;

    }

    BTB[index].counter = counter_meta;

 
}

/* insert a new branch in the table */
VOID BTB_insert(ADDRINT ins_ptr)
{
    UINT64 index;

    index = mask & ins_ptr;

    if(BTB[index].valid)
    {
        BTB[index].ReplaceCount++;
        CountReplaced++;
    }

    BTB[index].valid = true;
    BTB[index].prediction = true;  // Missed branches always enter as taken/true
    BTB[index].tag = ins_ptr;
    
    //BTB[index].counter = 2;
   // BTB_update(ins_ptr,true);
    BTB[index].HistoryReg = (BTB[index].HistoryReg << 1) | 1;
    PHT_Table[BTB[index].HistoryReg & 0xFFF] = 2;
	
    HistoryReg_Gshare = (HistoryReg_Gshare << 1) | 1;
    PHT_Table_gshare[(index ^ (HistoryReg_Gshare & 0x3FF))] = 2;  	
    
 
}

/* ===================================================================== */


/* ===================================================================== */

VOID WriteResults(bool limit_reached)
{
    int i;

    string output_file = KnobOutputFile.Value();
    if(KnobPid) output_file += "." + decstr(getpid());
    
    std::ofstream out(output_file.c_str());

    if(limit_reached)
        out << "Reason: limit reached\n";
    else
        out << "Reason: fini\n";
    out << "Count Seen: " << CountSeen << endl;
    out << "Count Taken: " << CountTaken << endl;
    out << "Count Correct: " << CountCorrect << endl;
    out << "Count Replaced: " << CountReplaced << endl;
    out << "Prediction Accuracy: " << (float) CountCorrect/CountSeen << endl;
    out << "Total BTB Misses: " << BTBMiss << endl;
    out << "Total BTB Hits: " << BTBHit << endl;
    
    for(i = 0; i < BTB_SIZE; i++)
    {
        out << "BTB entry: " << i << ";" << BTB[i].valid << ";" << BTB[i].ReplaceCount << endl;
    }
    out.close();
}

/* ===================================================================== */

VOID br_predict(ADDRINT ins_ptr, INT32 taken)
{
    CountSeen++;
    if (taken)
        CountTaken++;

    
    if(BTB_lookup(ins_ptr)) 
     {
        if(BTB_prediction(ins_ptr) == taken) CountCorrect++;
        BTB_update(ins_ptr, taken);
     }
     else
     {
        if(!taken) CountCorrect++;
        else BTB_insert(ins_ptr);
     }
    
   
    if(CountSeen == KnobBranchLimit.Value())
    {
        WriteResults(true);
        exit(0);
    }
} 


//  IARG_INST_PTR   
// ADDRINT ins_ptr

/* ===================================================================== */

VOID Instruction(INS ins, void *v)
{

// The subcases of direct branch and indirect branch are
// broken into "call" or "not call".  Call is for a subroutine
// These are left as subcases in case the programmer wants
// to extend the statistics to see how sub cases of branches behave

    if( INS_IsRet(ins) )
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) br_predict, 
            IARG_INST_PTR, IARG_BRANCH_TAKEN,  IARG_END);
    }
    else if( INS_IsSyscall(ins) )
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) br_predict, 
            IARG_INST_PTR, IARG_BRANCH_TAKEN,  IARG_END);
    }
    else if (INS_IsDirectBranchOrCall(ins))
    {
        if( INS_IsCall(ins) ) {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) br_predict, 
                IARG_INST_PTR, IARG_BRANCH_TAKEN,  IARG_END);
        }
        else {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) br_predict, 
                IARG_INST_PTR, IARG_BRANCH_TAKEN,  IARG_END);
        }
    }
    else if( INS_IsIndirectBranchOrCall(ins) )
    {
        if( INS_IsCall(ins) ) {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) br_predict, 
                IARG_INST_PTR, IARG_BRANCH_TAKEN,  IARG_END);
    }
        else {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) br_predict, 
                IARG_INST_PTR, IARG_BRANCH_TAKEN,  IARG_END);
        }
    }

}

/* ===================================================================== */

#define OUT(n, a, b) out << n << " " << a << setw(16) << CountSeen. b  << " " << setw(16) << CountTaken. b << endl

VOID Fini(int n, void *v)
{
    WriteResults(false);
}


/* ===================================================================== */


/* ===================================================================== */

int main(int argc, char *argv[])
{
    
    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }

    BTB_init(); // Initialize hardware structures
        
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
