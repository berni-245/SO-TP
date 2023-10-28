#include <registers.h>

static char registersNames[REGISTER_QUANTITY][STR_LEN] = {"RAX", "RBX", "RCX", "RDX", "RBP", "RSP", "RDI", "RSI",
                                                          "R08", "R09", "R10", "R11", "R12", "R13", "R14", "R15"};

Register registers[REGISTER_QUANTITY]; 
    

Register * getRegisters(){
    
    uint64_t * registersValues = getRegistersValues();   // obtengo los valores en hexa
    
    for(int i = 0; i < REGISTER_QUANTITY; i++){
        for(int j = 0; j < STR_LEN; j++){
            registers[i].name[j] = registersNames[i][j]; // debería almacenar el null, hago esto porque no me deja hacer
        }                                                // asignación directa, así que hago un strCopy manual
        registers[i].value = registersValues[i];
    }

}