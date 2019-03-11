;	Static Name Aliases
;
	TITLE   pcgetch
	.287

BREAK_TEXT	SEGMENT  BYTE PUBLIC 'CODE'
BREAK_TEXT	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS


DGROUP	GROUP	CONST,	_BSS,	_DATA
	ASSUME  CS: BREAK_TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP


BREAK_TEXT      SEGMENT

	PUBLIC	_break_off

oldb	DB 	0

_break_off	PROC FAR
	push	bp
	mov	bp,sp

	mov ax, 3300h
	int 21h
	mov oldb, dl
	mov dl, 0
	mov ax, 3301h
	int 21h

	mov	sp,bp
	pop	bp
	ret	
_break_off	ENDP

	PUBLIC	_break_rst

_break_rst	PROC FAR
	push	bp
	mov	bp,sp

	mov ax, 3301h
	mov dl, oldb
	int 21h

	mov	sp,bp
	pop	bp
	ret	
_break_rst	ENDP

BREAK_TEXT	ENDS
END
