Assembler : Assembler.c
	gcc -g -ansi -pedantic -Wall Assembler.c PreAssembler.c  MacroFunctions.c FirstPass.c LabelFunctions.c SecondPass.c InstructionsFunctions.c DirectivesFunctions.c LineProcessFunctions.c ValidationFunctions.c FilesFunctions.c -lm -o Assembler
