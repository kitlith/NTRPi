    .global header
    .global header_size
    .section .rodata
header:
    .incbin HEADER
1:
header_size:
    .int 1b - header
