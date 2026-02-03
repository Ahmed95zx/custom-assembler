; pass_long.as — longer passing test with mixed features

.entry START
.entry DONE
.entry SUM

.extern EXTA
.extern EXTB

; ---------- code ----------
START:  mov #0, r0
        mov #1, r1
        mov #0, r2
        mov #0, r3
        mov #0, r4
        mov #0, r5
        mov #0, r6
        mov #0, r7

       
        mov MAT1[r0][r1], r2      
        add r2, SUM               
        inc COUNT                 
        prn r2

        ; touch externs to produce .ext entries
        bne EXTA
        jmp EXTB

LOOP1:  mov MAT1[r1][r0], r3      
        add r3, r4
        prn r3
        add r1, IDX               
        bne EXTA

        mov MAT2[r0][r1], r5
        add r5, r4
        prn r5
        add         r0,         IDX
        bne CONT1

        jmp EXTB

CONT1:  sub r1, r1                
        mov #2,          r1
        mov MAT2[r1][r0], r6
        add      r6,      TOTAL
        prn TOTAL

        ; mix addressing again
        mov STR2, r7              
        add r7, r0
        prn r0

        ; another extern touch
        bne EXTA

        ; small counting loop
        mov #3, r6
        mov #0, r7

LOOP2:  add r6, r7
        prn r7
        sub r6, r5              
        ; sub r6, r6              
        ; add #2, r6
        bne EXTA
        prn #-1

        ; read another matrix cell and accumulate
        mov MAT1[r0][r0], r3
        add r3, SUM
        inc COUNT

        ; progress and branch chain
        add r1, r0
        bne EXTB
        jmp MID

MID:    mov ARR, r2               
        add r2, r3
        prn r3

        mov MAT2[r1][r1], r5
        add r5, TOTAL
        prn TOTAL

        ; final extern pings
        bne EXTA
        jmp EXTB

DONE:   stop

; ---------- data ----------
STR1:   .string "longer test case!"
STR2:   .string "second string"
ARR:    .data 7,-3,15,0,22,-1
SUM:    .data 0
TOTAL:  .data 100
COUNT:  .data 0
IDX:    .data 0

; 2x3 and 3x2 matrices
MAT1:   .mat [2][3] 1,2,3,4,5,6
MAT2:   .mat [3][2] 7,8,9,10,11,12

