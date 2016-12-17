.globl _start
_start:
    ldr pc, vector_00
    ldr pc, vector_04
    ldr pc, vector_08
    ldr pc, vector_0C
    ldr pc, vector_10
    ldr pc, vector_14
    ldr pc, vector_18
    ldr pc, vector_1C
vector_00: .word reset
vector_04: .word hang
vector_08: .word hang
vector_0C: .word hang
vector_10: .word hang
vector_14: .word hang
vector_18: .word hang
vector_1C: .word hang

reset:
    mov r0,#0x8000
    mov r1,#0x0000
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}

    mov sp,#0x00800000
    bl pimain
hang: b hang

.globl dummy
dummy:
    bx lr

.globl start_mmu
start_mmu:
    mov r2,#0
    mcr p15,0,r2,c7,c7,0 ;@ invalidate caches
    mcr p15,0,r2,c8,c7,0 ;@ invalidate tlb
    mcr p15,0,r2,c7,c10,4 ;@ DSB ??

    mvn r2,#0
    bic r2,#0xC
    mcr p15,0,r2,c3,c0,0 ;@ domain

    mcr p15,0,r0,c2,c0,0 ;@ tlb base
    mcr p15,0,r0,c2,c0,1 ;@ tlb base

    mrc p15,0,r2,c1,c0,0
    orr r2,r2,r1
    mcr p15,0,r2,c1,c0,0

    bx lr

.globl stop_mmu
stop_mmu:
    mrc p15,0,r2,c1,c0,0
    bic r2,#0x1000
    bic r2,#0x0004
    bic r2,#0x0001
    mcr p15,0,r2,c1,c0,0
    bx lr

.globl invalidate_tlbs
invalidate_tlbs:
    mov r2,#0
    mcr p15,0,r2,c8,c7,0  ;@ invalidate tlb
    mcr p15,0,r2,c7,c10,4 ;@ DSB ??
    bx lr

;@-------------------------------------------------------------------------
;@
;@ Copyright (c) 2012 David Welch dwelch@dwelch.com
;@
;@ Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;@
;@ The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;@
;@ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;@
;@-------------------------------------------------------------------------
