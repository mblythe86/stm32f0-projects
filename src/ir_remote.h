/*
all signals are active low.  i.e. 1=pulled to ground
each digit represents approximately 880 microseconds (us)
Also, all are preceeded and followed by (mostly omitted) zeroes
Here's the pattern: each one starts with "101" as a header,
Then it's followed by 2-bit pairs of either
"01" (call this 1) or "10" (call this 0). This is Manchester coding.
Each signal below actually encodes 12 bits (24 transitions)
OK     : 101010101011010011010011001 - 111100100101 - 0xf25
Up     : 101100101011010100110011010 - 011100010100 - 0x714
Down   : 101010101011010100110011001 - 111100010101 - 0xf15
Left   : 101100101011010100110010110 - 011100010110 - 0x716
Right  : 101010101011010100110010101 - 111100010111 - 0xf17
Vol up : 101010101011010100110101010 - 111100010000 - 0xf10
Vol dn : 101100101011010100110101001 - 011100010001 - 0x711
Chn up : 101010101011010011010101010 - 111100100000 - 0xf20
Chn dn : 101100101011010011010101001 - 011100100001 - 0x721

*/
