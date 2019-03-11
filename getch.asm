;	Static Name Aliases
;
	TITLE   getch

GETCH_TEXT	SEGMENT  BYTE PUBLIC 'CODE'
GETCH_TEXT	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS

DGROUP	GROUP	CONST,	_BSS,	_DATA
	ASSUME  CS: GETCH_TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP

EXTRN	_UpdModLine:WORD


IFDEF	MSDOS ;	------------------ generic MSDOS

_DATA      SEGMENT
minutes	DB	0
last	DB  0
_DATA      ENDS

GETCH_TEXT      SEGMENT

	PUBLIC	_getrawinchar

_getrawinchar	PROC FAR
	push	bp
	mov	bp,sp

    cmp last, 0
	je gnext
	mov al, last
	mov last, 0
	jmp ghome

gnext:
	mov dl, 0ffh
	mov ah, 06h
	int 21h
	jnz git
	call waitfun
	jmp gnext
git:
	cmp al, 0
	jne ghome
	mov al, 0ffh
gread:
ghome:
	mov ah, 0
	mov	sp,bp
	pop	bp
	ret	
_getrawinchar	ENDP

	PUBLIC	_rawkey_ready

_rawkey_ready	PROC FAR
	push	bp
	mov	bp,sp

	cmp last, 0
	jne ky
	mov dl, 0ffh
	mov ah, 06h
	int 21h
	jz kn
	cmp al, 0
	jne ks
	mov al, 0ffh
ks:	mov last, al

ky:	mov ax, 1
	mov	sp,bp
	pop	bp
	ret

kn: mov ax, 0
	mov	sp,bp
	pop	bp
	ret
_rawkey_ready	ENDP

waitfun	PROC near
	; use bios call again for pc version
	mov ah, 2ch
	int 21h
	cmp cl, minutes
	je skip
	mov minutes, cl
	mov _UpdModLine, 1
skip:
	ret
waitfun	ENDP

GETCH_TEXT	ENDS

ELSE	;-------------------------- 	IBMPC code

EXTRN	_UpdModLine:WORD

_DATA      SEGMENT
minutes	DW	0
last    DB  	0
_DATA      ENDS

GETCH_TEXT      SEGMENT

	PUBLIC	_getrawinchar

_getrawinchar	PROC FAR
	push	bp
	mov	bp,sp

    cmp last, 0
	je gnext
	mov al, last
	mov last, 0
	jmp ghome
gnext:
	mov ah, 1
	int 16h
	jnz git
	call waitfun
	jmp gnext
git:
gread:
	mov ah, 0
	int 16h
	cmp al, 0
	jnz ghome
	mov last, ah
	mov al, 0ffh
ghome:
	mov ah, 0
	mov	sp,bp
	pop	bp
	ret	
_getrawinchar	ENDP

	PUBLIC	_rawkey_ready

_rawkey_ready	PROC FAR
	push	bp
	mov	bp,sp

	cmp last, 0
	jne kt
	mov ah, 1
	int 16h
	jz kf
kt:	mov ax, 1
	mov	sp,bp
	pop	bp
	ret
kf: mov ax, 0
	mov	sp,bp
	pop	bp
	ret
_rawkey_ready	ENDP

waitfun	PROC near
	mov ah, 0
	int 1ah
	mov cx, dx
	cmp dx, minutes
	jb update
	sub dx, 444h
;	jb update
	cmp dx, minutes
	ja update
	ret
update:
	mov minutes, cx
	mov _UpdModLine, 1
	ret
waitfun	ENDP

GETCH_TEXT	ENDS

ENDIF

END
