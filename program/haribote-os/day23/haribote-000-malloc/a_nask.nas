;; [FORMAT "WCOFF"]
; [INSTRSET "i486p"]
[BITS 32]
; [FILE "a_nask.nas"]

GLOBAL	api_putchar
GLOBAL  api_putstr0
GLOBAL  api_end
GLOBAL  api_openwin, api_putstrwin, api_boxfilwin
GLOBAL  api_initmalloc
GLOBAL  api_malloc
GLOBAL  api_free
GLOBAL  api_point
GLOBAL	api_refreshwin
GLOBAL  api_linewin
GLOBAL  api_closewin
GLOBAL  api_getkey


[SECTION .text]

api_putchar:   ; void api_putchar (int c);
    mov     edx, 1
    mov     al,[esp+4]   ; c
    int     0x40
    ret

api_putstr0:   ; void api_putstr0 (char *s);
    push    ebx
    mov        edx, 2
    mov        ebx, [esp+8]  ; s
    int        0x40
    pop        ebx
    ret
    
api_end:  ; void api_end (void);
    mov     edx, 4
    int     0x40


api_openwin:   ; int api_openwin (char *buf, int xsiz, int ysiz, int col_inv, char *title);
    push    edi
    push    esi
    push    ebx
    mov     edx, 5
    mov     ebx, [esp+16]    ; buf
    mov     esi, [esp+20]    ; xsiz
    mov     edi, [esp+24]    ; ysiz
    mov     eax, [esp+28]    ; col_inv
    mov     ecx, [esp+32]    ; title
    int     0x40
    pop     ebx
    pop     esi
    pop     edi
    ret


api_putstrwin: ; void api_putstrwin (int win, int x, int y, int col, int len, char *str);
    push    edi
    push    esi
    push    ebp
    push    ebx
    mov     edx, 6
    mov     ebx, [esp+20]   ; win
    mov     esi, [esp+24]   ; x
    mov     edi, [esp+28]   ; y
    mov     eax, [esp+32]   ; col
    mov     ecx, [esp+36]   ; len
    mov     ebp, [esp+40]   ; str
    int     0x40
    pop     ebx
    pop     ebp
    pop     esi
    pop     edi
    ret


api_boxfilwin ; void api_boxfilwin (int win, int x0, int y0, int x1, int y1, int col);
    push    edi
    push    esi
    push    ebp
    push    ebx
    mov     edx, 7
    mov     ebx, [esp+20]   ; win
    mov     eax, [esp+24]   ; x0
    mov     ecx, [esp+28]   ; y0
    mov     esi, [esp+32]   ; x1
    mov     edi, [esp+36]   ; y1
    mov     ebp, [esp+40]   ; col
    int     0x40
    pop     ebx
    pop     ebp
    pop     esi
    pop     edi
    ret


api_initmalloc: ; void api_initmalloc (void);
    push    ebx
    mov        edx, 8
    mov        ebx, [cs:0x0020]
    mov        eax, ebx
    add        eax, 32*1024
    mov        ecx, [cs:0x0000]
    sub        ecx, eax
    int     0x040
    pop        ebx
    ret

api_malloc: ; char *api_malloc (int size);
    push    ebx
    mov     edx, 9
    mov     ebx, [cs:0x0020]
    mov     ecx, [esp+8]
    int     0x40
    pop     ebx
    ret

api_free: ; void api_free (char *addr, int size);
    push    ebx
    mov     edx, 10
    mov     ebx, [cs:0x0020]
    mov     eax, [esp + 8]
    mov     ecx, [esp + 12]
    int     0x40
    pop     ebx
    ret

api_point: ; void api_point (int win, int x, int y, int col);
    push    edi
    push    esi
    push    ebx
    mov     edx, 11
    mov     ebx, [esp + 16]   ; win
    mov     esi, [esp + 20]   ; x
    mov     edi, [esp + 24]   ; y
    mov     eax, [esp + 28]   ; col
    int     0x40
    pop     ebx
    pop     esi
    pop     edi
    ret

api_refreshwin: ; void api_refreshwin (int win, int x0, in ty0, int x1, int y1);
    push    edi
    push    esi
    push    ebx
    mov     edx, 12
    mov     ebx, [esp + 16]   ; win
    mov     eax, [esp + 20]   ; x0
    mov     ecx, [esp + 24]   ; y0
    mov     esi, [esp + 28]   ; x1
    mov     edi, [esp + 32]   ; y1
    int     0x40
    pop     ebx
    pop     esi
    pop     edi
    ret

api_linewin: ; void api_linewin (int win, int x0, int y0, int x1, int y1, int col);
    push	edi
	push	esi
	push	ebp
	push	ebx
	mov		edx, 13
    mov     ebx, [esp + 20]   ; win
    mov     eax, [esp + 24]   ; x0
    mov     ecx, [esp + 28]   ; y0
    mov     esi, [esp + 32]   ; x1
    mov     edi, [esp + 36]   ; y1
    mov     ebp, [esp + 40]   ; col
    int     0x40
    pop     ebx
	pop		ebp
    pop     esi
    pop     edi
	ret

api_closewin: ; void api_closewin (int win);
	push	ebx
	mov		edx, 14
	mov		ebx, [esp + 8] ; win
	int		0x40
	pop		ebx
	ret

api_getkey: ; int api_getkey (int mode);
    mov		edx, 15
	mov		eax, [esp + 4];  mode
	int		0x40
	ret

