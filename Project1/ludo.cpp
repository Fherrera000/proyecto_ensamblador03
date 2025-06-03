#include <windows.h>

extern "C" {

    // Variables globales
	// el jugador gana cuando tiene 2 fichas en la meta, las dos fichas tienen que dar una vuelta completa ya que es un vector circular
    // No he implementado casillas seguras y la logica del movimiento esta algo chafa, por si puedes checar
    unsigned char jugadores[32 * 4] = { 0 }; // 4 jugadores * 32 bytes
    unsigned int tablero[52] = { 0 };
    unsigned int posiciones_inicio[4] = { 0, 13, 26, 39 }; // < -- Posiciones de inicio para cada jugador, es un vector circular
    unsigned int turno_actual = 0;
    unsigned int estado_juego = 1;
    int ganador = -1;
    unsigned int ultimo_dado = 0;
    unsigned int semilla_random = 12345;

    __declspec(dllexport) int obtener_offset_jugador(int jugador) {
        return jugador * 32;
    }

    __declspec(dllexport) void* obtener_ficha_jugador(int jugador, int numFicha) {
        if (jugador < 0 || jugador > 3 || (numFicha != 0 && numFicha != 1))
            return nullptr;

        unsigned int offset = jugador * 32;
        return &jugadores[offset + (numFicha == 1 ? 12 : 0)];
    }

    __declspec(dllexport) void capturar_ficha(void* ficha, int jugador) {
        __asm {
            mov ebx, ficha
            mov ecx, jugador
            mov dword ptr[ebx], -1
            mov dword ptr[ebx + 4], 0
            push ecx
            call obtener_offset_jugador
            add esp, 4
            mov edx, eax
            mov eax, dword ptr[jugadores + edx + 24]
            inc eax
            mov dword ptr[jugadores + edx + 24], eax
        }
    }

    __declspec(dllexport) void verificar_captura(int jugador, int posicion) {
        __asm {
            mov esi, jugador
            mov edi, posicion
            xor ecx, ecx
            siguiente_jugador :
            cmp ecx, 4
                jge fin_verificar
                cmp ecx, esi
                je continuar_loop

                push 0
                push ecx
                call obtener_ficha_jugador
                add esp, 8
                mov ebx, eax
                test ebx, ebx
                jz continuar_loop
                mov eax, dword ptr[ebx]
                cmp eax, edi
                    jne verificar_ficha2
                    push ecx
                    push ebx
                    call capturar_ficha
                    add esp, 8

                    verificar_ficha2:
                push 1
                    push ecx
                    call obtener_ficha_jugador
                    add esp, 8
                    mov ebx, eax
                    test ebx, ebx
                    jz continuar_loop
                    mov eax, dword ptr[ebx]
                    cmp eax, edi
                        jne continuar_loop
                        push ecx
                        push ebx
                        call capturar_ficha
                        add esp, 8

                        continuar_loop:
                    inc ecx
                        jmp siguiente_jugador
                        fin_verificar :
        }
    }

    __declspec(dllexport) int verificar_victoria(int jugador) {
        __asm {
            mov eax, jugador
            push eax
            call obtener_offset_jugador
            add esp, 4
            mov ebx, eax
            mov eax, dword ptr[jugadores + ebx + 28]
            cmp eax, 2
            jne no_ganador
            mov eax, jugador
            mov dword ptr[ganador], eax
            mov dword ptr[estado_juego], 0
            mov eax, 1
            jmp fin_verificar
            no_ganador :
            mov eax, 0
                fin_verificar :
        }
    }

    __declspec(dllexport) int puede_mover_ficha(void* ficha, int dado) {
        if (!ficha) return 0;
        __asm {
            mov ebx, ficha
            mov eax, dword ptr[ebx]
                cmp eax, -1
                    jne en_tablero
                    cmp dado, 6
                    je puede
                    jmp no_puede
                    en_tablero :
                cmp eax, 52
                    jge no_puede
                    add eax, dado
                    cmp eax, 59
                    jg no_puede
                    puede :
                mov eax, 1
                    jmp fin
                    no_puede :
                mov eax, 0
                    fin :
        }
    }

    __declspec(dllexport) void sacar_ficha_de_casa(void* ficha, int jugador) {
        __asm {
            mov ebx, ficha
            mov ecx, jugador
            mov eax, ecx
            shl eax, 2
            mov eax, dword ptr[posiciones_inicio + eax]
            mov dword ptr[ebx], eax
            mov dword ptr[ebx + 4], 1
            mov dword ptr[ebx + 8], 1
            push ecx
            call obtener_offset_jugador
            add esp, 4
            mov edx, eax
            mov eax, dword ptr[jugadores + edx + 24]
            dec eax
            mov dword ptr[jugadores + edx + 24], eax
            mov eax, dword ptr[ebx]
                cmp eax, 52
                    jge fin_sacar
                    mov edx, ecx
                    inc edx
                    mov dword ptr[tablero + eax * 4], edx
                    fin_sacar :
        }
    }

    __declspec(dllexport) void mover_ficha(int jugador, int ficha_idx, int dado) {
        __asm {
            push eax
            push ebx
            push ecx
            push edx
            push esi

            mov esi, jugador
            mov ecx, ficha_idx
            mov edx, dado

            push ecx
            push esi
            call obtener_ficha_jugador
            add esp, 8
            mov ebx, eax
            test ebx, ebx
            jz fin_mover

            push edx
            push ebx
            call puede_mover_ficha
            add esp, 8
            cmp eax, 1
            jne fin_mover

            mov eax, dword ptr[ebx]
                cmp eax, -1
                    jne mover_tablero
                    cmp edx, 6
                    jne fin_mover
                    push esi
                    push ebx
                    call sacar_ficha_de_casa
                    add esp, 8
                    mov eax, dword ptr[ebx]
                    push eax
                        push esi
                        call verificar_captura
                        add esp, 8
                        jmp fin_mover

                        mover_tablero :
                    cmp eax, 52
                        jge calcular_nueva
                        mov dword ptr[tablero + eax * 4], 0
                        calcular_nueva :
                        add eax, edx
                        cmp eax, 59
                        jg fin_mover
                        mov dword ptr[ebx], eax
                        cmp eax, 59
                        jne actualizar_tablero
                        mov dword ptr[ebx], 999
                        mov dword ptr[ebx + 4], 0
                        push esi
                        call obtener_offset_jugador
                        add esp, 4
                        mov ecx, eax
                        mov eax, dword ptr[jugadores + ecx + 28]
                        inc eax
                        mov dword ptr[jugadores + ecx + 28], eax
                        jmp fin_mover

                        actualizar_tablero :
                    mov ecx, esi
                        inc ecx
                        mov dword ptr[tablero + eax * 4], ecx
                        push eax
                        push esi
                        call verificar_captura
                        add esp, 8
                        mov eax, dword ptr[ebx + 8]
                        inc eax
                        mov dword ptr[ebx + 8], eax

                        fin_mover :
                    pop esi
                        pop edx
                        pop ecx
                        pop ebx
                        pop eax
        }
    }

    __declspec(dllexport) int lanzar_dado() {
        __asm {
            push edx
            push ecx
            mov eax, dword ptr[semilla_random]
                mov ecx, 1103515245
                    mul ecx
                    add eax, 12345
                    mov dword ptr[semilla_random], eax
                    xor edx, edx
                    mov ecx, 6
                    div ecx
                    inc edx
                    mov eax, edx
                    mov dword ptr[ultimo_dado], eax
                    pop ecx
                    pop edx
        }
    }

    __declspec(dllexport) void inicializar_juego() {
        __asm {
            lea edi, jugadores
            mov ecx, 128
            xor eax, eax
            rep stosb
            lea edi, tablero
            mov ecx, 52
            xor eax, eax
            rep stosd
            mov dword ptr[turno_actual], 0
            mov dword ptr[estado_juego], 1
            mov dword ptr[ganador], -1
            mov dword ptr[semilla_random], 54321
        }
    }


    __declspec(dllexport) unsigned int* obtener_tablero() {
        return tablero;
    }
}
