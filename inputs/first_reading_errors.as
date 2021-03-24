;errors check - first pass phase

;;;;;;;label errors;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
myLabel: .string "This should be fine"
1myLabel: .string "This shouldn't"
thisIsAveryVeryVeryVeryVeryLongLabel: .data 12, 4, 56, 78, 10
this label contains spaces in it: mov L1, L1
thisLabelIsErroneous  : sub r1, r4
myLabel: .string "This label was already used on first line"

;;;;;;;comma errors;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
myArray: .data 12,6, -9, 10,
myArray2: .data 12,6,-9,10,,
myArray3: .data ,12,6, -9, , 10
myArray4: .data 12,     ,    6,-9,10
myArray5: .data 12, 6, -9, 0, -10       ,    
mov, r3, K
cmp K, , #-6
sub ,r1, r4 
add , L3  L3  
add , L3,  L3
inc , r1  
inc, r1

;;;;;;;;;data errors;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.data
.data lost, 4, 8, 15, 16, 23, 42
.data 4, 8, 1  5 , 16, 23, 42
.data 4,8,15,16,23,4 +2
.data  4, 8, 15, x, 16, 23, 42
.data 4097
.data -4097
.data a, b, c, d
.data 3, 4, 6.5, 9


;;;;;;;;;;string errors;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.string
.string za wardo
.string za wardo"
.string "za wardo
.string za "wardo".
.string za ."wardo"
.string ",

;;;;;;extraneous words;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov A, r1 r2 
cmp A, r1 r2 
add A, r0 r1 
sub #3, r1, r5
sub #3, r1 r5 
inc r1, r4 
jmp %ROOF %FLOOR
jsr BAT MAN
red r3, r5
customLabel1: .data 943 .data
customLabel2: .string "sdf" .string
customLabel6: mov A, r1 r2 
customLabel7: cmp A, r1, r2
customLabel9: add A, r0, r1
customLabel14: lea HAHAHA, r3 r4 
customLabel16: clr r2 r6
customLabel21: jmp %ROOF, %FLOOR
customLabel24: bne Bob Sponge
customLabel25: jsr BAT, MAN
customLabel28: red r3 r5

;;;;;;;;missing word;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.data 
.string 
.extern
.entry 
cmp A
add A
sub #3
lea HAHAHA
clr
inc
bne
jsr
red
someLabel343: .data 
someLabel34: .entry 
someLabel38: sub #3
someLabel69: lea
someLabel40: clr

;;;;;;;;;;unknown words;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
COOK tasty, pie
.Data 12, 675
.sTring "Whoops"

;;;;;;;;;;;operand addressing errors;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		mov   	%ohno  , 				%ohyes
mov		 %ohno ,  r3
mov	 #-1, %ohyes
cmp %ohno, %ohyes
add %ohno	, %ohyes
add #-1, 									#5
add #-1, %ohyes
												sub %ohno, #1
	sub  	 	 	%ohno, r3
sub #-1, #5
 sub #-1, %ohyes
lea #-1, someLabel47
										lea #-1, %ohno
		lea 		%ohno, 		someLabel47
lea someLabel47, #-1
clr #-1
   clr %ohno
not #-1
       	not		 %ohno
  inc #-1
 inc %ohno
dec #-1
			dec %ohno
jmp #-1
		bne r1
jsr r1
	red #-1
prn %ohno

;;;;;;;;;;extra text after command;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
rts #-1
stop r1
