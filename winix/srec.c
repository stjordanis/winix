#include <kernel/kernel.h>
#include <winix/srec.h>



#define BUF_LEN        100

    // sys method used by shell to read srec file from serial port 1
    // and replace the parameter process with the new dynamically loaded process
int exec_read_srec(struct proc *p){
    char buf[BUF_LEN];
    size_t *memory_values = NULL;
    int wordslength = 0;
    int wordsLoaded = 0;
    int temp = 0;
    int i = 0;
    size_t entry = 0;

    // read a line from the port
    for(i = 0; i < BUF_LEN - 1; i++) {
    buf[i] = kgetc(p);     // read
        // temp = (int)buf[i];
        // kprintf("%d ",temp);
    if(buf[i] == '\n') { // test for end

        buf[i+1] = '\0';
        break;
    }
}
    // assuming the first line of the srec is of type S6, so we can read the length of the words to be loaded
if ((wordslength = winix_load_srec_words_length(buf))) {
        // allocate a new array for storing temporary text
    memory_values = (size_t *)kmalloc(wordslength);
    while(1){
            // read line
        for(i = 0; i < BUF_LEN - 1; i++) {
        buf[i] = kgetc(p);     // read
                 // temp = (int)buf[i];
                 // kprintf("%d ",temp);
        if(buf[i] == '\n') { // test for end
            buf[i] = '\0';
            break;
        }
    }
    if (i >= BUF_LEN) {
        // kprintf("incorrect srec file format: line too big\n");
        return ERR;
    }
            // if the line is of type S7, then we are at the end of the srec file, the data returned, should
            // be the entry point of the process
    if (buf[1] == '7') {
        temp = wordsLoaded;
        entry = winix_load_srec_mem_val(buf,memory_values,wordsLoaded,wordslength);
                // kprintf("entry 0x%08x",entry);
        if (temp != wordsLoaded) {
          // kprintf("wordsLoaded %d, previous wordsLoaded %d, last s7 is incorrect\n");
            return ERR;
        }
        break;
    }else{
                // if it's not the end of srec, read the line, store the text values read into the memory temp array
                // and add the wordsLoaded by the amount added
        if ((temp = winix_load_srec_mem_val(buf,memory_values,wordsLoaded,wordslength))) {
            wordsLoaded += temp;
        }else{
                    // kprintf("something went wrong\n");
            return ERR;
        }
    }
    if (wordsLoaded > wordslength) {
        // kprintf("wordsLoaded %d exceed wordslength %d\n",wordsLoaded,wordslength);
        return ERR;
    }
    if (wordsLoaded % 100 == 0) {
        kputc('.');
    }
}
    // kprintf("wordsLoaded %d wordslength %d\n",wordsLoaded,wordslength);


    // p = exec_replace_existing_proc(p,memory_values,wordslength,entry,SYSTEM_PRIORITY,p->name);
    return OK;
    }else{
        return ERR;
    }
}

    // load S6 srec type, and return the size of memory words
    // return 0 if checksum failed
int winix_load_srec_words_length(char *line){
    int i=0;

    int index = 0;
    int checksum = 0;
    byte_t byteCheckSum = 0;
    int recordType = 0;
    int byteCount = 0;
    char buffer[128];
    char tempBufferCount = 0;
    int wordsCount = 0;
    int length = 0;
    byte_t readChecksum = 0;
    int data = 0;

    index = 0;
    checksum = 0;
        // kprintf("loop %d\n",linecount );
                // Start code, always 'S'
    if (line[index++] != 'S') {
          // kprintf("Expect S, but input data is %d\n",line[index-1]);
    }

    recordType = line[index++] - '0';
    if (recordType != 6) {
          // kprintf("recordType %d\n",recordType );
          // kprintf("format is incorrect\n" );
        return ERR;
    }
    tempBufferCount = substring(buffer,line,index,2);
                // kprintf("record value %s, value in base 10: %d,length %d\r\n",buffer,hex2int(buffer,tempBufferCount),tempBufferCount);
    byteCount = hex2int(buffer,tempBufferCount);
    index += 2;
    checksum += byteCount;
    tempBufferCount = substring(buffer,line,index,(byteCount-1)*2 );
                        // kprintf("temp byte value %s, value in base 10: %d,length %d\r\n",buffer,hex2int(buffer,tempBufferCount),tempBufferCount);
    data = hex2int(buffer,tempBufferCount);
            // kprintf("data %d\n", data);
    index += (byteCount-1)*2;
    checksum += data;

                // Checksum, two hex digits. Inverted LSB of the sum of values, including byte count, address and all data.
                // readChecksum = (byte)Convert.ToInt32(line.substring(index, 2), 16);
        // kprintf("checksum %d\n",checksum );
    tempBufferCount = substring(buffer,line,index,2);
                // kprintf("read checksum value %s, value in base 10: %d,length %d\r\n",buffer,hex2int(buffer,tempBufferCount),tempBufferCount);
    readChecksum = (byte_t)hex2int(buffer,tempBufferCount);
        // kprintf("readChecksum %d\n",readChecksum & 0xffffffff);
        // kprintf("readChecksum %d\n",readChecksum & 0x000000FF);
      //  kprintf("checksum %d\n",checksum );

    if (checksum > 255) {
        byteCheckSum = (byte_t)(checksum & 0xffffffff);
                    // kprintf("checksum %d\n",byteCheckSum );
        byteCheckSum = (byte_t)(checksum & 0x000000FF);
                    // kprintf("checksum %d\n",byteCheckSum );
        byteCheckSum = ~byteCheckSum;
                    // kprintf("checksum %d\n",byteCheckSum);
        byteCheckSum &= 0x000000ff;
                    // kprintf("checksum %d\n",byteCheckSum);
    }else{
        byteCheckSum = ~byteCheckSum;
    }

    if (readChecksum != byteCheckSum){
                    // kprintf("checksum %d, readChecksum %d\r\n",byteCheckSum,readChecksum );
                    // kprintf("failed checksum\r\n" );
        return ERR;
    }
    return data;
}

    // read the Srec line, and return the number of words loaded
    // lines is appended with the new words read from the srec file
int winix_load_srec_mem_val(char *line,size_t *memory_values,int start_index,int memvalLength){
    int wordsLoaded = 0;
    int index = 0;
    int checksum = 0;
    char byteCheckSum = 0;
    int recordType = 0;
    int addressLength = 0;
    int byteCount = 0;
    char buffer[128];
    char tempBufferCount = 0;
    int address = 0;
    char data[255];
    int readChecksum = 0;
    int datalength = 0;
    size_t memVal = 0;
    int i = 0;
    int j = 0;

    // kprintf("%s\r\n",line);
    // kprintf("loop %d\n",linecount );
    // Start code, always 'S'
    if (line[index++] != 'S') {
        // kprintf("Expect S, but input data is %d\n",line[index-1]);
        return ERR;
    }

    // Record type, 1 digit, 0-9, defining the data field
    //0: Vendor-specific data
    //1: 16-bit data sequence
    //2: 24 bit data sequence
    //3: 32-bit data sequence
    //5: Count of data sequences in the file. Not required.
    //7: Starting address for the program, 32 bit address
    //8: Starting address for the program, 24 bit address
    //9: Starting address for the program, 16 bit address
    recordType = line[index++] - '0';

    switch (recordType)
    {
        case 0:
        case 1:
        case 9:
        addressLength = 2;
        break;

        case 5:
        case 6:
        addressLength = 0;
        break;

        case 2:
        case 8:
        addressLength = 3;
        break;

        case 3:
        case 7:
        addressLength = 4;
        break;

        default:
        kprintf("unknown record type\n");
        return ERR;
    }
    tempBufferCount = substring(buffer,line,index,2);
    // kprintf("record value %s, value in base 10: %d,length %d\r\n",buffer,hex2int(buffer,tempBufferCount),tempBufferCount);
    byteCount = hex2int(buffer,tempBufferCount);
    index += 2;
    checksum += byteCount;

    // byteCount = ((int)line[index++])*10 + ((int)line[index++]);
    // int byteCount = Convert.ToInt32(line.substring(index, 2), 16);
    // kprintf("byteCount %d\r\n",byteCount);



                // Address, 4, 6 or 8 hex digits determined by the record type
    for (i = 0; i < addressLength; i++)
    {
        tempBufferCount = substring(buffer,line,index+i*2,2);
        // kprintf("temp byte value %s, value in base 10: %d,length %d\r\n",buffer,hex2int(buffer,tempBufferCount),tempBufferCount);
        checksum += hex2int(buffer,tempBufferCount);
        // string ch = line.substring(index + i * 2, 2);
        // checksum += Convert.ToInt32(ch, 16);
    }
    if (addressLength!=0) {
        tempBufferCount = substring(buffer,line,index,addressLength*2);
        // kprintf("temp address value %s, value in base 10: %d,length %d\r\n",buffer,hex2int(buffer,tempBufferCount),tempBufferCount);
        address = hex2int(buffer,tempBufferCount);
    }


    // address = Convert.ToInt32(line.substring(index, addressLength * 2), 16);
    // kprintf("index %d\n",index );
    index += addressLength * 2;
    // kprintf("index %d\n",index );
    byteCount -= addressLength ;
    // kprintf("byteCount %d\n",byteCount );
    // Data, a sequence of bytes.
    // data.length = 255

    for (i = 0; i < byteCount-1; i++)
    {
        tempBufferCount = substring(buffer,line,index,2);
        // kprintf("temp byte value %s, value in base 10: %d,length %d\r\n",buffer,hex2int(buffer,tempBufferCount),tempBufferCount);
        data[i] = hex2int(buffer,tempBufferCount);
        // data[i] = (byte)Convert.ToInt32(line.substring(index, 2), 16);
        index += 2;
        checksum += data[i];
    }

    // Checksum, two hex digits. Inverted LSB of the sum of values, including byte count, address and all data.
    // readChecksum = (byte)Convert.ToInt32(line.substring(index, 2), 16);

    tempBufferCount = substring(buffer,line,index,2);
    // kprintf("read checksum value %s, value in base 10: %d,length %d\r\n",buffer,hex2int(buffer,tempBufferCount),tempBufferCount);
    readChecksum = hex2int(buffer,tempBufferCount);
    // kprintf("checksum %d\r\n",checksum );
    byteCheckSum = (byte)(checksum & 0xFF);
    // kprintf("checksum %d\r\n",byteCheckSum );
    byteCheckSum = ~byteCheckSum;

    byteCheckSum &= 0x000000ff;
    // kprintf("checksum %d\r\n",byteCheckSum );
    if (readChecksum != byteCheckSum){
        // kprintf("checksum %d, readChecksum %d\r\n",byteCheckSum,readChecksum );
        // kprintf("failed checksum\r\n" );
        return ERR;
    }

    // Put in memory
    if ((byteCount-1) % 4 != 0) {
        // kprintf("checksum %d, readChecksum %d\r\n",byteCheckSum,readChecksum );
        // kprintf("Data should only contain full 32-bit words.\n");
        return ERR;
    }

    // kprintf("recordType %d\n", recordType);
    // kprintf("%lu\n",(size_t)data[0] );
    // kprintf("byteCount %d\n",byteCount );
    switch (recordType)
    {
        case 3: // data intended to be stored in memory.

        for (i = 0; i < byteCount-1; i += 4)
        {
            memVal = 0;
            for (j = i; j < i + 4; j++)
            {

                memVal <<= 8;
                memVal |= data[j];
            }

            memory_values[start_index + wordsLoaded] = memVal;
            wordsLoaded++;
                        // kprintf("0x%08x\n",memVal );
        }
        break;


        case 7: // entry point for the program.
        return address;
        break;
    }
    return wordsLoaded;
}

